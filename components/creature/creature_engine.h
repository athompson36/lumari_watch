#pragma once
#include <stdint.h>

void creature_engine_init(void);
void creature_engine_update(void);
void creature_engine_render(uint16_t *framebuffer);
void creature_engine_add_xp(unsigned n);
/* Momentum: feed steps so the meter can animate (0–100). */
void creature_engine_add_steps(unsigned n);
