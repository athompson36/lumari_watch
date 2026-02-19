# Lumari Watch — Dependency audit

Full list of dependencies for building and running the project (hardware and QEMU).

---

## 1. ESP-IDF (required)

| Item | Requirement |
|------|-------------|
| **Framework** | [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/get-started.html) |
| **Version** | **v5.3.2+** required (waveshare/qmi8658); v5.4 or v6.x also supported |
| **Target** | **esp32s3** (Waveshare 2.06" board; QEMU uses same target) |
| **Python** | 3.10+ (3.11+ for EIM offline install) |

ESP-IDF brings: toolchain (xtensa-esp-elf for S3), CMake, Ninja, esptool, idf_tools, Python venv and packages.

---

## 2. System prerequisites (by OS)

### macOS (Homebrew)

| Package | Purpose |
|---------|---------|
| **git** | Clone ESP-IDF and the project |
| **cmake** | Build system |
| **ninja** | Build system |
| **pkg-config** | Configure/link (ESP-IDF and many libs) |
| **dfu-util** | USB DFU (flashing) |
| **python3** | 3.10+ (ESP-IDF scripts and component manager) |
| **libgcrypt** | QEMU runtime |
| **glib** | QEMU runtime |
| **pixman** | QEMU runtime |
| **sdl2** | QEMU graphics window (required for `--graphics`) |
| **libslirp** | QEMU networking (optional) |
| **ccache** | Optional; faster rebuilds |

One-liner (build + QEMU):

```bash
brew install git cmake ninja pkg-config dfu-util python3 libgcrypt glib pixman sdl2 libslirp ccache
```

**Xcode Command Line Tools** are required (e.g. `xcode-select --install` if you see `xcrun: error: invalid active developer path`).

**Apple Silicon (M1/M2):** If you hit toolchain “bad CPU type” errors, install Rosetta 2:  
`/usr/sbin/softwareupdate --install-rosetta --agree-to-license`

### Linux (examples)

**Ubuntu/Debian:**

```bash
sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
# QEMU + graphics
sudo apt-get install -y libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0
```

**Arch:**

```bash
sudo pacman -S --needed gcc git make flex bison gperf python cmake ninja ccache dfu-util libusb libgcrypt glib2 pixman sdl2 libslirp
```

**CentOS (and RHEL-style):**

```bash
sudo yum install -y --enablerepo=powertools libgcrypt glib2 pixman SDL2 libslirp
# Plus standard build tools: git, cmake, ninja-build, python3, etc.
```

---

## 3. ESP-IDF component manager (auto-fetched at build)

These are pulled by `idf.py build` from [components.espressif.com](https://components.espressif.com/) via `idf_component.yml` in the project.

| Component | Registry ID | Used by | Purpose |
|-----------|-------------|---------|---------|
| CO5300 LCD driver | `espressif/esp_lcd_co5300` | hal | Waveshare 2.06" AMOLED (QSPI) |
| QEMU RGB panel | `espressif/esp_lcd_qemu_rgb` | hal | Virtual display in QEMU |
| QMI8658 IMU | `waveshare/qmi8658` | services | 6-axis accel/gyro on Waveshare |

No manual install; first build downloads them into `managed_components/`.

---

## 4. In-tree components (no extra install)

| Component | Path | Deps |
|-----------|------|------|
| config | `components/config` | — |
| display_hal | `components/display_hal` | driver, esp_lcd, esp_lcd_co5300, esp_lcd_qemu_rgb |
| input | `components/input` | driver, config |
| services | `components/services` | config, input, qmi8658 |
| creature | `components/creature` | engine |
| engine | `components/engine` | display_hal, config, creature, quest |
| quest | `components/quest` | config, creature |

---

## 5. QEMU (optional; for emulator)

- **Prebuilt** (serial only; no SDL display window with current prebuilt):

  ```bash
  python $IDF_PATH/tools/idf_tools.py install qemu-xtensa qemu-riscv32
  . $IDF_PATH/export.sh
  ```

- **Custom build from Espressif fork** (for SDL display window): clone, apply display fix, and build, then set `LUMARI_QEMU_BUILD_DIR`:

  ```bash
  ./scripts/build_qemu_from_fork.sh        # uses $HOME/esp/qemu-espressif by default; applies patches/
  export LUMARI_QEMU_BUILD_DIR=$HOME/esp/qemu-espressif
  ./run_qemu.sh
  ```

  The script applies `scripts/patches/qemu-esp_rgb-address-translation.patch` so the SDL window shows framebuffer content (without it, the window can open but stay black). Prerequisites: same as §2 (sdl2, libgcrypt, glib, pixman, libslirp, pkg-config, ninja). See [esp-toolchain-docs qemu/esp32s3](https://github.com/espressif/esp-toolchain-docs/blob/main/qemu/esp32s3/README.md).

- **System libs** for the graphics window: see §2 (sdl2, libgcrypt, glib, pixman, libslirp).

---

## 6. Summary checklist

| # | Dependency | How to install |
|---|------------|-----------------|
| 1 | Homebrew (macOS) or apt/pacman/yum (Linux) | [brew.sh](https://brew.sh) / system package manager |
| 2 | System packages (git, cmake, ninja, pkg-config, python3, sdl2, QEMU libs, …) | See §2 for your OS |
| 3 | ESP-IDF | See §7 below |
| 4 | QEMU binaries (optional; for emulator) | `idf_tools.py install qemu-xtensa qemu-riscv32` |
| 5 | Project component deps (CO5300, QEMU RGB, QMI8658) | Fetched at first `idf.py build` |

**QEMU graphics:** For `idf.py qemu --graphics` (or `./run_qemu.sh`) you need the QEMU system libs in §2 (especially **sdl2** on macOS/Linux) and the QEMU binaries from step 4.

---

## 7. Installing ESP-IDF

### Option A — EIM (recommended on macOS for v6.0+)

```bash
brew tap espressif/eim
brew install --cask eim-gui   # or: brew install eim
eim install                   # or: eim wizard
```

Then add ESP-IDF to PATH (EIM will show the command; often something like `. ~/esp/esp-idf/export.sh` or the path EIM used).

### Option B — Legacy (v5.x, clone + install)

```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.3.2 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh
```

Use **v5.3.2** or newer (required by `waveshare/qmi8658`). After that, in every new terminal where you build Lumari:

```bash
. ~/esp/esp-idf/export.sh
```

To have `run_qemu.sh` find ESP-IDF automatically, create a project-local path file:

```bash
echo "$HOME/esp/esp-idf" > .idf_path
```

---

## 8. Verify install

```bash
# ESP-IDF in PATH
idf.py --version

# Target
idf.py set-target esp32s3

# Build (fetches component deps)
cd /path/to/lumari_watch
idf.py build

# Optional: QEMU with display
SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.qemu" idf.py build
idf.py qemu --graphics monitor
```

---

## 9. References

- [ESP-IDF Get Started (ESP32-S3)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html)
- [ESP-IDF macOS setup (EIM)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/macos-setup.html)
- [ESP-IDF Legacy setup (Linux/macOS)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/linux-macos-setup-legacy.html)
- [QEMU for ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/tools/qemu.html)
