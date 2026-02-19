#pragma once
#include <stdbool.h>
#include <stdint.h>

void render_engine_init(void);
void render_engine_frame(bool menu_open, uint32_t time_ms, bool lore_menu_open);
