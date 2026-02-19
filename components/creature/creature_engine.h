#pragma once
#include <stdint.h>

void creature_engine_init(void);
void creature_engine_update(void);
void creature_engine_render(uint16_t *framebuffer);
void creature_engine_add_xp(unsigned n);
/* Spend XP (e.g. for crafting). Returns true if current XP >= amount and amount was subtracted. */
bool creature_engine_spend_xp(unsigned amount);
/* Momentum: feed steps so the meter can animate (0–100). */
void creature_engine_add_steps(unsigned n);

unsigned creature_engine_get_xp(void);
unsigned creature_engine_get_momentum(void);
void creature_engine_set_state(unsigned xp, unsigned momentum);
