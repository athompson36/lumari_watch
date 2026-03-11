#include "screens/ui_inventory_screen.h"
#include "sprite_renderer.h"
#include "inventory_engine.h"
#include "lumari_config.h"

#define TITLE_COLOR 0xFEC0
#define TEXT_COLOR  0xFFFF

void ui_inventory_render(uint16_t* framebuffer)
{
    int title_y = STATUS_BAR_H + 4 * UI_SCALE;
    int row1 = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    int row2 = row1 + FONT_CHAR_H + 4 * UI_SCALE;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (9 * FONT_STRIDE) / 2, title_y, "INVENTORY", TITLE_COLOR);
    uint8_t eq = inventory_get_equipped();
    const accessory_def_t* def = inventory_get_def(eq);
    const char* name = (eq == 0) ? "NONE" : (def ? def->name : "???");
    int name_len = 0;
    while (name && name_len < 16 && name[name_len]) name_len++;
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (8 * FONT_STRIDE) / 2, row1, "EQUIPPED", TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (name_len * FONT_STRIDE) / 2, row2, name, TEXT_COLOR);
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (19 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "BOOT NEXT  PWR HOME", 0x528A);
}
