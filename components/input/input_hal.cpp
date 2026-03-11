#include "input_hal.h"
#include "lumari_config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_ft5x06.h"
#else
#include "driver/spi_master.h"
#endif

static const char *TAG = "input_hal";

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
/* Touch: FT3168/FT5x06 over I2C (addr 0x38); use esp_lcd_touch_ft5x06 driver (BSP-compatible). */
static i2c_master_bus_handle_t s_i2c_bus = nullptr;
static esp_lcd_panel_io_handle_t s_touch_io = nullptr;
static esp_lcd_touch_handle_t s_touch_handle = nullptr;
static bool s_touch_ok = false;
#else
/* XPT2046 resistive over SPI */
static spi_device_handle_t s_touch_spi = nullptr;
static bool s_touch_ok = false;
#define XPT2046_CMD_X  0xD0
#define XPT2046_CMD_Y  0x90
#define XPT2046_MIN    200
#define XPT2046_MAX    3900
#endif

#if !LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
static uint16_t xpt2046_read_coord(uint8_t cmd)
{
    uint8_t tx[3] = { cmd, 0, 0 };
    uint8_t rx[3] = { 0 };
    spi_transaction_t t = {};
    t.length = 24;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    if (spi_device_polling_transmit(s_touch_spi, &t) != ESP_OK)
        return 0;
    uint16_t v = (uint16_t)((rx[1] << 8) | rx[2]) >> 3;
    return v & 0xFFF;
}
#endif

void input_hal_init(void)
{
#if LUMARI_BOARD_QEMU
    s_touch_ok = false;
    ESP_LOGI(TAG, "Input init (QEMU: touch/button stubbed)");
    return;
#endif
    /* Buttons: BOOT (GPIO0, active-low), PWR (GPIO10, active-high) on Waveshare */
    gpio_config_t btn0 = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn0);
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    /* PWR button is on AXP2101 PMIC, polled via power_service_poll_pwr_button_short(); no GPIO10. */

    /* Touch RST: assert reset before I2C so FT3168 is in known state when we first talk to it */
    if (TOUCH_PIN_RST >= 0) {
        gpio_config_t rst = {
            .pin_bit_mask = (1ULL << TOUCH_PIN_RST),
            .mode         = GPIO_MODE_OUTPUT,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&rst);
        gpio_set_level((gpio_num_t)TOUCH_PIN_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level((gpio_num_t)TOUCH_PIN_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    /* Touch: I2C bus then FT5x06 driver (FT3168-compatible; BSP uses same driver). */
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port          = I2C_NUM_0,
        .sda_io_num         = (gpio_num_t)I2C_MASTER_SDA,
        .scl_io_num         = (gpio_num_t)I2C_MASTER_SCL,
        .clk_source         = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt   = 7,
        .intr_priority      = 0,
        .trans_queue_depth  = 0,
        .flags              = { .enable_internal_pullup = true },
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "I2C bus init failed (%s)", esp_err_to_name(err));
        return;
    }
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    io_cfg.scl_speed_hz = 400000;
    err = esp_lcd_new_panel_io_i2c(s_i2c_bus, &io_cfg, &s_touch_io);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Touch panel IO failed (%s); touch disabled", esp_err_to_name(err));
    } else {
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = (uint16_t)(SCREEN_WIDTH - 1),
            .y_max = (uint16_t)(SCREEN_HEIGHT - 1),
            .rst_gpio_num = (gpio_num_t)TOUCH_PIN_RST,
            .int_gpio_num = (gpio_num_t)TOUCH_PIN_INT,
            .levels = { .reset = 0, .interrupt = 0 },
            .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
        };
        err = esp_lcd_touch_new_i2c_ft5x06(s_touch_io, &tp_cfg, &s_touch_handle);
        s_touch_ok = (err == ESP_OK);
        if (!s_touch_ok)
            ESP_LOGW(TAG, "Touch FT5x06 init failed (%s); touch disabled", esp_err_to_name(err));
        else
            ESP_LOGI(TAG, "Input init OK (BOOT GPIO%d, PWR=AXP2101, touch FT5x06 0x%02X)",
                     (int)BUTTON_BOOT_PIN, (int)TOUCH_I2C_ADDR);
    }
    ESP_LOGI(TAG, "Button level at init: BOOT=%d", gpio_get_level((gpio_num_t)BUTTON_BOOT_PIN));
#else
    spi_device_interface_config_t dev_cfg = {};
    dev_cfg.clock_speed_hz = 2 * 1000 * 1000;
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = TOUCH_PIN_CS;
    dev_cfg.queue_size = 1;
    esp_err_t err = spi_bus_add_device((spi_host_device_t)LCD_SPI_HOST, &dev_cfg, &s_touch_spi);
    s_touch_ok = (err == ESP_OK);
    if (!s_touch_ok)
        ESP_LOGW(TAG, "Touch SPI add failed (%s); touch disabled", esp_err_to_name(err));
    else
        ESP_LOGI(TAG, "Input init OK (button GPIO%d, touch CS GPIO%d)", BUTTON_PIN, TOUCH_PIN_CS);
#endif
}

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
i2c_master_bus_handle_t input_hal_get_i2c_bus(void)
{
    return s_i2c_bus;
}
#endif


bool input_hal_touch_read(int *x, int *y)
{
    if (!x) return false;
    if (!y) return false;

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_touch_ok || s_touch_handle == nullptr) {
        *x = 0; *y = 0;
        return false;
    }
    if (esp_lcd_touch_read_data(s_touch_handle) != ESP_OK) {
        *x = 0; *y = 0;
        return false;
    }
    esp_lcd_touch_point_data_t points[1];
    uint8_t n = 0;
    esp_lcd_touch_get_data(s_touch_handle, points, &n, 1);
    if (n < 1) {
        *x = 0; *y = 0;
        return false;
    }
    *x = (int)points[0].x;
    *y = (int)points[0].y;
    if (*x < 0) *x = 0;
    if (*x >= SCREEN_WIDTH) *x = SCREEN_WIDTH - 1;
    if (*y < 0) *y = 0;
    if (*y >= SCREEN_HEIGHT) *y = SCREEN_HEIGHT - 1;
    return true;
#else
    if (!s_touch_ok) {
        *x = 0; *y = 0;
        return false;
    }
    uint16_t raw_x = xpt2046_read_coord(XPT2046_CMD_X);
    uint16_t raw_y = xpt2046_read_coord(XPT2046_CMD_Y);
    if (raw_x < XPT2046_MIN || raw_x > XPT2046_MAX || raw_y < XPT2046_MIN || raw_y > XPT2046_MAX) {
        *x = 0; *y = 0;
        return false;
    }
    *x = (int)((unsigned long)raw_x * (SCREEN_WIDTH - 1) / 4095);
    *y = (int)((unsigned long)raw_y * (SCREEN_HEIGHT - 1) / 4095);
    if (*x >= SCREEN_WIDTH) *x = SCREEN_WIDTH - 1;
    if (*y >= SCREEN_HEIGHT) *y = SCREEN_HEIGHT - 1;
    return true;
#endif
}

bool input_hal_button_read(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    /* BOOT = GPIO0 active low (pressed => low). PWR is on AXP2101; app must OR power_service_poll_pwr_button_short(). */
    return (gpio_get_level((gpio_num_t)BUTTON_BOOT_PIN) == 0);
#else
    return gpio_get_level((gpio_num_t)BUTTON_PIN) == 0;
#endif
}
