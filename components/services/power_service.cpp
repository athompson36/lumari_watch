#include "power_service.h"
#include "lumari_config.h"
#include "esp_log.h"

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "input_hal.h"
#include "driver/i2c_master.h"
#endif

static const char *TAG = "power_service";

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#define AXP2101_I2C_ADDR        0x34
#define AXP2101_REG_PMU_STATUS1 0x00
#define AXP2101_REG_PMU_STATUS2 0x01
#define AXP2101_REG_ADC_EN      0x30
#define AXP2101_REG_VBAT_H      0x34
#define AXP2101_REG_VBAT_L      0x35
#define AXP2101_REG_BAT_PERCENT 0xA4
#define AXP2101_REG_LDO_ON_OFF0 0x90
/* PWR key short-press: enable in INTEN2 (0x41) bit 1, status in INTSTS2 (0x49) bit 1 (BSP convention) */
#define AXP2101_REG_INTEN2  0x41
#define AXP2101_REG_INTSTS2 0x49
#define AXP2101_PKEY_SHORT_BIT  (1u << 1)

/** Vbat ADC LSB = 1.1 mV (typical AXP2101). */
#define AXP2101_VBAT_LSB_MV     1100
#define AXP2101_VBAT_LSB_DIV    1000

static i2c_master_dev_handle_t s_axp_dev = nullptr;
static bool s_power_ok = false;

static bool axp_read_u8(uint8_t reg, uint8_t *val)
{
    if (s_axp_dev == nullptr || val == nullptr) return false;
    uint8_t buf[1];
    esp_err_t err = i2c_master_transmit_receive(s_axp_dev, &reg, 1, buf, sizeof(buf), 50);
    if (err != ESP_OK) return false;
    *val = buf[0];
    return true;
}

static bool axp_read_u16(uint8_t reg, uint16_t *val)
{
    if (s_axp_dev == nullptr || val == nullptr) return false;
    uint8_t buf[2];
    esp_err_t err = i2c_master_transmit_receive(s_axp_dev, &reg, 1, buf, sizeof(buf), 50);
    if (err != ESP_OK) return false;
    *val = (uint16_t)((buf[0] << 8) | buf[1]);
    return true;
}

static bool axp_write_u8(uint8_t reg, uint8_t val)
{
    if (s_axp_dev == nullptr) return false;
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_axp_dev, buf, sizeof(buf), 50) == ESP_OK;
}

static power_charge_state_t status2_to_charge_state(uint8_t st1, uint8_t st2)
{
    (void)st1;
    unsigned charging_status = st2 & 7u;
    unsigned direction = (st2 >> 5) & 3u;
    if (direction == 2) return POWER_CHARGE_STATE_DISCHARGING;
    if (direction == 0) return POWER_CHARGE_STATE_STANDBY;
    switch (charging_status) {
        case 0: return POWER_CHARGE_STATE_TRICKLE;
        case 1: return POWER_CHARGE_STATE_PRE_CHARGE;
        case 2: return POWER_CHARGE_STATE_CC;
        case 3: return POWER_CHARGE_STATE_CV;
        case 4: return POWER_CHARGE_STATE_DONE;
        case 5: return POWER_CHARGE_STATE_NOT_CHARGING;
        default: return POWER_CHARGE_STATE_UNKNOWN;
    }
}
#endif

void power_service_init(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    i2c_master_bus_handle_t bus = input_hal_get_i2c_bus();
    if (bus == nullptr) {
        ESP_LOGW(TAG, "No I2C bus for AXP2101");
        return;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = AXP2101_I2C_ADDR,
        .scl_speed_hz    = 400000,
        .scl_wait_us     = 0,
        .flags           = {},
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &s_axp_dev);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 init failed (%s)", esp_err_to_name(err));
        return;
    }
    s_power_ok = true;
    /* Enable battery voltage ADC (bit 0 = VBAT in AdcChannelEnableControl). */
    (void)axp_write_u8(AXP2101_REG_ADC_EN, 0x01);
    /* Enable PWR key short-press IRQ so we can poll it as button (Waveshare PWR is on PMIC, not GPIO10). */
    (void)axp_write_u8(AXP2101_REG_INTEN2, AXP2101_PKEY_SHORT_BIT);
    uint8_t st1 = 0, st2 = 0;
    if (axp_read_u8(AXP2101_REG_PMU_STATUS1, &st1) && axp_read_u8(AXP2101_REG_PMU_STATUS2, &st2)) {
        ESP_LOGI(TAG, "AXP2101 OK (status1=0x%02x status2=0x%02x)", (unsigned)st1, (unsigned)st2);
    }
#else
    (void)TAG;
#endif
}

bool power_service_get_battery_mv(uint32_t *out_mv)
{
    if (out_mv == nullptr) return false;
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok) return false;
    uint16_t raw = 0;
    if (!axp_read_u16(AXP2101_REG_VBAT_H, &raw)) return false;
    /* 16-bit ADC; LSB = 1.1 mV (AXP2101 typical). */
    *out_mv = (uint32_t)raw * AXP2101_VBAT_LSB_MV / AXP2101_VBAT_LSB_DIV;
    return true;
#else
    (void)out_mv;
    return false;
#endif
}

bool power_service_get_charge_state(power_charge_state_t *out_state)
{
    if (out_state == nullptr) return false;
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok) return false;
    uint8_t st1 = 0, st2 = 0;
    if (!axp_read_u8(AXP2101_REG_PMU_STATUS1, &st1) || !axp_read_u8(AXP2101_REG_PMU_STATUS2, &st2))
        return false;
    *out_state = status2_to_charge_state(st1, st2);
    return true;
#else
    (void)out_state;
    return false;
#endif
}

bool power_service_get_battery_percent(uint8_t *out_percent)
{
    if (out_percent == nullptr) return false;
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok) return false;
    uint8_t val = 0;
    if (!axp_read_u8(AXP2101_REG_BAT_PERCENT, &val)) return false;
    *out_percent = (val > 100) ? 100 : val;
    return true;
#else
    (void)out_percent;
    return false;
#endif
}

bool power_service_get_vbus_present(bool *out_present)
{
    if (out_present == nullptr) return false;
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok) return false;
    uint8_t st1 = 0;
    if (!axp_read_u8(AXP2101_REG_PMU_STATUS1, &st1)) return false;
    *out_present = (st1 & (1u << 5)) != 0;
    return true;
#else
    (void)out_present;
    return false;
#endif
}

bool power_service_poll_pwr_button_short(void)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok || s_axp_dev == nullptr) return false;
    uint8_t v = 0;
    if (!axp_read_u8(AXP2101_REG_INTSTS2, &v)) return false;
    if ((v & AXP2101_PKEY_SHORT_BIT) == 0) return false;
    /* Clear the IRQ by writing the status bit (write 1 to clear on AXP2101). */
    (void)axp_write_u8(AXP2101_REG_INTSTS2, AXP2101_PKEY_SHORT_BIT);
    return true;
#else
    return false;
#endif
}

void power_service_set_aldo_mask(uint8_t enable_mask, uint8_t disable_mask)
{
#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
    if (!s_power_ok || s_axp_dev == nullptr) return;
    uint8_t cur = 0;
    if (!axp_read_u8(AXP2101_REG_LDO_ON_OFF0, &cur)) return;
    cur = (cur & (uint8_t)~disable_mask) | (enable_mask & 0x0Fu);
    (void)axp_write_u8(AXP2101_REG_LDO_ON_OFF0, cur);
#else
    (void)enable_mask;
    (void)disable_mask;
#endif
}
