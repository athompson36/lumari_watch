# Waveshare ESP32-S3-Touch-AMOLED-2.06 — Board & Firmware Context

This document consolidates hardware and firmware information from the official Waveshare wiki, Espressif BSP, community OLEDS3Watch firmware, and Home Assistant notes so we have a single reference for every pertinent aspect of the board.

---

## 1. Board summary (from Waveshare wiki)

- **SoC:** ESP32-S3R8 (Xtensa LX7 dual-core, 240 MHz), 512KB SRAM, 384KB ROM, **8MB PSRAM** (octal/OPI; use `CONFIG_SPIRAM_MODE_OCT=y`, not quad), **32MB external Flash**.
- **Connectivity:** 2.4 GHz Wi-Fi (802.11 b/g/n), Bluetooth 5 (BLE), onboard antenna, **Type-C USB**.
- **Display:** 2.06″ capacitive touch AMOLED, **410×502**, 16.7M colors. **CO5300** driver over **QSPI**; **FT3168** capacitive touch over **I2C** (10–400 kHz).
- **Sensors / peripherals:** **QMI8658** 6-axis IMU (accel + gyro); **PCF85063** RTC (battery-backed via AXP2101); **AXP2101** PMIC (power, charging, battery); **ES8311** audio codec (playback); **ES7210** ADC (microphone); **TF card** slot (SDMMC).
- **Buttons:** **BOOT** and **PWR** (customizable).
- **Power:** 3.7 V MX1.25 battery header; AXP2101 for charging and multiple rails.

Wiki: [ESP32-S3-Touch-AMOLED-2.06](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06).  
**Factory firmware, docs and input troubleshooting:** [WAVESHARE_FACTORY_AND_TROUBLESHOOTING.md](WAVESHARE_FACTORY_AND_TROUBLESHOOTING.md).

---

## 2. Pinout (authoritative)

Pin assignments below are taken from the **joaquimorg/esp32_s3_touch_amoled_2_06** BSP header (`esp32_s3_touch_amoled_2_06.h`) and match the **Lumari** config and typical schematic usage. The wiki’s “Pinout Definition” and “Schematic Diagram” sections are the official source; use them to resolve any doubt.

| Function | GPIO / value | Notes |
|----------|--------------|--------|
| **I2C (touch, IMU, RTC, codecs)** | | |
| I2C SDA | **15** | Shared bus |
| I2C SCL | **14** | 100–400 kHz |
| **Display (CO5300 QSPI)** | | |
| LCD_CS | **12** | |
| LCD_PCLK (SCK) | **11** | |
| LCD_DATA0–3 | **4, 5, 6, 7** | QSPI data |
| LCD_RST | **8** | Panel reset |
| Backlight | — | No GPIO; brightness via CO5300 command **0x51** |
| **Touch (FT3168 I2C)** | | |
| Touch RST | **9** | |
| Touch INT | **38** | |
| I2C address | **0x38** | |
| **Buttons** | | |
| BOOT | **0** | Active low |
| PWR | **10** | Active high (or via AXP2101 PWR key) |
| **uSD (TF) card (SDMMC 1-line)** | | |
| SD_CLK | **2** | |
| SD_CMD | **1** | |
| SD_D0 | **3** | |
| **I2S (audio)** | | |
| I2S_MCLK | **16** | |
| I2S_BCLK (SCLK) | **41** | |
| I2S_LCLK (WS) | **45** | |
| I2S_DOUT | **40** | |
| I2S_DIN (DSIN) | **42** | |
| Power amp enable | **46** | |
| **Other** | | |
| ES8311 (speaker) | I2C, default addr | On same I2C bus |
| ES7210 (mic) | I2C, default addr | On same I2C bus |

Lumari uses: display QSPI, touch RST/INT, I2C 14/15, BOOT/PWR; no TF or I2S in current HAL.

---

## 3. Display (CO5300 QSPI)

