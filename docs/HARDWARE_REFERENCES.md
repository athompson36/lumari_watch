# Hardware & development references

References for the **Waveshare ESP32-S3-Touch-AMOLED-2.06** board and related ESP-IDF/BSP work.

**Full board & firmware context:** For a single consolidated reference (pinout, display/touch/IMU/RTC/audio/TF, BSP vs Lumari, init sequences, commands), see **[BOARD_FIRMWARE_CONTEXT.md](BOARD_FIRMWARE_CONTEXT.md)**. It was built from the wiki, Espressif BSP, OLEDS3Watch, and HA thread so we have every pertinent aspect in one place.

---

## Official hardware docs

| Resource | Link | Use |
|--------|------|-----|
| **Waveshare Wiki – ESP32-S3-Touch-AMOLED-2.06** | https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.06 | Schematic, pinout, dimensions, Arduino/ESP-IDF setup. **Schematic Diagram** is the source for ESP-IDF pin assignments and display bus. |

---

## ESP-IDF BSP

| Resource | Link | Use |
|--------|------|-----|
| **Espressif Component Registry – waveshare/esp32_s3_touch_amoled_2_06** | https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_2_06 | Official BSP: add as dependency for panel and touch drivers instead of hand-rolling. |

Example (if switching to BSP for display/touch):

```yaml
# idf_component.yml
dependencies:
  waveshare/esp32_s3_touch_amoled_2_06: "*"
```

---

## Community firmware & notes

| Resource | Link | Use |
|--------|------|-----|
| **OLEDS3Watch (GitHub)** | https://github.com/joaquimorg/OLEDS3Watch | Example firmware; adapt display/touch/IMU init and config for ESP-IDF. |
| **Home Assistant – Waveshare watch display init** | https://community.home-assistant.io/t/waveshare-esp32-s3-touch-amoled-2-06-watch/914798 | Community display init and config; cross-check with our CO5300/QSPI and touch setup. |

---

## Development workflow (from these docs)

1. **Pinout & schematic**  
   Use the Waveshare wiki **Schematic Diagram** for:
   - Display bus (QSPI: CO5300 – CS, PCLK, DATA0–3, RST).
   - Touch (FT3168 I2C – SDA, SCL, RST, INT).
   - IMU (QMI8658), RTC (PCF85063), buttons (BOOT, PWR).

2. **Use the official BSP when possible**  
   Add `waveshare/esp32_s3_touch_amoled_2_06` so panel and touch come from the BSP instead of custom HALs.

3. **Reuse community patterns**  
   Check OLEDS3Watch and Home Assistant threads for:
   - Display init sequence and QSPI/LCD config.
   - FT3168 I2C address and register usage.
   - QMI8658 I2C and step/accel handling.

4. **Touch & IMU (I2C)**  
   - **FT3168**: I2C (e.g. 0x38); datasheets and app notes are linked from the wiki.  
   - **QMI8658**: I2C (e.g. 0x6B); same I2C bus as touch/RTC on this board.

---

## Current Lumari pinout (from `lumari_config.h`)

For **LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06** we use:

- **Display (CO5300 QSPI):** CS=12, PCLK=11, DATA0–3=4,5,6,7, RST=8.  
- **Touch (FT3168 I2C):** 0x38, RST=9, INT=38; shared I2C SDA=15, SCL=14.  
- **IMU (QMI8658):** same I2C bus.  
- **RTC (PCF85063):** 0x51, same I2C bus.  
- **Buttons:** BOOT=0 (active low), PWR=10 (active high).

Cross-check these against the **official schematic** on the wiki when debugging hardware issues.
