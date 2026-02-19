#!/usr/bin/env bash
# Create missing include/ and priv_include/ for espressif__esp_lcd_co5300 (registry 1.0.2 omits them).
# Run from project root. Uses curl to fetch headers from esp-iot-solution at the 1.0.2 commit.
set -e
cd "$(dirname "$0")/.."
CO5300_REF="5f3e921d52c8b454449503bf7750df6fb6aa1764"
BASE="https://raw.githubusercontent.com/espressif/esp-iot-solution/${CO5300_REF}/components/display/lcd/esp_lcd_co5300"
DIR="managed_components/espressif__esp_lcd_co5300"
if [[ ! -d "$DIR" ]]; then
  exit 0
fi
if [[ -d "$DIR/include" && -d "$DIR/priv_include" ]]; then
  exit 0
fi
echo "Fixing CO5300 component: adding missing include/ and priv_include/..."
mkdir -p "$DIR/include" "$DIR/priv_include"
curl -sSfL -o "$DIR/include/esp_lcd_co5300.h" "${BASE}/include/esp_lcd_co5300.h"
curl -sSfL -o "$DIR/priv_include/esp_lcd_co5300_interface.h" "${BASE}/priv_include/esp_lcd_co5300_interface.h"
echo "Done."
