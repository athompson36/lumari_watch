#pragma once
#include <stdint.h>

void display_hal_init(void);
void display_hal_flush(uint16_t *framebuffer);
void display_hal_set_brightness(uint8_t percent);
void display_hal_sleep(void);
void display_hal_wake(void);
