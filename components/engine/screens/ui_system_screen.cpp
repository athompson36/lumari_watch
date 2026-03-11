#include "screens/ui_system_screen.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

#define TITLE_COLOR 0xAD55
#define TEXT_COLOR  0xFFFF

void ui_system_render(uint16_t* framebuffer)
{
    int title_y = STATUS_BAR_H + 4 * UI_SCALE;
    int row1 = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    int row2 = row1 + FONT_CHAR_H + 4 * UI_SCALE;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (6 * FONT_STRIDE) / 2, title_y, "SYSTEM", TITLE_COLOR);
    draw_string(framebuffer, MENU_MARGIN, row1, "LONG PRESS MENU", TEXT_COLOR);
    draw_string(framebuffer, MENU_MARGIN, row2, "CRAFT EQUIP LORE", TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (19 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "BOOT NEXT  PWR HOME", 0x528A);
}
