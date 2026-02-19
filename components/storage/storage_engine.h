#pragma once

#include <stdint.h>
#include <stdbool.h>

#define STORAGE_NAMESPACE "lumari"

/* Call once at startup before loading any state. */
void storage_init(void);

/* Creature state */
void storage_save_creature(uint32_t xp, uint32_t momentum);
bool storage_load_creature(uint32_t *xp, uint32_t *momentum);

/* Quest state (current index 0-based, progress toward current goal) */
void storage_save_quest(unsigned quest_index, uint32_t progress);
bool storage_load_quest(unsigned *quest_index, uint32_t *progress);

/* Inventory: equipped_id (0 = none), unlocked as bitfield (bit 0 = accessory 1, etc.) */
void storage_save_inventory(uint8_t equipped_id, uint32_t unlocked_bitfield);
bool storage_load_inventory(uint8_t *equipped_id, uint32_t *unlocked_bitfield);

/* Aura: crafted (0 = not crafted, 1 = crafted) */
void storage_save_aura(bool crafted);
bool storage_load_aura(bool *crafted);

/* Lore: cutscene unlock bitfield (bit 0 = evolution, etc.) */
void storage_save_lore(uint32_t lore_bitfield);
bool storage_load_lore(uint32_t *lore_bitfield);
