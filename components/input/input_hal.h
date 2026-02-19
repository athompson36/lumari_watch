#pragma once
#include <stdint.h>
#include <stdbool.h>

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
#include "driver/i2c_master.h"
#endif

void input_hal_init(void);

/* Touch: returns true if touched; x/y in screen coords (0..width-1, 0..height-1) */
bool input_hal_touch_read(int *x, int *y);

/* Button: returns true if currently pressed */
bool input_hal_button_read(void);

#if LUMARI_BOARD_WAVESHARE_ESP32_S3_AMOLED_2_06
/* Shared I2C bus (touch + IMU + RTC). Call after input_hal_init(). */
i2c_master_bus_handle_t input_hal_get_i2c_bus(void);
#endif
