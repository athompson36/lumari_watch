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

typedef enum
{
    SETTINGS_PAGE_MAIN = 0,
    SETTINGS_PAGE_TIME_DATE,
    SETTINGS_PAGE_DISPLAY,
    SETTINGS_PAGE_WIFI,
    SETTINGS_PAGE_BLUETOOTH,
    SETTINGS_PAGE_COUNT
} SettingsPage;

typedef struct
{
    UIScreen current_screen;
    SettingsPage settings_page;
} UIState;
