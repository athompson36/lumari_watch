#include "ui_manager.h"
#include "ui_state.h"
#include "screens/ui_home_screen.h"
#include "screens/ui_quest_screen.h"
#include "screens/ui_inventory_screen.h"
#include "screens/ui_crafting_screen.h"
#include "screens/ui_lore_screen.h"
#include "screens/ui_system_screen.h"

static UIState s_ui_state;

void ui_manager_init(void)
{
    s_ui_state.current_screen = UI_SCREEN_HOME;
}

void ui_manager_update(void)
{
    /* Placeholder for input routing (e.g. touch per screen). */
}

void ui_manager_render(uint16_t* framebuffer, uint32_t time_ms)
{
    switch (s_ui_state.current_screen)
    {
        case UI_SCREEN_HOME:
            ui_home_render(framebuffer, time_ms);
            break;
        case UI_SCREEN_QUESTS:
            ui_quest_render(framebuffer);
            break;
        case UI_SCREEN_INVENTORY:
            ui_inventory_render(framebuffer);
            break;
        case UI_SCREEN_CRAFTING:
            ui_crafting_render(framebuffer);
            break;
        case UI_SCREEN_LORE:
            ui_lore_render(framebuffer);
            break;
        case UI_SCREEN_SYSTEM:
            ui_system_render(framebuffer);
            break;
        default:
            ui_home_render(framebuffer, time_ms);
            break;
    }
}

void ui_manager_go_home(void)
{
    s_ui_state.current_screen = UI_SCREEN_HOME;
}

void ui_manager_next_screen(void)
{
    s_ui_state.current_screen = (UIScreen)((int)s_ui_state.current_screen + 1);
    if (s_ui_state.current_screen >= UI_SCREEN_COUNT)
        s_ui_state.current_screen = UI_SCREEN_HOME;
}

void ui_manager_previous_screen(void)
{
    if (s_ui_state.current_screen == UI_SCREEN_HOME)
        s_ui_state.current_screen = (UIScreen)(UI_SCREEN_COUNT - 1);
    else
        s_ui_state.current_screen = (UIScreen)((int)s_ui_state.current_screen - 1);
}

int ui_manager_current_screen(void)
{
    return (int)s_ui_state.current_screen;
}
