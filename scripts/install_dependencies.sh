#!/usr/bin/env bash
# Install system deps, ESP-IDF, and QEMU for Lumari Watch on macOS (and optionally Linux).
# Run from repo root:  ./scripts/install_dependencies.sh
set -e
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

echo "=== Lumari Watch dependency installer ==="
echo ""

# --- 1. System prerequisites ---
if [[ "$(uname)" == "Darwin" ]]; then
  echo "Detected macOS. Checking Homebrew..."
  if ! command -v brew &>/dev/null; then
    echo "Homebrew is required. Install from https://brew.sh"
    exit 1
  fi
  echo "Installing system packages (git, cmake, ninja, pkg-config, python3, QEMU libs, ...)..."
  brew install git cmake ninja pkg-config dfu-util python3 libgcrypt glib pixman sdl2 libslirp ccache 2>/dev/null || true
  echo "System packages OK."
elif [[ "$(uname)" == "Linux" ]]; then
  echo "Detected Linux. Install system packages manually if needed, e.g.:"
  echo "  Ubuntu/Debian: sudo apt-get install -y git cmake ninja-build python3 python3-pip python3-venv dfu-util libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0"
  echo ""
  read -p "Continue with ESP-IDF install? [y/N] " -n 1 -r
  [[ "$REPLY" =~ ^[yY] ]] || exit 0
fi

# --- 2. ESP-IDF ---
IDF_DIR=""
if [[ -n "$IDF_PATH" && -f "$IDF_PATH/export.sh" ]]; then
  IDF_DIR="$IDF_PATH"
  echo "Using existing IDF_PATH: $IDF_DIR"
elif command -v eim &>/dev/null; then
  echo "EIM (Espressif Installation Manager) found. Installing ESP-IDF..."
  eim install -i v5.3.2 2>/dev/null || eim install
  # EIM install path is often ~/esp/esp-idf or similar; try to detect
  for d in "$HOME/esp/esp-idf" "$HOME/.espressif/esp-idf"; do
    if [[ -f "$d/export.sh" ]]; then
      IDF_DIR="$d"
      break
    fi
  done
  if [[ -z "$IDF_DIR" ]]; then
    echo "EIM install finished. Find your IDF path in EIM GUI or run: eim list"
    echo "Then create .idf_path in the project root with that path."
    exit 0
  fi
else
  # Legacy: clone + install
  IDF_DIR="$HOME/esp/esp-idf"
  if [[ ! -f "$IDF_DIR/export.sh" ]]; then
    echo "Cloning ESP-IDF v5.3.2 to $IDF_DIR (required by waveshare/qmi8658)..."
    mkdir -p "$(dirname "$IDF_DIR")"
    if [[ ! -d "$IDF_DIR/.git" ]]; then
      git clone -b v5.3.2 --recursive --depth 1 https://github.com/espressif/esp-idf.git "$IDF_DIR"
    fi
    echo "Installing ESP-IDF tools for esp32s3..."
    (cd "$IDF_DIR" && ./install.sh esp32s3)
  else
    echo "ESP-IDF already present at $IDF_DIR"
  fi
fi

# --- 3. QEMU (optional) ---
echo ""
echo "Installing QEMU for emulator (qemu-xtensa, qemu-riscv32)..."
export IDF_PATH="$IDF_DIR"
. "$IDF_DIR/export.sh" 2>/dev/null || true
if command -v idf.py &>/dev/null; then
  python "$IDF_PATH/tools/idf_tools.py" install qemu-xtensa qemu-riscv32
  echo "QEMU install OK."
else
  echo "Could not run idf_tools (export.sh failed?). Install QEMU later with:"
  echo "  . \$IDF_PATH/export.sh"
  echo "  python \$IDF_PATH/tools/idf_tools.py install qemu-xtensa qemu-riscv32"
fi

# --- 4. Project .idf_path ---
if [[ -n "$IDF_DIR" ]]; then
  echo "$IDF_DIR" > "$REPO_ROOT/.idf_path"
  echo ""
  echo "Wrote $REPO_ROOT/.idf_path with: $IDF_DIR"
fi

echo ""
echo "=== Done ==="
echo ""
echo "Next steps:"
echo "  1. In every new terminal where you build Lumari, load ESP-IDF:"
echo "     . \$IDF_PATH/export.sh"
echo "     (Or use the path in .idf_path:  . \$(cat .idf_path)/export.sh )"
echo ""
echo "  2. Build the project:"
echo "     idf.py set-target esp32s3"
echo "     idf.py build"
echo ""
echo "  3. Run in QEMU (after building with QEMU config):"
echo "     ./run_qemu.sh"
echo ""
echo "Full dependency list and options: docs/DEPENDENCIES.md"
