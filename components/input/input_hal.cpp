#include "input_hal.h"
#include "lumari_config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "driver/i2c_master.h"
#else
#include "driver/spi_master.h"
#endif

static const char *TAG = "input_hal";

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
/* FT3168 capacitive touch over I2C (addr 0x38) */
static i2c_master_bus_handle_t s_i2c_bus = nullptr;
static i2c_master_dev_handle_t s_touch_dev = nullptr;
static bool s_touch_ok = false;
/* FT3168 / FT5x06 style: reg 0x02 = touch points, 0x03-0x06 = P1 XH,XL,YH,YL */
#define FT3168_REG_POINTS  0x02
#define FT3168_REG_XH      0x03
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
    gpio_config_t btn1 = {
        .pin_bit_mask = (1ULL << BUTTON_PWR_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn1);

    /* Touch: I2C bus + FT3168 at 0x38 */
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port     = I2C_NUM_0,
        .sda_io_num   = (gpio_num_t)I2C_MASTER_SDA,
        .scl_io_num   = (gpio_num_t)I2C_MASTER_SCL,
        .clk_source   = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags        = { .enable_internal_pullup = true },
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "I2C bus init failed (%s)", esp_err_to_name(err));
        return;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = TOUCH_I2C_ADDR,
        .scl_speed_hz    = I2C_MASTER_FREQ_HZ,
    };
    err = i2c_master_bus_add_device(s_i2c_bus, &dev_cfg, &s_touch_dev);
    s_touch_ok = (err == ESP_OK);
    if (s_touch_ok && TOUCH_PIN_RST >= 0) {
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
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    if (!s_touch_ok)
        ESP_LOGW(TAG, "Touch I2C add failed (%s); touch disabled", esp_err_to_name(err));
    else
        ESP_LOGI(TAG, "Input init OK (BOOT=%d, PWR=%d, touch I2C 0x%02X)", BUTTON_BOOT_PIN, BUTTON_PWR_PIN, (int)TOUCH_I2C_ADDR);
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
    if (!s_touch_ok || s_touch_dev == nullptr) {
        *x = 0; *y = 0;
        return false;
    }
    uint8_t buf[6];
    esp_err_t err = i2c_master_transmit_receive(s_touch_dev, &FT3168_REG_POINTS, 1, buf, sizeof(buf), 50);
    if (err != ESP_OK) {
        *x = 0; *y = 0;
        return false;
    }
    int points = buf[0] & 0x0F;
    if (points < 1) {
        *x = 0; *y = 0;
        return false;
    }
    /* P1: X = (XH&0x0F)<<8 | XL, Y = (YH&0x0F)<<8 | YL */
    int raw_x = ((buf[1] & 0x0F) << 8) | buf[2];
    int raw_y = ((buf[3] & 0x0F) << 8) | buf[4];
    *x = raw_x * (SCREEN_WIDTH - 1) / 4095;
    *y = raw_y * (SCREEN_HEIGHT - 1) / 4095;
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
    /* BOOT = GPIO0, active low; treat as "button pressed" when low */
    return gpio_get_level((gpio_num_t)BUTTON_PIN) == 0;
}
