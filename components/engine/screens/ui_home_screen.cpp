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

    /* Status bar: below rounded top edge. Time (center), battery % (right), quest Q prog/goal (left), separator. */
    {
        int bar_y = STATUS_BAR_TOP;
        int bar_center_y = bar_y + STATUS_BAR_H / 2 - FONT_CHAR_H / 2;
        time_service_datetime_t dt;
        if (time_service_get(&dt)) {
            /* Time "H:MM" or "HH:MM" = 5 chars max; center in bar */
            int time_w = 5 * FONT_STRIDE;
            int time_x = (SCREEN_WIDTH - time_w) / 2;
            if (time_x < MENU_MARGIN) time_x = MENU_MARGIN;
            if (time_x + time_w > SCREEN_WIDTH - MENU_MARGIN) time_x = SCREEN_WIDTH - MENU_MARGIN - time_w;
            draw_time(framebuffer, time_x, bar_center_y, dt.hour, dt.min, TIME_COLOR);
        }
        uint8_t bat_pct = 0;
        if (power_service_get_battery_percent(&bat_pct)) {
            int bat_w = 2 * FONT_STRIDE; /* two digits */
            int bx = SCREEN_WIDTH - MENU_MARGIN - bat_w;
            if (bx < MENU_MARGIN) bx = MENU_MARGIN;
            draw_two_digits(framebuffer, bx, bar_center_y, (unsigned)bat_pct, BATTERY_COLOR);
        }
        uint32_t prog = quest_engine_get_progress();
        uint32_t goal = quest_engine_get_goal();
        if (goal > 0) {
            /* Quest: "Q prog/goal" with no overlap */
            draw_string(framebuffer, MENU_MARGIN, bar_center_y, "Q", QUEST_COLOR);
            draw_number(framebuffer, MENU_MARGIN + 4 * FONT_STRIDE, bar_center_y, (unsigned)(prog > 999 ? 999 : prog), QUEST_COLOR);
            draw_string(framebuffer, MENU_MARGIN + 5 * FONT_STRIDE, bar_center_y, " ", QUEST_COLOR);
            draw_number(framebuffer, MENU_MARGIN + 9 * FONT_STRIDE, bar_center_y, (unsigned)(goal > 999 ? 999 : goal), QUEST_COLOR);
        }
        draw_rect(framebuffer, 0, bar_y + STATUS_BAR_H - UI_SCALE, SCREEN_WIDTH, UI_SCALE, 0x3186);
    }

    /* Bottom nav: two buttons that fit on screen */
    draw_bottom_nav(framebuffer, "NEXT", "HOME", 0x2945, HINT_COLOR, 0x528A, time_ms);
}
