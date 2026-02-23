#!/usr/bin/env bash
# Run Lumari in QEMU with the display window.
# Emulator is configured to match Waveshare ESP32-S3-Touch-AMOLED-2.06 hardware where possible
# (aspect ratio 410:502, RGB565, no PSRAM). See docs/BOARD_FIRMWARE_CONTEXT.md §15.
set -e
cd "$(dirname "$0")"

# Always prefer IDF from .idf_path so we use the correct version (e.g. 5.3.2 for qmi8658)
_idf_sourced=
if [[ -f .idf_path ]]; then
  _p="$(sed -n '1s/[[:space:]]*#.*//; 1s/^[[:space:]]*//; 1s/[[:space:]]*$//; 1p' .idf_path)"
  _p="${_p/#\~/$HOME}"
  if [[ -n "$_p" && -f "$_p/export.sh" ]]; then
    . "$_p/export.sh" && _idf_sourced=1
  fi
fi
if [[ -z "$_idf_sourced" && -n "$IDF_PATH" && -f "$IDF_PATH/export.sh" ]]; then
  . "$IDF_PATH/export.sh" && _idf_sourced=1
fi
if [[ -z "$_idf_sourced" && ! -f .idf_path ]]; then
  for _d in "$HOME/esp/esp-idf" "/opt/esp-idf" "$HOME/.espressif/esp-idf" "$HOME/Documents/esp-idf"; do
    if [[ -f "$_d/export.sh" ]]; then
      . "$_d/export.sh" && _idf_sourced=1
      break
    fi
  done
fi
if [[ -z "$_idf_sourced" || ! -x "$(command -v idf.py 2>/dev/null)" ]]; then
  if ! command -v idf.py &>/dev/null; then
    echo "Error: idf.py not in PATH."
    echo ""
    if [[ -f .idf_path ]]; then
      _p="$(sed -n '1s/[[:space:]]*#.*//; 1s/^[[:space:]]*//; 1p' .idf_path)"
      _p="${_p/#\~/$HOME}"
      echo "The path in .idf_path doesn't exist or has no export.sh:"
      echo "  ${_p:-'(empty)'}"
      echo ""
      echo "Edit .idf_path and put the correct path to your esp-idf folder (the one that contains export.sh)."
    else
      echo "Create .idf_path with one line: the path to your esp-idf folder (the one that contains export.sh)."
      echo ""
      echo "  echo '/path/to/esp-idf' > .idf_path"
      echo ""
      echo "Common locations: ~/esp/esp-idf  or  the path shown in Cursor/VS Code ESP-IDF extension settings."
    fi
    echo ""
    echo "If you have a terminal where idf.py already works, get the path with:"
    echo "  dirname \$(dirname \$(which idf.py))"
    exit 1
  fi
fi
# Prefer custom-built QEMU from Espressif fork (has RGB framebuffer → SDL window).
# Otherwise use prebuilt (idf_tools); 9.0.0 has esp32s3; 8.1.3 does not.
_qemu_xtensa_bin=
if [[ -n "${LUMARI_QEMU_BUILD_DIR:-}" ]]; then
  _build="${LUMARI_QEMU_BUILD_DIR/#\~/$HOME}"
  if [[ -x "${_build}/build/qemu-system-xtensa" ]]; then
    _qemu_xtensa_bin="${_build}/build"
  fi
fi
if [[ -z "${_qemu_xtensa_bin:-}" ]]; then
  for _q in "$HOME/.espressif/tools/qemu-xtensa"/esp_develop_9.0.0*/qemu/bin \
            "$HOME/.espressif/tools/qemu-xtensa"/esp_develop_8.1*/qemu/bin \
            "$HOME/.espressif/tools/qemu-xtensa"/*/qemu/bin; do
    [[ -x "${_q}/qemu-system-xtensa" ]] && _qemu_xtensa_bin="${_q}" && break
  done
fi
[[ -n "${_qemu_xtensa_bin:-}" ]] && export PATH="${_qemu_xtensa_bin}:$PATH"
# Ensure we're on IDF 5.3.2+ (required by waveshare/qmi8658)
if [[ -n "$IDF_PATH" ]]; then
  _v=""
  [[ -f "$IDF_PATH/tools/cmake/version.cmake" ]] && _v="$(grep -E 'IDF_VERSION_(MAJOR|MINOR|PATCH)' "$IDF_PATH/tools/cmake/version.cmake" 2>/dev/null | head -3)"
  if [[ -n "$_v" ]]; then
    _major=$(echo "$_v" | grep MAJOR | sed 's/.*\([0-9]*\).*/\1/')
    _minor=$(echo "$_v" | grep MINOR | sed 's/.*\([0-9]*\).*/\1/')
    _patch=$(echo "$_v" | grep PATCH | sed 's/.*\([0-9]*\).*/\1/')
    if [[ -n "$_major" && -n "$_minor" ]]; then
      if [[ "$_major" -lt 5 ]] || { [[ "$_major" -eq 5 ]] && [[ "$_minor" -lt 3 ]]; }; then
        echo "Error: ESP-IDF $_major.$_minor.${_patch} is too old. waveshare/qmi8658 requires 5.3.2+."
        echo "Upgrade: cd $IDF_PATH && git fetch --tags && git checkout v5.3.2 && git submodule update --init --recursive && ./install.sh esp32s3"
        exit 1
      fi
      if [[ "$_major" -eq 5 ]] && [[ "$_minor" -eq 3 ]] && [[ -n "$_patch" ]] && [[ "$_patch" -lt 2 ]]; then
        echo "Error: ESP-IDF $_major.$_minor.$_patch is too old. waveshare/qmi8658 requires 5.3.2+."
        echo "Upgrade: cd $IDF_PATH && git fetch --tags && git checkout v5.3.2 && git submodule update --init --recursive && ./install.sh esp32s3"
        exit 1
      fi
    fi
  fi
