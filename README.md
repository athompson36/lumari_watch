# Lumari Watch

ESP-IDF firmware for the Lumari watch: a living creature on your wrist with quests, XP, and a companion ecosystem.

## Development

**Current phase:** [Phase 0](ROADMAP.md#-phase-0--hardware-bring-up-12-weeks) done; [Phase 1 — Lumari core](ROADMAP.md#-phase-1--lumari-core-24-weeks) started (seedling + XP stub). Set `LUMARI_RUN_PHASE0` to `0` in `lumari_config.h` to run the Lumari loop instead of the Phase 0 demo.

Full plan: **[ROADMAP.md](ROADMAP.md)** (Phases 0–7: bring-up → Lumari core → core loop → lore → mini-games → iPhone app → polish → expansion).

## Overview

- **Display:** 410×502 RGB565, driven via SPI (ESP LCD + SPIRAM).
- **Rendering:** Framebuffer-backed render engine with layer manager and sprite renderer.
- **Creature:** Creature engine (placeholder sprite) with hooks for future behavior.
- **Time:** Time service stub for future RTC integration.

## Supported hardware

**Primary target:** [Waveshare ESP32-S3-Touch-AMOLED-2.06](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06) (2.06″ 410×502 AMOLED, CO5300 QSPI, FT3168 I2C touch, QMI8658 IMU, PCF85063 RTC, BOOT/PWR buttons). Pinout and drivers are in `components/config/lumari_config.h` behind `LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06`. Set target to `esp32s3` and build.

Other boards can be supported by defining a different board in config and implementing the corresponding HAL paths.

**Hardware & BSP links:** [docs/HARDWARE_REFERENCES.md](docs/HARDWARE_REFERENCES.md) — Waveshare wiki, official BSP, OLEDS3Watch, and community display/touch notes.

## Requirements

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/get-started.html) (v5.x recommended)
- For Waveshare 2.06: **ESP32-S3** target, 8MB PSRAM

## Project structure

```
lumari_watch/
├── main/                 # App entry, main loop (render + creature update)
├── components/
│   ├── config/           # lumari_config.h (resolution, FPS)
│   ├── creature/         # Creature logic and rendering hook
│   ├── engine/           # Render engine, framebuffer, layers, sprites
│   ├── hal/              # Display HAL (init, flush)
│   └── services/         # Time service (stub for RTC)
├── sdkconfig.defaults    # SPIRAM, LCD, main stack size
└── CMakeLists.txt
```

## Build and run

### On real hardware (Waveshare 2.06)

```bash
# Set up ESP-IDF environment (e.g. source export.sh)
idf.py set-target esp32s3   # for Waveshare ESP32-S3-Touch-AMOLED-2.06
idf.py build
idf.py -p /dev/tty.usbmodem* flash monitor
```

Dependencies (CO5300 display, QMI8658 IMU) are pulled via the component manager from `idf_component.yml` in `components/hal` and `components/services`.

### Run in QEMU (no hardware)

You can run the firmware in Espressif’s QEMU emulator and see the display in a window. Touch, IMU, and RTC are stubbed.

**1. Install QEMU (once)**

```bash
# From your ESP-IDF directory
python $IDF_PATH/tools/idf_tools.py install qemu-xtensa qemu-riscv32
. $IDF_PATH/export.sh
```

On Linux (e.g. Ubuntu) install: `libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0`.  
On macOS: `brew install libgcrypt libglib pixman sdl2 libslirp`.

**2. Enable QEMU build**

```bash
idf.py set-target esp32s3
idf.py menuconfig
# → Lumari → [*] Build for QEMU emulator (virtual display, no hardware)
# Save and exit
```

**3. Build and run with graphics**

```bash
idf.py build
idf.py qemu --graphics monitor
```

A window shows the 410×502 virtual display (Phase 0 circle demo or Phase 1 seedling, depending on `LUMARI_RUN_PHASE0`). Console output appears in the terminal. Exit QEMU with Ctrl-A then `q`.

## Configuration

- **Screen:** `SCREEN_WIDTH`, `SCREEN_HEIGHT` in `components/config/lumari_config.h`
- **FPS:** `TARGET_FPS_IDLE`, `TARGET_FPS_ACTIVE` in the same file
- **System:** `sdkconfig.defaults` (SPIRAM, LCD panel SPI, main task stack)

## License

See [LICENSE](LICENSE) in the repository root.
