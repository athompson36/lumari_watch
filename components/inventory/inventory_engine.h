#pragma once

#include <stdint.h>
#include <stdbool.h>

#define INVENTORY_MAX_ACCESSORIES 8

typedef enum {
    ACCESSORY_TYPE_HAT = 0,
    ACCESSORY_TYPE_BODY,
    ACCESSORY_TYPE_COUNT
} accessory_type_t;

typedef struct {
    uint8_t id;
    const char *name;
    uint32_t unlock_xp;
    accessory_type_t type;
} accessory_def_t;

void inventory_init(void);

/* Call each frame or when XP changes; unlocks any accessories whose unlock_xp <= xp. */
void inventory_check_unlocks(uint32_t xp);

void inventory_equip(uint8_t id);
void inventory_unequip(void);
uint8_t inventory_get_equipped(void);

bool inventory_get_unlocked(uint8_t id);
unsigned inventory_get_count(void);
const accessory_def_t *inventory_get_def(uint8_t id);

/* Cycle through unlocked accessories (including 0 = none). Returns next/prev equipped id. */
uint8_t inventory_next_unlocked(uint8_t current);
uint8_t inventory_prev_unlocked(uint8_t current);

/* Set state from storage (bitfield: bit 0 = id 1, etc.). */
void inventory_set_unlocked_bitfield(uint32_t bitfield);
void inventory_set_equipped(uint8_t id);
uint32_t inventory_get_unlocked_bitfield(void);

/* Draw the equipped accessory. (cx, head_cy) = head center, body_cy = body center y, body_r/head_r = radii. */
void inventory_draw_accessory(
    uint16_t *framebuffer,
    int cx, int head_cy, int body_cy, int body_r, int head_r
);
