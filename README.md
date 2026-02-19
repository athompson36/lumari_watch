# Lumari Watch

ESP-IDF firmware for the Lumari watch: creature/rendering engine, display HAL, and time service on an LCD panel over SPI.

## Overview

- **Display:** 410×502 RGB565, driven via SPI (ESP LCD + SPIRAM).
- **Rendering:** Framebuffer-backed render engine with layer manager and sprite renderer.
- **Creature:** Creature engine (placeholder sprite) with hooks for future behavior.
- **Time:** Time service stub for future RTC integration.

## Requirements

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/get-started.html) (v5.x recommended)
- Target: ESP32 with SPIRAM and SPI LCD support

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

```bash
# Set up ESP-IDF environment (e.g. source export.sh)
idf.py set-target esp32   # or your board target
idf.py build
idf.py -p <PORT> flash monitor
```

## Configuration

- **Screen:** `SCREEN_WIDTH`, `SCREEN_HEIGHT` in `components/config/lumari_config.h`
- **FPS:** `TARGET_FPS_IDLE`, `TARGET_FPS_ACTIVE` in the same file
- **System:** `sdkconfig.defaults` (SPIRAM, LCD panel SPI, main task stack)

## License

See [LICENSE](LICENSE) in the repository root.
