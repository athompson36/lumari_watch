#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CUTSCENE_ID_EVOLUTION  0
#define CUTSCENE_COUNT         1

/* Unlock flags: bit 0 = evolution cutscene. Main loads/saves via storage. */
void cutscene_lore_set_bitfield(uint32_t bitfield);
uint32_t cutscene_lore_get_bitfield(void);
bool cutscene_lore_unlocked(unsigned cutscene_id);
void cutscene_lore_set_unlocked(unsigned cutscene_id);

void cutscene_init(void);
bool cutscene_is_active(void);
unsigned cutscene_get_id(void);
void cutscene_start(unsigned cutscene_id);
/* Advance to next slide. Returns false when cutscene ended. */
bool cutscene_advance(void);
/* Current slide text for display (valid while cutscene is active). */
const char *cutscene_get_current_line(void);
