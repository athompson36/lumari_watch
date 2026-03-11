#pragma once
#include <stdint.h>

void ui_manager_init(void);
void ui_manager_update(void);
void ui_manager_render(uint16_t* framebuffer, uint32_t time_ms);

void ui_manager_go_home(void);
void ui_manager_next_screen(void);
void ui_manager_previous_screen(void);

/* Current screen (for input routing if needed). */
int ui_manager_current_screen(void);

/* Settings sub-navigation (only when current_screen == UI_SCREEN_SYSTEM). */
int ui_manager_get_settings_page(void);
void ui_manager_settings_enter(int page);
void ui_manager_settings_back(void);
int ui_manager_is_in_settings_subpage(void);