- **Interface:** QSPI (CS, PCLK, DATA0–3). Same pins as in table above.
- **Driver:** Wiki and Lumari use **CO5300**. Espressif provides **`espressif/esp_lcd_co5300`** (e.g. v2.0.3) for ESP-IDF. Some BSPs (e.g. joaquimorg’s) use **`waveshare/esp_lcd_sh8601`** and SH8601 init commands; the 2.06″ panel may share a compatible command set or the BSP may be written for a sibling panel. For our project, **CO5300** and **esp_lcd_co5300** are the reference.
- **Brightness:** Write to CO5300 register **0x51** (1 byte, 0–255). No LEDC/GPIO backlight.
- **Sleep / wake:**  
  - Sleep: display off then **0x10** (sleep in); optionally gate panel rails (ALDO) if using AXP2101.  
  - Wake: **0x11** (sleep out), delay ≥120 ms, then display on; if rails were gated, re-enable and optionally re-init panel.
- **Resolution:** 410×502; RGB565 typical. Rounding to even coordinates (e.g. for 2-pixel alignment) is used in some BSPs to avoid tearing.

---

## 4. Touch (FT3168 I2C)

- **Chip:** FT3168, self-capacitance, I2C **0x38**, 10–400 kHz.
- **Pins:** RST=9, INT=38; SDA/SCL=15/14 (shared I2C).
- **Driver:** Official/community BSPs often use **`espressif/esp_lcd_touch_ft5x06`**; FT3168 is compatible enough with the FT5x06 I2C interface for that driver to work. Lumari uses a custom FT3168 path in the input HAL; for BSP adoption, FT5x06 driver is the reference.

---

## 5. IMU (QMI8658) and RTC (PCF85063)

- **QMI8658:** 6-axis (accel + gyro), same I2C bus (e.g. 0x6B). Used for gesture, step count, etc. Lumari uses **`waveshare/qmi8658`**; step delta and wrist detection are implemented in `imu_service`.
- **PCF85063:** RTC at **0x51** on same I2C, battery-backed via AXP2101. Lumari uses it in `time_service` (BCD get/set).

---

## 6. Power (AXP2101)

- PMIC: charging, battery, multiple DC/ALDO rails. Panel power can be gated via ALDO1–ALDO4 for deep sleep.
- **PWR key:** Short-press can be detected via PMU IRQ or polling (e.g. `bsp_power_poll_pwr_button_short()` in joaquimorg BSP).
- **Events:** VBUS insert/remove, charge start/done. Optional background monitor if no IRQ pin.

---

## 7. Audio (ES8311 / ES7210)

- **ES8311:** Speaker/playback codec on I2C; output via I2S (MCLK, BCLK, LCLK, DOUT); power amp on GPIO 46.
- **ES7210:** Microphone ADC on I2C; input via same I2S (DIN).  
Lumari does not use audio in the current HAL.

---

## 8. TF card (SDMMC)

- 1-line SDMMC: CLK=2, CMD=1, D0=3. Mount point and format-on-fail are BSP Kconfig options. Lumari does not use TF in the current HAL.

---

## 9. Official Waveshare BSP (Espressif registry)

- **Component:** **waveshare/esp32_s3_touch_amoled_2_06** (e.g. v1.0.6).  
- **Add:** `idf.py add-dependency "waveshare/esp32_s3_touch_amoled_2_06^1.0.6"`  
- **Target:** esp32s3. **HW version:** V1.0 per wiki.  
- **Dependencies (from registry):** ESP-IDF ≥5.3, `espressif/esp_lcd_panel_io_additions`, `espressif/esp_lcd_touch_ft5x06`, `espressif/esp_codec_dev`, `lvgl/lvgl` (≥8,<10), `espressif/esp_lvgl_port`, and **waveshare/esp_lcd_sh8601**. So the **published** BSP uses SH8601 + FT5x06; for a CO5300-only build (like Lumari), we keep **esp_lcd_co5300** and our HAL or a fork of the BSP with CO5300.

---

## 10. Community firmware: OLEDS3Watch

