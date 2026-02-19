#pragma once
#include <stdint.h>
#include <stdbool.h>

#define LAYER_ACTION_CYCLED    0
#define LAYER_ACTION_CRAFT     1
#define LAYER_ACTION_PLAY_CUTSCENE 2
#define LAYER_ACTION_OPEN_LORE 3
#define LAYER_ACTION_CLOSE_LORE 4

void layer_manager_render(uint16_t* framebuffer, bool menu_open, uint32_t time_ms, bool lore_menu_open);

/* When menu is open: touch handling. Returns LAYER_ACTION_* (craft 1, play cutscene 2, open lore 3, close lore 4, else 0). */
int layer_manager_menu_handle_touch(int touch_x, int touch_y, int screen_width, int screen_height, bool lore_menu_open);
