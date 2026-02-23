# Waveshare ESP32-S3-Touch-AMOLED-2.06 — Factory Firmware, Documentation & Input Troubleshooting

**Product:** ESP32-S3-Touch-AMOLED-2.06 (Mfg P/N: ESP32-S3-Touch-AMOLED-2.06, label e.g. ESP32-S3-Touch-AMOLED-2.06-EN)  
**Brand:** Waveshare  
**Purpose:** Single reference for factory firmware, official docs, and troubleshooting “no input” (touch + buttons).

---

## 1. Factory firmware and demo sources

| Resource | URL / location | Notes |
|----------|----------------|--------|
| **Waveshare Wiki (main)** | https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06 | Pinout, demos, FAQ, resources |
| **Waveshare Wiki (alt)** | https://www.waveshare.net/wiki/ESP32-S3-Touch-AMOLED-2.06 | Same content |
| **GitHub demo (official)** | https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06 | Arduino + ESP-IDF demos; **FirmWare/** contains test firmware to flash |
| **Test firmware path (in demo)** | `ESP32-S3-Touch-AMOLED-2.06-Demo\Firmware` (from wiki) or repo **FirmWare/** | Prebuilt binary to verify board; use [Flash firmware flashing and erasing](https://www.waveshare.com/wiki/Flash_Firmware_Flashing_and_Erasing) |
| **Official BSP (ESP Component Registry)** | `waveshare/esp32_s3_touch_amoled_2_06` ^1.0.6 | Add: `idf.py add-dependency "waveshare/esp32_s3_touch_amoled_2_06^1.0.6"` |
| **BSP source (Waveshare)** | https://github.com/waveshareteam/Waveshare-ESP32-components (path: `bsp/esp32_s3_touch_amoled_2_06`) | Uses SH8601 + **esp_lcd_touch_ft5x06**; different display driver than CO5300 |
| **Community BSP (joaquimorg)** | https://github.com/joaquimorg/esp32_s3_touch_amoled_2_06 | C BSP; SH8601 + FT5x06 touch; AXP2101 PWR key poll; same pins (I2C 14/15, Touch RST 9, INT 38) |
| **Product page** | https://www.waveshare.com/esp32-s3-touch-amoled-2.06.htm | Purchase / overview |

There is **no separate “factory firmware” download** listed on the wiki; the **test firmware in the demo repo (FirmWare/)** is the closest. Use it to confirm touch and buttons work on hardware.

---

## 2. Official documentation and datasheets (from wiki)

| Document | URL |
|----------|-----|
| **Schematic** | https://files.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06/ESP32-S3-Touch-AMOLED-2.06.pdf |
| **Dimensional drawing** | https://files.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06/Esp32-s3-touch-amoled-2_06_dimensions.pdf |
| **FT3168 (touch)** | https://files.waveshare.com/wiki/common/FT3168.pdf |
| **AXP2101 (PMIC)** | https://files.waveshare.com/wiki/common/X-power-AXP2101_SWcharge_V1.0.pdf |
| **PCF85063 (RTC)** | https://files.waveshare.com/wiki/common/PCF85063A.pdf |
| **QMI8658 (IMU)** | https://files.waveshare.com/wiki/common/QMI8658C.pdf |
| **ES8311 / ES7210** | Wiki “Other Components” section (ES8311 user guide, ES7210 datasheet) |
| **ESP32-S3 TRM / datasheet** | Wiki “ESP32-S3” section |

---

## 3. Buttons and touch (from Waveshare wiki FAQ)

- **BOOT**
  - **GPIO0.** Low level = pressed. Used for download mode (hold BOOT then power on). In normal operation: click, double-click, multi-click, long-press.
- **PWR**
  - **Not a direct GPIO.** Wiki: “Under normal working conditions, the action can be judged by the high and low levels of the **EXIO6 detection buttons of the extended IO**, and the **high level is pressed**.” So PWR is read via the **AXP2101 PMIC** (power key / EXIO), not GPIO10. Long-press (e.g. 6 s) powers off; short-press powers on when off.
- **Touch**
  - **FT3168** self-capacitance, I2C **0x38**, 10–400 kHz. RST = GPIO9, INT = GPIO38; SDA/SCL = 15/14 (shared I2C). Arduino demos say “Keep trying to initialize the touch controller FT3168” — init can be retried on failure.

**Implication for Lumari:** PWR must be read via AXP2101 (e.g. `power_service_poll_pwr_button_short()`), not GPIO10. BOOT = GPIO0 low. Touch = FT3168 on I2C 0x38; BSPs use **esp_lcd_touch_ft5x06** (FT3168 is FT5x06-compatible).

---

## 4. BSP vs Lumari (touch and buttons)

| Item | Official/joaquimorg BSP | Lumari |
|------|-------------------------|--------|
| Display | SH8601 (QSPI) | CO5300 (QSPI) |
| Touch driver | **esp_lcd_touch_ft5x06** (full init, reset, read) | Custom FT3168 (reg 0x02 + 6-byte read) |
| Touch RST | BSP_LCD_TOUCH_RST = GPIO 9 | TOUCH_PIN_RST = 9 |
| Touch I2C | Same bus, CONFIG_BSP_I2C_CLK_SPEED_HZ (e.g. 400 kHz) | 400 kHz, 100 ms timeout |
| PWR button | **bsp_power_poll_pwr_button_short()** (AXP2101 INTSTS2 bit 1) | **power_service_poll_pwr_button_short()** (same idea) |
| BOOT button | GPIO0 (BSP has BSP_CAPS_BUTTONS 0; may not expose) | GPIO0 in **input_hal_button_read()** |

So: **PWR** is aligned with factory (PMIC). **Touch** in Lumari uses a minimal FT3168 read; BSP uses the full FT5x06 driver (reset sequence, init, then read). If touch still fails after I2C probe and RST timing, the next step is to use **esp_lcd_touch_ft5x06** for touch.

---

## 5. No-input troubleshooting checklist

Use this after flashing Lumari firmware. Serial monitor: `idf.py -p /dev/tty.usbmodem* monitor`.

### 5.1 Confirm firmware and init

- [ ] **Boot log:** “Input init OK (BOOT GPIO0, PWR=AXP2101, touch 0x38 probe=OK)” or “probe=fail”.
- [ ] **Button level:** “Button level at init: BOOT=%d” — expect 1 when not pressed, 0 when pressed.
- [ ] **AXP2101:** “AXP2101 OK (status1=0x… status2=0x…)”.

If **touch probe=fail**: FT3168 not answering on I2C. Check: RST sequence (GPIO9), I2C pins (14/15), power (same rail as display). If **probe=OK** but no touch in app: try esp_lcd_touch_ft5x06 or add a periodic log of raw touch reg 0x02 + coords.

### 5.2 Buttons

- **BOOT (GPIO0):** If “Button level at init” never changes when you press BOOT, GPIO0 may be wrong or strapping may affect it; confirm on schematic.
- **PWR:** Only works via AXP2101. Ensure **power_service_init()** runs after **input_hal_init()** (I2C bus exists) and that **power_service_poll_pwr_button_short()** is OR’ed in the app (e.g. `btn = input_hal_button_read() || power_service_poll_pwr_button_short()`). One true per short press.

### 5.3 Touch

- **Init order:** Touch RST (GPIO9) before I2C bus creation; then add touch device; then other I2C devices (IMU, RTC, AXP2101).
- **I2C:** 400 kHz, 100 ms timeout. Shared bus with IMU/RTC/PMIC — ensure no driver holds the bus.
- **Protocol:** Reg 0x02 = number of points; 0x03–0x06 = P1 XH, XL, YH, YL. Same as FT5x06.
- **Fallback:** Add **esp_lcd_touch_ft5x06** component and use it for touch (keeps CO5300 for display). BSP does: `esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, ret_touch)` with rst_gpio_num=9, int_gpio_num=38.

### 5.4 Verify hardware with factory/test firmware

1. Download [ESP32-S3-Touch-AMOLED-2.06](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06) → **FirmWare/** (or wiki “test firmware” path).
2. Flash using [Flash firmware flashing and erasing](https://www.waveshare.com/wiki/Flash_Firmware_Flashing_and_Erasing).
3. If **touch and buttons work** with test firmware but not with Lumari → software (init order, driver, or PWR/BOOT handling). If **they do not work** with test firmware → hardware (cable, RST, I2C, or defective unit).

### 5.5 Schematic check

If in doubt on pins, open the schematic PDF (section 2) and confirm:

- Touch: SDA/SCL (14/15), RST (9), INT (38), VCC/GND.
- BOOT: GPIO0.
- PWR: AXP2101 power key (no GPIO10 for PWR on this board).

---

## 6. Summary

- **Factory/test firmware:** In repo [waveshareteam/ESP32-S3-Touch-AMOLED-2.06](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.06) under **FirmWare/**; wiki references “test firmware” in the demo package.
- **Documentation:** Wiki + schematic + datasheets (FT3168, AXP2101, etc.) linked in section 2.
- **Buttons:** BOOT = GPIO0 (low = pressed). PWR = AXP2101 PMIC (poll short-press IRQ); not GPIO10.
- **Touch:** FT3168 @ 0x38, RST=9, INT=38, I2C 14/15. BSP uses **esp_lcd_touch_ft5x06**; Lumari uses custom read — if probe fails or touch still no input, switch to FT5x06 driver or verify hardware with test firmware.

See **BOARD_FIRMWARE_CONTEXT.md** for full pinout and driver notes.
