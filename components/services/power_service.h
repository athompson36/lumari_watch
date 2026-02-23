#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Power / battery service (AXP2101 on Waveshare; stubbed on QEMU). */

typedef enum {
    POWER_CHARGE_STATE_UNKNOWN = 0,
    POWER_CHARGE_STATE_NOT_CHARGING,
    POWER_CHARGE_STATE_TRICKLE,
    POWER_CHARGE_STATE_PRE_CHARGE,
    POWER_CHARGE_STATE_CC,
    POWER_CHARGE_STATE_CV,
    POWER_CHARGE_STATE_DONE,
    POWER_CHARGE_STATE_DISCHARGING,
    POWER_CHARGE_STATE_STANDBY,
} power_charge_state_t;

/** Initialize power service (I2C AXP2101 on Waveshare; no-op on QEMU). */
void power_service_init(void);

/**
 * Read battery voltage in millivolts.
 * @return true if value was read, false on error or unsupported board.
 */
bool power_service_get_battery_mv(uint32_t *out_mv);

/**
 * Read charge state (charging / discharging / standby / done etc.).
 * @return true if state was read, false on error or unsupported board.
 */
bool power_service_get_charge_state(power_charge_state_t *out_state);

/**
 * Read battery percentage (0–100) from fuel gauge, if available.
 * @return true if value was read, false on error or unsupported board.
 */
bool power_service_get_battery_percent(uint8_t *out_percent);

/**
 * Check if VBUS (USB power) is present.
 * @return true if VBUS good, false otherwise or unsupported board.
 */
bool power_service_get_vbus_present(bool *out_present);

/**
 * Poll for PWR key short press (Waveshare: PWR button is on AXP2101 PMIC, not GPIO).
 * Returns true once per detected short press and clears the PMIC IRQ.
 * @return true if a short press was detected this call, false otherwise.
 */
bool power_service_poll_pwr_button_short(void);

/**
 * Enable/disable ALDO rails (for deep sleep panel power gating).
 * Bits 0–3 = ALDO1–ALDO4. Only on Waveshare; no-op on QEMU.
 * @param enable_mask bits to set (1 = enable), 0 = leave unchanged per rail
 * @param disable_mask bits to clear (1 = disable)
 */
void power_service_set_aldo_mask(uint8_t enable_mask, uint8_t disable_mask);
