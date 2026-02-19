#!/usr/bin/env bash
# Build QEMU from Espressif's fork (esp-develop) with esp32s3 and the virtual
# RGB framebuffer device, so the SDL display window works. The prebuilt
# qemu-xtensa from idf_tools does not include the framebuffer at 0x21000000.
#
# Prerequisites: libgcrypt, glib, pixman, sdl2, libslirp, pkg-config, ninja.
#   macOS: brew install libgcrypt glib pixman sdl2 libslirp pkg-config ninja
#   Ubuntu: sudo apt-get install libgcrypt20-dev libglib2.0-dev libpixman-1-dev libsdl2-dev libslirp-dev pkg-config ninja-build
# (gnutls is disabled in configure to avoid extra dependency.)
#
# Usage:
#   ./scripts/build_qemu_from_fork.sh [DIR]
# If DIR is omitted, uses $LUMARI_QEMU_BUILD_DIR or $HOME/esp/qemu-espressif.
#
# After building, set LUMARI_QEMU_BUILD_DIR to the clone directory (the one
# that contains 'build/qemu-system-xtensa'). run_qemu.sh will then use it
# and the display window will show the framebuffer.
set -e
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

# Where to clone and build
DEST="${1:-${LUMARI_QEMU_BUILD_DIR:-$HOME/esp/qemu-espressif}}"
DEST="${DEST/#\~/$HOME}"

echo "=== Building QEMU from Espressif fork (esp-develop) ==="
echo "  Destination: $DEST"
echo ""

# Clone or update
if [[ ! -d "$DEST/.git" ]]; then
  echo "Cloning https://github.com/espressif/qemu (branch esp-develop)..."
  mkdir -p "$(dirname "$DEST")"
  git clone -b esp-develop --depth 1 https://github.com/espressif/qemu.git "$DEST"
else
  echo "Updating existing clone..."
  (cd "$DEST" && git fetch origin esp-develop && git checkout esp-develop && git pull --ff-only origin esp-develop || true)
fi

cd "$DEST"

# Apply Lumari patch for esp_rgb address translation (fixes black SDL window with framebuffer at 0x20000000)
PATCH="$REPO_ROOT/scripts/patches/qemu-esp_rgb-address-translation.patch"
if [[ -f "$PATCH" ]] && ! grep -q 'ESP_RGB_VRAM_BASE' hw/display/esp_rgb.c 2>/dev/null; then
  echo "Applying patch: qemu-esp_rgb-address-translation.patch"
  git apply -p1 < "$PATCH"
fi

# Configure (from esp-toolchain-docs qemu/esp32s3/README.md; omit --enable-debug for faster build)
if [[ ! -f build/config-host.mak && ! -f build/build.ninja ]]; then
  echo "Configuring (xtensa-softmmu, SDL, slirp, gcrypt)..."
  ./configure --target-list=xtensa-softmmu \
    --enable-gcrypt \
    --enable-slirp \
    --enable-sdl \
    --disable-gnutls \
    --disable-strip \
    --disable-user \
    --disable-capstone \
    --disable-vnc \
    --disable-gtk
else
  echo "Build dir already configured; skipping configure."
fi

echo "Building (this may take several minutes)..."
ninja -C build

if [[ -x build/qemu-system-xtensa ]]; then
  echo ""
  echo "=== Build complete ==="
  echo "  Binary: $DEST/build/qemu-system-xtensa"
  echo ""
  echo "To use this QEMU with Lumari (and get the display window), set:"
  echo "  export LUMARI_QEMU_BUILD_DIR=\"$DEST\""
  echo "Then run from the project root:"
  echo "  ./run_qemu.sh"
  echo ""
  echo "Or persist it (e.g. in .env or your shell profile):"
  echo "  echo 'export LUMARI_QEMU_BUILD_DIR=\"$DEST\"' >> ~/.zshrc"
else
  echo "Build failed: build/qemu-system-xtensa not found." >&2
  exit 1
fi
