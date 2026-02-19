#pragma once

#include <stdint.h>
#include <stdbool.h>

#define AURA_CRAFT_COST_XP  200

void aura_engine_init(void);
bool aura_crafted(void);
/* Returns true if can craft (XP >= cost and not already crafted). */
bool aura_can_craft(unsigned current_xp);
/* Mark aura as crafted (call after spending XP). */
void aura_do_craft(void);
void aura_set_crafted(bool crafted);

/* Draw simple glow around (cx, cy). time_ms used for subtle pulse. */
void aura_draw(
    uint16_t *framebuffer,
    int cx, int cy,
    uint32_t time_ms
);
