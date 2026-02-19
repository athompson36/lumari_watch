#pragma once
#include <stdint.h>

void framebuffer_manager_init(uint16_t** out_buffer);
uint16_t* framebuffer_manager_get();
void framebuffer_clear(uint16_t color);
