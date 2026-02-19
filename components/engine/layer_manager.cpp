#include "layer_manager.h"
#include "creature_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

#define MENU_OVERLAY_COLOR  0x3186
#define MENU_TEXT_COLOR     0xFFFF

void layer_manager_render(uint16_t* framebuffer, bool menu_open)
{
    creature_engine_render(framebuffer);
    if (menu_open) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MENU_OVERLAY_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 12, 80, "MENU", MENU_TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 33, 120, "TAP TO CLOSE", MENU_TEXT_COLOR);
    }
}