- **Repo:** [joaquimorg/OLEDS3Watch](https://github.com/joaquimorg/OLEDS3Watch).  
- **Stack:** ESP-IDF v5.x, **ESP-Brookesia** (phone-style GUI), LVGL 9.2, **waveshare/qmi8658**.  
- **Board component:** Local/vendored **esp32_s3_touch_amoled_2_06** (from joaquimorg/esp32_s3_touch_amoled_2_06), which uses **waveshare/esp_lcd_sh8601** and **esp_lcd_touch_ft5x06**, plus **esp_codec_dev**, **esp_lvgl_port**, LVGL.  
- **Main flow:** `bsp_extra_init()` → `bsp_display_start_with_config()` (LVGL + display + touch) → Brookesia phone (stylesheets 410×502), apps (Clock, Notifications, Calculator, 2048, Settings), status bar clock timer.  
- **Display init:** SH8601 QSPI bus (same pins as our table), SH8601 init commands, brightness 0x51, sleep 0x10 / wake 0x11; optional ALDO gating for panel power.  
- **Touch:** FT5x06 on shared I2C.  
- **Use for Lumari:** Pinout and sleep/wake/brightness sequences align; we can adopt BSP-style display/touch init or keep our CO5300 + FT3168 HAL and only reuse patterns (e.g. rounder, buffer sizes).

---

## 11. Arduino demos (Waveshare wiki)

- **Libraries:** GFX_Library_for_Arduino (CO5300), Arduino_DriveBus (touch; wiki lists CST816 but board is FT3168), SensorLib (PCF85063, QMI8658), XPowersLib (AXP2101), LVGL, Mylibrary (pin macros).  
- **Demos:** HelloWorld (CO5300 GFX), AsciiTable, PCF85063 simple time, QMI8658 accel chart, AXP2101 ADC data (brightness 0–255 via `Display_Brightness`), LVGL widgets, SD test (TF), ES8311 audio.  
- **TF (Arduino):** SPI-style pins in wiki: CS=17, DI(MOSI)=1, DO(MISO)=3, SCK=2 (different from SDMMC 1-line above; confirm which interface the board uses).  
- **Takeaway:** Brightness and sleep/wake behavior match our CO5300 usage; I2C shared for RTC/IMU/touch.

---

## 12. Home Assistant thread

- **Thread:** [Waveshare ESP32-S3-Touch-AMOLED-2.06 Watch](https://community.home-assistant.io/t/waveshare-esp32-s3-touch-amoled-2-06-watch/914798).  
- **Content (from fetch):** Short question whether anyone is using the board; link to Waveshare product page. No detailed display init or register-level notes in the captured snippet. Use for community follow-ups and links, not for register-level details.

---

## 13. Lumari alignment

- **Pins:** Matches the table in §2 (QSPI 12,11,4–7,8; touch 9,38; I2C 15,14; BOOT 0, PWR 10).  
- **Display:** **esp_lcd_co5300**, brightness 0x51, sleep/wake in `display_hal`.  
- **Touch:** Custom FT3168 I2C in input HAL; can be swapped to BSP FT5x06 if we adopt the full BSP.  
- **IMU/RTC:** QMI8658 and PCF85063 on shared I2C; `imu_service` / `time_service` already use that bus.  
- **No audio/TF in HAL yet;** when added, use §7 and §8 pins and BSP/Kconfig where applicable.

---

## 14. Quick reference: commands and addresses

| Item | Value / command |
|------|------------------|
| Touch I2C addr | 0x38 |
| RTC (PCF85063) I2C addr | 0x51 |
| QMI8658 I2C addr | 0x6B (typical) |
| Brightness (CO5300) | Register 0x51, 1 byte 0–255 |
| Display sleep | 0x10 (after display off) |
| Display wake | 0x11, then delay ≥120 ms, then display on |
| I2C speed | 400 kHz typical |

---

## 15. QEMU emulator configuration

The emulator is configured to match the **Waveshare ESP32-S3-Touch-AMOLED-2.06** hardware (this document and [Waveshare wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06)) where possible when running under ESP-IDF's QEMU (Espressif fork, esp32s3 machine).

| Item | Hardware (datasheet / wiki) | QEMU emulator |
|------|-----------------------------|---------------|
| **SoC** | ESP32-S3R8, 512KB SRAM, 8MB PSRAM, 32MB Flash | ESP32-S3; **no PSRAM** (CONFIG_SPIRAM=n in sdkconfig.defaults.qemu) so framebuffer must fit in internal RAM. |
| **Display resolution** | **410×502** (2.06" AMOLED, CO5300 QSPI, RGB565) | **198×240** — same **aspect ratio** (410:502 ≈ 0.817) so UI layout matches; reduced size for RAM. |
| **Display interface** | CO5300 over QSPI | Virtual RGB panel (esp_lcd_qemu_rgb); `idf.py qemu --graphics` opens SDL window. |
| **Color format** | RGB565 | RGB565 (RGB_QEMU_BPP_16). |
| **Touch** | FT3168 I2C 0x38, RST=9, INT=38 | Stubbed (no touch in QEMU). |
| **Buttons** | BOOT=GPIO0, PWR=GPIO10 | BOOT pin defined for build; input stubbed. |
| **IMU / RTC** | QMI8658, PCF85063 on I2C 14/15 | Stubbed. |

**Run:** `./run_qemu.sh` (sources IDF, ensures QEMU build with `sdkconfig.defaults` + `sdkconfig.defaults.qemu`, then `idf.py qemu --graphics`). Exit: Ctrl-A then q.

**QEMU keyboard / button mapping**

The menu footer shows **L R** and **BTN CLOSE** — these are the hardware control labels on the real watch:

| Label | Hardware | In Lumari | In QEMU |
|-------|----------|-----------|---------|
| **L** | Left side button | Not used yet | — |
| **R** | Right side button | Not used yet | — |
| **BTN** | BOOT button (GPIO0, active low) | Short press: toggle menu. Long press (≈800 ms): open menu. | Firmware reads GPIO0; standard ESP-IDF QEMU does **not** expose a keyboard key for GPIO0, so the button cannot be simulated from the host unless you add custom handling (e.g. UART or QEMU GPIO hook). |
| **CLOSE** | Same as BTN when menu is open | Short press closes menu | Same as BTN. |

**Touch:** On hardware, tap the screen to pick EQUIP, LORE, CRAFT AURA, etc. In QEMU touch is **stubbed** (no touch input), so menu items cannot be selected unless the project adds mouse→touch or key→action mapping for the emulator.

**Exit QEMU:** Focus the terminal where QEMU is running, press **Ctrl-A**, then type **q** and Enter.

**No GUI with prebuilt QEMU:** The Espressif prebuilt QEMU (from `idf_tools.py install qemu-xtensa`) does **not** map the virtual framebuffer device at `0x21000000`, so no SDL window appears (stub mode, serial only).

**GUI with custom-built QEMU:** Build QEMU from [Espressif’s fork](https://github.com/espressif/qemu) (branch `esp-develop`), then use it so the SDL window shows:

```bash
./scripts/build_qemu_from_fork.sh          # clone, apply display fix patch, build (default: $HOME/esp/qemu-espressif)
export LUMARI_QEMU_BUILD_DIR="$HOME/esp/qemu-espressif"
./run_qemu.sh                              # uses custom QEMU and rebuilds with real framebuffer
```

**Black screen fix:** The upstream esp-develop RGB device uses subregion address spaces with offset addressing; the guest framebuffer at `0x20000000` must be translated to a region offset. The project applies `scripts/patches/qemu-esp_rgb-address-translation.patch` automatically when you run `build_qemu_from_fork.sh`. Without this patch, the SDL window can open but stay black.

**Dependencies:** System libs for QEMU (SDL2, libgcrypt, glib, pixman, libslirp) and optional QEMU binaries: see [docs/DEPENDENCIES.md](DEPENDENCIES.md) §2 and §5.

---

This file is the single place to look for a full picture of the board’s hardware and how official/community firmware and Lumari use it. For links and a short “how to use” workflow, see [HARDWARE_REFERENCES.md](HARDWARE_REFERENCES.md).
