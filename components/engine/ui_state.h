#pragma once

typedef enum
{
    UI_SCREEN_HOME = 0,
    UI_SCREEN_QUESTS,
    UI_SCREEN_INVENTORY,
    UI_SCREEN_CRAFTING,
    UI_SCREEN_LORE,
    UI_SCREEN_SYSTEM,
    UI_SCREEN_COUNT
} UIScreen;

typedef struct
{
    UIScreen current_screen;
} UIState;