fi

# Custom-built QEMU (LUMARI_QEMU_BUILD_DIR): real framebuffer (CONFIG_LUMARI_QEMU=n).
# Prebuilt QEMU: stub framebuffer (CONFIG_LUMARI_QEMU=y).
# Both set CONFIG_LUMARI_BOARD_QEMU=y for QEMU board profile (198x240, stubbed peripherals).
_use_custom_qemu=
if [[ -n "${LUMARI_QEMU_BUILD_DIR:-}" ]]; then
  _b="${LUMARI_QEMU_BUILD_DIR/#\~/$HOME}"
  [[ -x "${_b}/build/qemu-system-xtensa" ]] && _use_custom_qemu=1
fi

if [[ -n "$_use_custom_qemu" ]]; then
  _qemu_defaults="sdkconfig.defaults;sdkconfig.defaults.qemu_fb"
  _qemu_expected="CONFIG_LUMARI_QEMU=n"
  _qemu_label="custom framebuffer"
else
  _qemu_defaults="sdkconfig.defaults;sdkconfig.defaults.qemu"
  _qemu_expected="CONFIG_LUMARI_QEMU=y"
  _qemu_label="stub"
fi

NEED_REBUILD=
if [[ ! -f sdkconfig ]]; then
  NEED_REBUILD=1
elif ! grep -q "^CONFIG_LUMARI_BOARD_QEMU=y" sdkconfig 2>/dev/null; then
  NEED_REBUILD=1
elif ! grep -q "^${_qemu_expected}" sdkconfig 2>/dev/null; then
  NEED_REBUILD=1
# Force rebuild if Phase 0 is on (red circle) so QEMU runs full Lumari loop
elif grep -q "^CONFIG_LUMARI_RUN_PHASE0=y" sdkconfig 2>/dev/null; then
  NEED_REBUILD=1
fi
# Build dir incomplete (e.g. missing CMakeFiles/rules.ninja) → rebuild
if [[ -z "$NEED_REBUILD" && -f build/build.ninja && ! -f build/CMakeFiles/rules.ninja ]]; then
  NEED_REBUILD=1
fi
# On macOS we run QEMU ourselves and need flash_args to generate images; if missing, do a full QEMU build
if [[ -z "$NEED_REBUILD" && "$(uname -s)" = "Darwin" && ( ! -f build/flash_args ) ]]; then
  NEED_REBUILD=1
fi

if [[ -n "$NEED_REBUILD" ]]; then
  echo "Building for QEMU (${_qemu_label})..."
  rm -rf build
  rm -f sdkconfig
  [[ -x scripts/fix_co5300_headers.sh ]] && ./scripts/fix_co5300_headers.sh
  SDKCONFIG_DEFAULTS="$_qemu_defaults" idf.py build
fi

echo "Starting QEMU (serial in this terminal)..."
if [[ -z "$_use_custom_qemu" ]]; then
  echo ""
  echo "  Prebuilt QEMU: no display window (framebuffer not at 0x21000000)."
  echo "  For the SDL graphics window, build QEMU from Espressif fork and set LUMARI_QEMU_BUILD_DIR:"
  echo "    ./scripts/build_qemu_from_fork.sh"
  echo "    export LUMARI_QEMU_BUILD_DIR=\$HOME/esp/qemu-espressif"
  echo "    ./run_qemu.sh"
  echo ""
fi
echo "Exit: Ctrl-A then q"
# Use idf.py qemu --graphics so the virtual framebuffer device is enabled (SDL window) when using custom QEMU.
[[ -n "${_qemu_xtensa_bin:-}" ]] && export PATH="${_qemu_xtensa_bin}:$PATH"
idf.py qemu --graphics
