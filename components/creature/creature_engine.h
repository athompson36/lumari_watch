#pragma once
#include <stdint.h>

void creature_engine_init(void);
void creature_engine_update(void);
void creature_engine_render(uint16_t *framebuffer);
void creature_engine_add_xp(unsigned n);
