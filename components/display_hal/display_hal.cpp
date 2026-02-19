#include "display_hal.h"
#include "lumari_config.h"
#include "esp_err.h"
#include "esp_log.h"
#include <string.h>

#if LUMARI_BOARD_QEMU
#include "esp_lcd_qemu_rgb.h"
#elif LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_intr_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_co5300.h"
#include "hal/lcd_types.h"
#else
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#endif

static const char *TAG = "display_hal";

static esp_lcd_panel_handle_t s_panel_handle = nullptr;
#if LUMARI_BOARD_QEMU
static void *s_qemu_fb = nullptr;
#elif LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
static esp_lcd_panel_io_handle_t s_io_handle = nullptr;
#endif

#ifndef LCD_FLUSH_CHUNK_LINES
#define LCD_FLUSH_CHUNK_LINES 80
#endif

void display_hal_init(void)
{
#if LUMARI_BOARD_QEMU
    /* QEMU virtual RGB panel */
    const esp_lcd_rgb_qemu_config_t qemu_cfg = {
        .width  = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .bpp    = RGB_QEMU_BPP_16,
    };
    esp_err_t err = esp_lcd_new_rgb_qemu(&qemu_cfg, &s_panel_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "QEMU RGB panel init failed (%s)", esp_err_to_name(err));
        return;
    }
    ESP_ERROR_CHECK(esp_lcd_rgb_qemu_get_frame_buffer(s_panel_handle, &s_qemu_fb));
    ESP_LOGI(TAG, "QEMU display init OK %dx%d", (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
#elif LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    /* CO5300 AMOLED over QSPI — manual config to match current ESP-IDF (CO5300 macros are outdated) */
    const size_t max_transfer = (size_t)SCREEN_WIDTH * LCD_FLUSH_CHUNK_LINES * sizeof(uint16_t);
    const spi_bus_config_t bus_cfg = {
        .data0_io_num = LCD_PIN_DATA0,
        .data1_io_num = LCD_PIN_DATA1,
        .sclk_io_num = LCD_PIN_PCLK,
        .data2_io_num = LCD_PIN_DATA2,
        .data3_io_num = LCD_PIN_DATA3,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = (int)max_transfer,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize((spi_host_device_t)LCD_QSPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    const esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = LCD_PIN_CS,
        .dc_gpio_num = -1,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 32,
        .lcd_param_bits = 8,
        .flags = {
            .dc_high_on_cmd = 0,
            .dc_low_on_data = 0,
            .dc_low_on_param = 0,
            .octal_mode = 0,
            .quad_mode = 1,
            .sio_mode = 0,
            .lsb_first = 0,
            .cs_high_active = 0,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_QSPI_HOST, &io_cfg, &s_io_handle));

    static co5300_vendor_config_t vendor_config = {
        .init_cmds = NULL,
        .init_cmds_size = 0,
        .flags = {
            .use_mipi_interface = 0,
            .use_qspi_interface = 1,
        },
    };
    const esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = (gpio_num_t)LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
        .flags = {},
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_co5300(s_io_handle, &panel_cfg, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));
    display_hal_set_brightness(100);
#else
    /* ST7789 SPI + LEDC backlight */
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);
    ledc_channel_config_t ch_cfg = {
        .gpio_num   = LCD_PIN_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&ch_cfg);
    display_hal_set_brightness(100);

    spi_bus_config_t bus_cfg = {
        .sclk_io_num     = LCD_PIN_SCLK,
        .mosi_io_num     = LCD_PIN_MOSI,
        .miso_io_num     = LCD_PIN_MISO,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = (size_t)SCREEN_WIDTH * LCD_FLUSH_CHUNK_LINES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize((spi_host_device_t)LCD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num    = (gpio_num_t)LCD_PIN_DC,
        .cs_gpio_num    = (gpio_num_t)LCD_PIN_CS,
        .pclk_hz        = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits   = 8,
        .lcd_param_bits = 8,
        .spi_mode       = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_cfg, &io_handle));

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = (gpio_num_t)LCD_PIN_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_cfg, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));
#endif

    ESP_LOGI(TAG, "Display init OK %dx%d", (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
}

void display_hal_flush(uint16_t *framebuffer)
{
    if (s_panel_handle == nullptr || framebuffer == nullptr) {
        return;
    }

#if LUMARI_BOARD_QEMU
    if (s_qemu_fb != nullptr) {
        size_t size = (size_t)SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
        memcpy(s_qemu_fb, framebuffer, size);
        esp_lcd_rgb_qemu_refresh(s_panel_handle);
    }
#else
    for (int y = 0; y < SCREEN_HEIGHT; y += LCD_FLUSH_CHUNK_LINES) {
        int y_end = y + LCD_FLUSH_CHUNK_LINES;
        if (y_end > SCREEN_HEIGHT) {
            y_end = SCREEN_HEIGHT;
        }
        uint16_t *chunk = framebuffer + (size_t)y * SCREEN_WIDTH;
        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y, SCREEN_WIDTH, y_end, chunk);
    }
#endif
}

void display_hal_set_brightness(uint8_t percent)
{
    if (percent > 100) percent = 100;

#if LUMARI_BOARD_QEMU
    (void)percent;
#elif LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    /* CO5300: brightness via command 0x51 (0–255) */
    if (s_io_handle != nullptr) {
        uint8_t val = (uint8_t)(255 * (uint32_t)percent / 100);
        esp_lcd_panel_io_tx_param(s_io_handle, 0x51, &val, 1);
    }
#else
    /* LEDC PWM backlight */
    const int res = 8;
    uint32_t duty = ((1U << res) - 1) * (uint32_t)percent / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
#endif
}

void display_hal_sleep(void)
{
#if !LUMARI_BOARD_QEMU
    if (s_panel_handle != nullptr)
        esp_lcd_panel_disp_on_off(s_panel_handle, false);
#endif
}

void display_hal_wake(void)
{
#if !LUMARI_BOARD_QEMU
    if (s_panel_handle != nullptr)
        esp_lcd_panel_disp_on_off(s_panel_handle, true);
#endif
}
