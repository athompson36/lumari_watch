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

**Hardware & BSP links:** [docs/HARDWARE_REFERENCES.md](docs/HARDWARE_REFERENCES.md) — Waveshare wiki, official BSP, OLEDS3Watch, and community display/touch notes. **Factory firmware and input troubleshooting:** [docs/WAVESHARE_FACTORY_AND_TROUBLESHOOTING.md](docs/WAVESHARE_FACTORY_AND_TROUBLESHOOTING.md).

## Requirements

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/get-started.html) (v5.x recommended)
- For Waveshare 2.06: **ESP32-S3** target, 8MB PSRAM

**Full audit and one-shot install:** See **[docs/DEPENDENCIES.md](docs/DEPENDENCIES.md)** for the complete dependency list. To install ESP-IDF and system deps (macOS or Linux), run from the repo root:

```bash
./scripts/install_dependencies.sh
```

## Project structure

```
lumari_watch/
├── main/                 # App entry, main loop (render + creature update)
├── components/
│   ├── config/           # lumari_config.h (resolution, FPS)
│   ├── creature/         # Creature logic and rendering hook
│   ├── engine/           # Render engine, framebuffer, layers, sprites
│   ├── display_hal/      # Display HAL (init, flush)
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

**Blank screen or device seems stuck in bootloader after flash:**  
The board may still be in download mode. **Release the BOOT button** (do not hold it), then **press Reset (or PWR) once**. It should then boot the app from flash. If the screen stays blank, run `idf.py -p /dev/tty.usbmodem* monitor` in a terminal, press Reset on the device, and check the serial output: bootloader messages mean it didn’t leave download mode; app logs mean the firmware is running and the issue may be display init or wiring.

Dependencies (CO5300 display, QMI8658 IMU) are pulled via the component manager from `idf_component.yml` in `components/display_hal` and `components/services`.

### Run in QEMU (no hardware)

You can run the firmware in Espressif’s QEMU emulator and see the display in a window. Touch, IMU, and RTC are stubbed.

**1. Install QEMU (once)**

```bash
# From your ESP-IDF directory
python $IDF_PATH/tools/idf_tools.py install qemu-xtensa qemu-riscv32
. $IDF_PATH/export.sh
```

On Linux (e.g. Ubuntu) install: `libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0`.  
On macOS: `brew install libgcrypt glib pixman sdl2 libslirp` (full list: [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md)).

**2. Build for QEMU**

Either use the QEMU defaults file (no menuconfig):

```bash
idf.py set-target esp32s3
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.qemu" idf.py build
```

Or enable in menuconfig: `idf.py menuconfig` → **Lumari** → `[*] Build for QEMU emulator`, then `idf.py build`.

**3. Run with graphics**

```bash
idf.py qemu --graphics monitor
```

With a QEMU build that includes the virtual framebuffer device, a window would show the display. The **Espressif prebuilt QEMU** (from `idf_tools.py install qemu-xtensa`) does **not** include that device, so **no SDL window appears** — the app runs and serial output appears in the terminal only. Exit QEMU with Ctrl-A then `q`.

**If the window doesn’t open:**

1. **Run from a normal terminal** (not a background/IDE task) so the SDL window can open:
   ```bash
   cd /path/to/lumari_watch
   . $IDF_PATH/export.sh
   ./run_qemu.sh
   ```
   Or run `idf.py qemu --graphics monitor` yourself after sourcing IDF.

2. **Use a QEMU build.** The firmware must be built with the QEMU board config or the virtual display won’t be used. Rebuild once (source ESP-IDF first; see item 5 below if `idf.py` is not found):
   ```bash
   SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.qemu" idf.py build
   ```
   Then run `idf.py qemu --graphics monitor`. The script `run_qemu.sh` does this check and rebuild for you.

3. **Install SDL2** (required for the graphics window). On macOS: `brew install sdl2`. On Ubuntu/Debian: `sudo apt-get install libsdl2-2.0-0`.

4. **Build keeps re-running CMake (loop):** Avoid `idf.py -v build`; it can trigger repeated reconfiguration. If the build loops, do a full clean and build once: `rm -rf build managed_components` then `./run_qemu.sh` or (after sourcing IDF) `SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.qemu" idf.py build`. If you see the CO5300 "include directory is not a directory" error, run `./scripts/fix_co5300_headers.sh` once, then (in the same shell) source IDF and build again.

5. **`idf.py: command not found`:** Source the ESP-IDF environment first in the same terminal: `source "$(cat .idf_path)/export.sh"` or `. $IDF_PATH/export.sh`. Or use `./run_qemu.sh`, which sources IDF for you.

6. **"Timed out waiting for port 5555 to be open":** `run_qemu.sh` runs QEMU in the foreground (serial in the same terminal) so the port is not used and the timeout is avoided. To use monitor on a separate port, run `idf.py qemu --graphics monitor` from a real terminal (not over SSH); if it still times out, use `./run_qemu.sh` instead.

## Configuration

- **Screen:** `SCREEN_WIDTH`, `SCREEN_HEIGHT` in `components/config/lumari_config.h`
- **FPS:** `TARGET_FPS_IDLE`, `TARGET_FPS_ACTIVE` in the same file
- **System:** `sdkconfig.defaults` (SPIRAM, LCD panel SPI, main task stack)

## License

See [LICENSE](LICENSE) in the repository root.
