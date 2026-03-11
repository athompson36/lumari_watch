#include "screens/ui_crafting_screen.h"
#include "sprite_renderer.h"
#include "aura_engine.h"
#include "creature_engine.h"
#include "lumari_config.h"

#define TITLE_COLOR 0x4A49
#define TEXT_COLOR  0xFFFF

void ui_crafting_render(uint16_t* framebuffer)
{
    int title_y = STATUS_BAR_H + 4 * UI_SCALE;
    int row_y = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (8 * FONT_STRIDE) / 2, title_y, "CRAFTING", TITLE_COLOR);
    if (aura_crafted())
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (9 * FONT_STRIDE) / 2, row_y, "AURA CALM", TEXT_COLOR);
    else if (aura_can_craft((unsigned)creature_engine_get_xp()))
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (16 * FONT_STRIDE) / 2, row_y, "CRAFT AURA 200 XP", TEXT_COLOR);
    else
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (10 * FONT_STRIDE) / 2, row_y, "NEED 200 XP", TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (19 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "BOOT NEXT  PWR HOME", 0x528A);
}
