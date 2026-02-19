#include "time_service.h"
#include "esp_log.h"

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "input_hal.h"
#include "driver/i2c_master.h"
#endif

static const char *TAG = "time_service";

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#define PCF85063_I2C_ADDR  0x51
#define PCF85063_REG_SEC   0x04
#define PCF85063_OS_BIT    0x80

static i2c_master_dev_handle_t s_rtc_dev = nullptr;
static bool s_rtc_ok = false;

static uint8_t bcd_to_byte(uint8_t bcd) { return (bcd >> 4) * 10 + (bcd & 0x0F); }
static uint8_t byte_to_bcd(uint8_t v)   { return ((v / 10) << 4) | (v % 10); }
#endif

void time_service_init(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    i2c_master_bus_handle_t bus = input_hal_get_i2c_bus();
    if (bus == nullptr) {
        ESP_LOGW(TAG, "No I2C bus for RTC");
        return;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = PCF85063_I2C_ADDR,
        .scl_speed_hz    = 100000,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &s_rtc_dev);
    s_rtc_ok = (err == ESP_OK);
    if (s_rtc_ok) {
        time_service_datetime_t dt;
        if (time_service_get(&dt))
            ESP_LOGI(TAG, "RTC OK %04u-%02u-%02u %02u:%02u:%02u",
                     (unsigned)dt.year, (unsigned)dt.month, (unsigned)dt.day,
                     (unsigned)dt.hour, (unsigned)dt.min, (unsigned)dt.sec);
        else
            ESP_LOGW(TAG, "RTC read failed or oscillator was stopped");
    } else
        ESP_LOGW(TAG, "RTC init failed (%s)", esp_err_to_name(err));
#else
    (void)TAG;
#endif
}

bool time_service_get(time_service_datetime_t *out)
{
    if (out == nullptr) return false;
    out->sec = 0; out->min = 0; out->hour = 0;
    out->day = 1; out->month = 1; out->year = 2025;

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_rtc_ok || s_rtc_dev == nullptr) return false;
    uint8_t reg = PCF85063_REG_SEC;
    uint8_t buf[7];
    esp_err_t err = i2c_master_transmit_receive(s_rtc_dev, &reg, 1, buf, sizeof(buf), 50);
    if (err != ESP_OK) return false;
    if (buf[0] & PCF85063_OS_BIT) return false;
    out->sec   = bcd_to_byte(buf[0] & 0x7F);
    out->min   = bcd_to_byte(buf[1] & 0x7F);
    out->hour  = bcd_to_byte(buf[2] & 0x3F);
    out->day   = bcd_to_byte(buf[3] & 0x3F);
    out->month = bcd_to_byte(buf[4] & 0x1F);
    out->year  = (uint16_t)bcd_to_byte(buf[5]) + 2000U;
#endif
    return true;
}

bool time_service_set(const time_service_datetime_t *dt)
{
    if (dt == nullptr) return false;
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_rtc_ok || s_rtc_dev == nullptr) return false;
    uint8_t buf[8];
    buf[0] = PCF85063_REG_SEC;
    buf[1] = byte_to_bcd(dt->sec % 60);
    buf[2] = byte_to_bcd(dt->min % 60);
    buf[3] = byte_to_bcd(dt->hour % 24);
    buf[4] = byte_to_bcd(dt->day < 1 ? 1 : (dt->day > 31 ? 31 : dt->day));
    buf[5] = byte_to_bcd(dt->month < 1 ? 1 : (dt->month > 12 ? 12 : dt->month));
    buf[6] = byte_to_bcd((uint8_t)((dt->year >= 2000 ? dt->year - 2000 : 0) % 100));
    esp_err_t err = i2c_master_transmit(s_rtc_dev, buf, sizeof(buf), 50);
    return (err == ESP_OK);
#else
    (void)dt;
    return false;
#endif
}

bool time_service_ok(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    return s_rtc_ok;
#else
    return false;
#endif
}
