#include "screens/ui_lore_screen.h"
#include "sprite_renderer.h"
#include "cutscene_engine.h"
#include "lumari_config.h"

#define TITLE_COLOR 0x5DDF
#define TEXT_COLOR  0xFFFF

void ui_lore_render(uint16_t* framebuffer)
{
    int title_y = STATUS_BAR_H + 4 * UI_SCALE;
    int y = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    int line_h = FONT_CHAR_H + 4 * UI_SCALE;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (4 * FONT_STRIDE) / 2, title_y, "LORE", TITLE_COLOR);
    if (cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION))
        draw_string(framebuffer, MENU_MARGIN, y, "EVOLUTION", TEXT_COLOR), y += line_h;
    if (cutscene_lore_unlocked(CUTSCENE_ID_AETHERON_INTRO))
        draw_string(framebuffer, MENU_MARGIN, y, "AETHERON", TEXT_COLOR), y += line_h;
    if (cutscene_lore_unlocked(CUTSCENE_ID_PIXEL_MODE))
        draw_string(framebuffer, MENU_MARGIN, y, "PIXEL MODE", TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (19 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "BOOT NEXT  PWR HOME", 0x528A);
}
