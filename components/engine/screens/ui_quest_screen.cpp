#include "screens/ui_quest_screen.h"
#include "sprite_renderer.h"
#include "quest_engine.h"
#include "lumari_config.h"

#define TITLE_COLOR 0x07E0
#define TEXT_COLOR  0xFFFF

void ui_quest_render(uint16_t* framebuffer)
{
    int title_y = STATUS_BAR_H + 4 * UI_SCALE;
    int row_y = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (6 * FONT_STRIDE) / 2, title_y, "QUESTS", TITLE_COLOR);
    uint32_t prog = quest_engine_get_progress();
    uint32_t goal = quest_engine_get_goal();
    draw_string(framebuffer, MENU_MARGIN, row_y, "PROGRESS", TEXT_COLOR);
    draw_number(framebuffer, MENU_MARGIN + 9 * FONT_STRIDE, row_y, (unsigned)prog, TEXT_COLOR);
    draw_string(framebuffer, MENU_MARGIN + 12 * FONT_STRIDE, row_y, "/", TEXT_COLOR);
    draw_number(framebuffer, MENU_MARGIN + 13 * FONT_STRIDE, row_y, (unsigned)goal, TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (19 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "BOOT NEXT  PWR HOME", 0x528A);
}
