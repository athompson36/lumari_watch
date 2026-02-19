#include "layer_manager.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

#define MENU_OVERLAY_COLOR  0x3186
#define MENU_TEXT_COLOR     0xFFFF
#define QUEST_COLOR        0x07E0

void layer_manager_render(uint16_t* framebuffer, bool menu_open)
{
    creature_engine_render(framebuffer);

    /* Quest progress HUD (top-left): "Q progress/goal" */
    {
        uint32_t prog = quest_engine_get_progress();
        uint32_t goal = quest_engine_get_goal();
        if (goal > 0) {
            draw_string(framebuffer, 8, 10, "Q", QUEST_COLOR);
            draw_number(framebuffer, 22, 10, (unsigned)prog, QUEST_COLOR);
            draw_string(framebuffer, 52, 10, " ", QUEST_COLOR);
            draw_number(framebuffer, 58, 10, (unsigned)goal, QUEST_COLOR);
        }
    }

    if (menu_open) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MENU_OVERLAY_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 12, 80, "MENU", MENU_TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 33, 120, "TAP TO CLOSE", MENU_TEXT_COLOR);
    }
}
