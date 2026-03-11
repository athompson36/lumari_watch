#include "screens/ui_home_screen.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "inventory_engine.h"
#include "aura_engine.h"
#include "sprite_renderer.h"
#include "time_service.h"
#include "power_service.h"
#include "lumari_config.h"

#define TIME_COLOR    0xAD55
#define DATE_COLOR    0x8C71
#define BATTERY_COLOR 0xAD55
#define QUEST_COLOR   0x07E0
#define HINT_COLOR    0x528A

void ui_home_render(uint16_t* framebuffer, uint32_t time_ms)
{
    inventory_check_unlocks((uint32_t)creature_engine_get_xp());
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;
    if (aura_crafted())
        aura_draw(framebuffer, cx, cy, time_ms);
    creature_engine_render(framebuffer, time_ms);

    /* Status bar: time (center), date, battery % (right), separator */
    {
        time_service_datetime_t dt;
        if (time_service_get(&dt)) {
            int time_w = 5 * FONT_STRIDE * 5;
            int time_x = (SCREEN_WIDTH - time_w) / 2;
            if (time_x < MENU_MARGIN) time_x = MENU_MARGIN;
            draw_time(framebuffer, time_x, 2 * UI_SCALE, dt.hour, dt.min, TIME_COLOR);
            draw_short_date(framebuffer, time_x + 2 * UI_SCALE, 2 * UI_SCALE + FONT_CHAR_H + 2, dt.month, dt.day, DATE_COLOR);
        }
        uint8_t bat_pct = 0;
        if (power_service_get_battery_percent(&bat_pct)) {
            int bx = SCREEN_WIDTH - 2 * (6 * FONT_STRIDE) - MENU_MARGIN;
            if (bx < 0) bx = 0;
            draw_two_digits(framebuffer, bx, 2 * UI_SCALE, (unsigned)bat_pct, BATTERY_COLOR);
        }
        draw_rect(framebuffer, 0, STATUS_BAR_H - UI_SCALE, SCREEN_WIDTH, UI_SCALE, 0x3186);
    }

    /* Quest HUD (top-left) */
    {
        uint32_t prog = quest_engine_get_progress();
        uint32_t goal = quest_engine_get_goal();
        if (goal > 0) {
            draw_string(framebuffer, MENU_MARGIN, STATUS_BAR_H / 2 - FONT_CHAR_H / 2, "Q", QUEST_COLOR);
            draw_number(framebuffer, MENU_MARGIN + FONT_STRIDE + 5 * FONT_STRIDE, STATUS_BAR_H / 2 - FONT_CHAR_H / 2, (unsigned)prog, QUEST_COLOR);
            draw_string(framebuffer, MENU_MARGIN + FONT_STRIDE * 2 + 10 * FONT_STRIDE, STATUS_BAR_H / 2 - FONT_CHAR_H / 2, " ", QUEST_COLOR);
            draw_number(framebuffer, MENU_MARGIN + FONT_STRIDE * 3 + 15 * FONT_STRIDE, STATUS_BAR_H / 2 - FONT_CHAR_H / 2, (unsigned)goal, QUEST_COLOR);
        }
    }

    /* Bottom hint */
    draw_string(framebuffer, SCREEN_WIDTH / 2 - (15 * FONT_STRIDE) / 2, SCREEN_HEIGHT - FONT_CHAR_H - 4 * UI_SCALE, "LONG PRESS MENU", HINT_COLOR);
}
