#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Quest types for progress tracking */
typedef enum {
    QUEST_TYPE_STEPS = 0,
    QUEST_TYPE_COUNT
} quest_type_t;

/* Call once at startup */
void quest_engine_init(void);

/* Add progress for the current quest. When goal is reached, rewards XP and advances. */
void quest_engine_add_progress(quest_type_t type, uint32_t amount);

/* 0 = none, 1.. = current quest index (for UI). */
uint32_t quest_engine_get_current_quest_id(void);

/* Progress toward current goal (0..goal). */
uint32_t quest_engine_get_progress(void);

/* Current goal value (e.g. steps needed). */
uint32_t quest_engine_get_goal(void);

/* True if the last add_progress caused a completion (for one-shot feedback if needed). */
bool quest_engine_just_completed(void);
