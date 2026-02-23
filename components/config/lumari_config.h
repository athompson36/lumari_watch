#pragma once

#include "sdkconfig.h"

/* Board selection: QEMU (from menuconfig) or Waveshare (default) */
#if defined(CONFIG_LUMARI_BOARD_QEMU) && CONFIG_LUMARI_BOARD_QEMU
#define LUMARI_BOARD_QEMU 1
#else
#define LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06 1
#endif

/* 1 = Phase 0 demo (circle, touch, button, shake). 0 = Lumari game loop. Use Kconfig if available. */
#if defined(CONFIG_LUMARI_RUN_PHASE0) && CONFIG_LUMARI_RUN_PHASE0
#define LUMARI_RUN_PHASE0  1
#else
#define LUMARI_RUN_PHASE0  0
#endif

#if LUMARI_BOARD_QEMU
/* QEMU: same aspect ratio as hardware (410:502 per Waveshare wiki / BOARD_FIRMWARE_CONTEXT.md).
 * Resolution reduced so framebuffer fits in internal RAM (no PSRAM in emulator).
 * 198×240 preserves 410/502 ≈ 0.817; RGB565 framebuffer = 95,040 bytes. */
#define SCREEN_WIDTH  198
#define SCREEN_HEIGHT 240
#else
/* Hardware: Waveshare ESP32-S3-Touch-AMOLED-2.06 — 2.06" AMOLED 410×502, 16.7M colors, CO5300 QSPI. */
#define SCREEN_WIDTH  410
#define SCREEN_HEIGHT 502
#endif

#define TARGET_FPS_IDLE   30
#define TARGET_FPS_ACTIVE 60

#if LUMARI_BOARD_QEMU

/* QEMU: virtual RGB panel only; touch/IMU/RTC stubbed */
#undef LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#define BUTTON_PIN       0
#define LCD_SPI_HOST     SPI2_HOST
#define TOUCH_PIN_CS     (-1)

#elif LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06

/* Display: CO5300 AMOLED over QSPI (ESP32-S3) */
#define LCD_QSPI_HOST          SPI2_HOST
#define LCD_PIN_CS             12
#define LCD_PIN_PCLK           11
#define LCD_PIN_DATA0          4
#define LCD_PIN_DATA1          5
#define LCD_PIN_DATA2          6
#define LCD_PIN_DATA3          7
#define LCD_PIN_RST            8
#define LCD_PIN_BACKLIGHT      (-1)

/* Touch: FT3168 capacitive over I2C (shared bus with IMU/RTC) */
#define TOUCH_I2C_ADDR          0x38
#define TOUCH_PIN_RST           9
#define TOUCH_PIN_INT           38

/* I2C bus: touch, QMI8658 IMU, PCF85063 RTC */
#define I2C_MASTER_SDA          15
#define I2C_MASTER_SCL          14
#define I2C_MASTER_FREQ_HZ      400000

/* Buttons: BOOT = GPIO0 (low when pressed); PWR = AXP2101 PMIC (poll power_service_poll_pwr_button_short) */
#define BUTTON_BOOT_PIN         0
#define BUTTON_PWR_PIN          10  /* not used: PWR is on PMIC */
#define BUTTON_PIN              0

/* IMU: QMI8658 on same I2C */
#define IMU_I2C_SDA             I2C_MASTER_SDA
#define IMU_I2C_SCL             I2C_MASTER_SCL
#define IMU_I2C_FREQ_HZ         I2C_MASTER_FREQ_HZ

#else

/* Generic / dev board (ST7789 SPI, XPT2046 touch) */
#define LCD_SPI_HOST            SPI2_HOST
#define LCD_PIXEL_CLOCK_HZ      (20 * 1000 * 1000)
#define LCD_PIN_SCLK            18
#define LCD_PIN_MOSI            19
#define LCD_PIN_MISO            21
#define LCD_PIN_DC              5
#define LCD_PIN_RST             3
#define LCD_PIN_CS              4
#define LCD_PIN_BACKLIGHT       2
#define TOUCH_PIN_CS             15
#define BUTTON_PIN              0
#define IMU_I2C_SDA             21
#define IMU_I2C_SCL             22
#define IMU_I2C_FREQ_HZ         100000

#endif
