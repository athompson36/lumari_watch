#pragma once
#include <stdint.h>

void ui_system_render(uint16_t* framebuffer, uint32_t time_ms);

/* Touch handling: returns action id. 0 = none. App must call ui_system_apply_action when needed. */
int ui_system_handle_touch(int x, int y);

/* Apply a settings action (e.g. TIME_HOUR_UP, DISPLAY_SAVE). Called from app after handle_touch. */
void ui_system_apply_action(int action);

/* Action ids returned by ui_system_handle_touch / used by ui_system_apply_action */
#define SYS_ACT_NONE           0
#define SYS_ACT_OPEN_TIME      1
#define SYS_ACT_OPEN_DISPLAY   2
#define SYS_ACT_OPEN_WIFI      3
#define SYS_ACT_OPEN_BT        4
#define SYS_ACT_BACK           5
#define SYS_ACT_TIME_HOUR_UP   10
#define SYS_ACT_TIME_HOUR_DOWN 11
#define SYS_ACT_TIME_MIN_UP    12
#define SYS_ACT_TIME_MIN_DOWN  13
#define SYS_ACT_TIME_DAY_UP    14
#define SYS_ACT_TIME_DAY_DOWN  15
#define SYS_ACT_TIME_MONTH_UP  16
#define SYS_ACT_TIME_MONTH_DOWN 17
#define SYS_ACT_TIME_YEAR_UP   18
#define SYS_ACT_TIME_YEAR_DOWN 19
#define SYS_ACT_TIME_SAVE      20
#define SYS_ACT_DISPLAY_BRIGHT_25  30
#define SYS_ACT_DISPLAY_BRIGHT_50  31
#define SYS_ACT_DISPLAY_BRIGHT_75  32
#define SYS_ACT_DISPLAY_BRIGHT_100 33
#define SYS_ACT_DISPLAY_24H_TOGGLE 34
#define SYS_ACT_DISPLAY_SAVE       35
#define SYS_ACT_WIFI_TOGGLE   40
#define SYS_ACT_BT_TOGGLE     41
