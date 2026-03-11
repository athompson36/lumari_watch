#pragma once
#include <stdint.h>

typedef enum {
    CREATURE_MOOD_IDLE,
    CREATURE_MOOD_HAPPY,
    CREATURE_MOOD_SLEEP,
} creature_mood_t;

void creature_engine_init(void);
void creature_engine_update(void);
void creature_engine_render(uint16_t *framebuffer, uint32_t time_ms);
void creature_engine_add_xp(unsigned n);
/* Spend XP (e.g. for crafting). Returns true if current XP >= amount and amount was subtracted. */
bool creature_engine_spend_xp(unsigned amount);
/* Momentum: feed steps so the meter can animate (0–100). */
void creature_engine_add_steps(unsigned n);

unsigned creature_engine_get_xp(void);
unsigned creature_engine_get_momentum(void);
void creature_engine_set_state(unsigned xp, unsigned momentum);

/* Digital pet mood: set happy until time_ms (e.g. now_ms + 2000); tick(time_ms) clears it. */
void creature_engine_set_mood(creature_mood_t mood);
void creature_engine_set_happy_until_ms(uint32_t end_time_ms);
void creature_engine_tick(uint32_t time_ms);
creature_mood_t creature_engine_get_mood(void);
/* True if creature has "evolved" (e.g. 100+ XP) for alternate visual. */
int creature_engine_evolved_stage(void);
