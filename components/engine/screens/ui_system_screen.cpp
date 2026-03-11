#include "screens/ui_system_screen.h"
#include "sprite_renderer.h"
#include "lumari_config.h"
#include "ui_manager.h"
#include "ui_state.h"
#include "time_service.h"
#include "display_hal.h"
#include "storage_engine.h"

#define TITLE_COLOR 0xAD55
#define TEXT_COLOR  0xFFFF
#define DIM_COLOR   0x528A
#define ROW_BG     0x2104
#define BORDER_COLOR 0x528A

static time_service_datetime_t s_time_edit;
static int s_brightness_index = 2; /* 0=25%, 1=50%, 2=75%, 3=100% */
static bool s_24h = false;
static bool s_wifi_on = false;
static bool s_bt_on = false;

static const uint8_t BRIGHTNESS_PERCENT[] = { 30, 50, 75, 100 };

static int row_y(int row_index)
{
    int panel_top = STATUS_BAR_TOP + STATUS_BAR_H + 2 * UI_SCALE;
    int title_h = FONT_CHAR_H + 8 * UI_SCALE;
    return panel_top + title_h + 4 * UI_SCALE + row_index * (MENU_ROW_H + MENU_ROW_PAD);
}

static int row_top(int row_index) { return row_y(row_index) - MENU_ROW_PAD / 2; }
static int row_bottom(int row_index) { return row_y(row_index) + MENU_ROW_H + MENU_ROW_PAD / 2; }

static void ensure_time_loaded(void)
{
    time_service_get(&s_time_edit);
}

static void ensure_display_loaded(void)
{
    uint8_t b = 80, t = 0, w = 0, bt = 0;
    storage_load_settings(&b, &t, &w, &bt);
    s_24h = (t != 0);
    s_wifi_on = (w != 0);
    s_bt_on = (bt != 0);
    if (b <= 25) s_brightness_index = 0;
    else if (b <= 50) s_brightness_index = 1;
    else if (b <= 75) s_brightness_index = 2;
    else s_brightness_index = 3;
}

void ui_system_render(uint16_t* framebuffer, uint32_t time_ms)
{
    int panel_top = STATUS_BAR_TOP + STATUS_BAR_H + 2 * UI_SCALE;
    int panel_h = SCREEN_HEIGHT - BOTTOM_BAR_H - panel_top - 4 * UI_SCALE;
    int page = ui_manager_get_settings_page();

    draw_panel(framebuffer, MENU_MARGIN, panel_top, SCREEN_WIDTH - 2 * MENU_MARGIN, panel_h, ROW_BG, BORDER_COLOR);
    int title_y = panel_top + 4 * UI_SCALE;

    if (page == SETTINGS_PAGE_MAIN) {
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (8 * FONT_STRIDE) / 2, title_y, "SETTINGS", TITLE_COLOR);
        int r0 = row_y(0), r1 = row_y(1), r2 = row_y(2), r3 = row_y(3);
        draw_rect(framebuffer, MENU_MARGIN + 4, row_top(0), SCREEN_WIDTH - 2 * MENU_MARGIN - 8, MENU_ROW_H + MENU_ROW_PAD, ROW_BG);
        draw_string(framebuffer, MENU_MARGIN + 8, r0, "TIME AND DATE", TEXT_COLOR);
        draw_rect(framebuffer, MENU_MARGIN + 4, row_top(1), SCREEN_WIDTH - 2 * MENU_MARGIN - 8, MENU_ROW_H + MENU_ROW_PAD, ROW_BG);
        draw_string(framebuffer, MENU_MARGIN + 8, r1, "DISPLAY", TEXT_COLOR);
        draw_rect(framebuffer, MENU_MARGIN + 4, row_top(2), SCREEN_WIDTH - 2 * MENU_MARGIN - 8, MENU_ROW_H + MENU_ROW_PAD, ROW_BG);
        draw_string(framebuffer, MENU_MARGIN + 8, r2, "WIFI", TEXT_COLOR);
        draw_rect(framebuffer, MENU_MARGIN + 4, row_top(3), SCREEN_WIDTH - 2 * MENU_MARGIN - 8, MENU_ROW_H + MENU_ROW_PAD, ROW_BG);
        draw_string(framebuffer, MENU_MARGIN + 8, r3, "BLUETOOTH", TEXT_COLOR);
        draw_bottom_nav(framebuffer, "NEXT", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
        return;
    }

    if (page == SETTINGS_PAGE_TIME_DATE) {
        ensure_time_loaded();
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (11 * FONT_STRIDE) / 2, title_y, "TIME AND DATE", TITLE_COLOR);
        int y = title_y + FONT_CHAR_H + 6 * UI_SCALE;
        int line_h = FONT_CHAR_H + 6 * UI_SCALE;
        int btn_w = 12 * UI_SCALE;
        int val_x = SCREEN_WIDTH / 2 - 4 * FONT_STRIDE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "HOUR", TEXT_COLOR);
        draw_number(framebuffer, val_x, y, (unsigned)s_time_edit.hour, TEXT_COLOR);
        draw_button(framebuffer, MENU_MARGIN + 8, y - 2, btn_w, FONT_CHAR_H + 4, "-", 0x2945, TEXT_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w, y - 2, btn_w, FONT_CHAR_H + 4, "+", 0x2945, TEXT_COLOR);
        y += line_h;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "MIN", TEXT_COLOR);
        draw_number(framebuffer, val_x, y, (unsigned)s_time_edit.min, TEXT_COLOR);
        draw_button(framebuffer, MENU_MARGIN + 8, y - 2, btn_w, FONT_CHAR_H + 4, "-", 0x2945, TEXT_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w, y - 2, btn_w, FONT_CHAR_H + 4, "+", 0x2945, TEXT_COLOR);
        y += line_h;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "DAY", TEXT_COLOR);
        draw_number(framebuffer, val_x, y, (unsigned)s_time_edit.day, TEXT_COLOR);
        draw_button(framebuffer, MENU_MARGIN + 8, y - 2, btn_w, FONT_CHAR_H + 4, "-", 0x2945, TEXT_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w, y - 2, btn_w, FONT_CHAR_H + 4, "+", 0x2945, TEXT_COLOR);
        y += line_h;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "MONTH", TEXT_COLOR);
        draw_number(framebuffer, val_x, y, (unsigned)s_time_edit.month, TEXT_COLOR);
        draw_button(framebuffer, MENU_MARGIN + 8, y - 2, btn_w, FONT_CHAR_H + 4, "-", 0x2945, TEXT_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w, y - 2, btn_w, FONT_CHAR_H + 4, "+", 0x2945, TEXT_COLOR);
        y += line_h;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "YEAR", TEXT_COLOR);
        draw_number(framebuffer, val_x, y, (unsigned)s_time_edit.year, TEXT_COLOR);
        draw_button(framebuffer, MENU_MARGIN + 8, y - 2, btn_w, FONT_CHAR_H + 4, "-", 0x2945, TEXT_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w, y - 2, btn_w, FONT_CHAR_H + 4, "+", 0x2945, TEXT_COLOR);
        y += line_h + 4 * UI_SCALE;
        draw_button(framebuffer, SCREEN_WIDTH / 2 - 6 * FONT_STRIDE, y, 12 * FONT_STRIDE, BOTTOM_BAR_H - 4, "SAVE", 0x07E0, TEXT_COLOR);
        draw_bottom_nav(framebuffer, "BACK", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
        return;
    }

    if (page == SETTINGS_PAGE_DISPLAY) {
        ensure_display_loaded();
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (7 * FONT_STRIDE) / 2, title_y, "DISPLAY", TITLE_COLOR);
        int y = title_y + FONT_CHAR_H + 8 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "BRIGHTNESS", TEXT_COLOR);
        y += FONT_CHAR_H + 4 * UI_SCALE;
        int bw = (SCREEN_WIDTH - 2 * MENU_MARGIN - 24 - 3 * 8) / 4;
        if (bw > 14 * FONT_STRIDE) bw = 14 * FONT_STRIDE;
        for (int i = 0; i < 4; i++) {
            int bx = MENU_MARGIN + 8 + i * (bw + 8);
            uint16_t bg = (s_brightness_index == i) ? 0x07E0 : 0x2945;
            draw_button(framebuffer, bx, y, bw, FONT_CHAR_H + 6, (i == 0) ? "30%" : (i == 1) ? "50%" : (i == 2) ? "75%" : "100%", bg, TEXT_COLOR);
        }
        y += FONT_CHAR_H + 12 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "12H / 24H", TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 * FONT_STRIDE, y, s_24h ? "24H" : "12H", s_24h ? 0x07E0 : DIM_COLOR);
        draw_button(framebuffer, SCREEN_WIDTH - MENU_MARGIN - 8 * FONT_STRIDE - 10 * UI_SCALE, y - 2, 10 * UI_SCALE, FONT_CHAR_H + 4, "T", 0x2945, TEXT_COLOR);
        y += FONT_CHAR_H + 10 * UI_SCALE;
        draw_button(framebuffer, SCREEN_WIDTH / 2 - 6 * FONT_STRIDE, y, 12 * FONT_STRIDE, BOTTOM_BAR_H - 4, "SAVE", 0x07E0, TEXT_COLOR);
        draw_bottom_nav(framebuffer, "BACK", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
        return;
    }

    if (page == SETTINGS_PAGE_WIFI) {
        ensure_display_loaded();
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (4 * FONT_STRIDE) / 2, title_y, "WIFI", TITLE_COLOR);
        int y = title_y + FONT_CHAR_H + 12 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "STATUS", TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 4 * FONT_STRIDE, y, s_wifi_on ? "ON" : "OFF", s_wifi_on ? 0x07E0 : DIM_COLOR);
        y += FONT_CHAR_H + 8 * UI_SCALE;
        draw_button(framebuffer, SCREEN_WIDTH / 2 - 8 * FONT_STRIDE, y, 16 * FONT_STRIDE, BOTTOM_BAR_H - 4, s_wifi_on ? "TURN OFF" : "TURN ON", 0x2945, TEXT_COLOR);
        y += BOTTOM_BAR_H + 4 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "NOT CONNECTED", DIM_COLOR);
        draw_bottom_nav(framebuffer, "BACK", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
        return;
    }

    if (page == SETTINGS_PAGE_BLUETOOTH) {
        ensure_display_loaded();
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (9 * FONT_STRIDE) / 2, title_y, "BLUETOOTH", TITLE_COLOR);
        int y = title_y + FONT_CHAR_H + 12 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "STATUS", TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 4 * FONT_STRIDE, y, s_bt_on ? "ON" : "OFF", s_bt_on ? 0x07E0 : DIM_COLOR);
        y += FONT_CHAR_H + 8 * UI_SCALE;
        draw_button(framebuffer, SCREEN_WIDTH / 2 - 8 * FONT_STRIDE, y, 16 * FONT_STRIDE, BOTTOM_BAR_H - 4, s_bt_on ? "TURN OFF" : "TURN ON", 0x2945, TEXT_COLOR);
        y += BOTTOM_BAR_H + 4 * UI_SCALE;
        draw_string(framebuffer, MENU_MARGIN + 8, y, "NOT PAIRED", DIM_COLOR);
        draw_bottom_nav(framebuffer, "BACK", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
        return;
    }

    draw_bottom_nav(framebuffer, "NEXT", "HOME", 0x2945, 0xAD55, 0x528A, time_ms);
}

static int hit_row(int y, int panel_top, int content_bottom)
{
    if (y < panel_top || y > content_bottom) return -1;
    for (int i = 0; i < 4; i++) {
        if (y >= row_top(i) && y <= row_bottom(i)) return i;
    }
    return -1;
}

static int time_save_btn_top(void)
{
    int title_y = STATUS_BAR_TOP + STATUS_BAR_H + 2 * UI_SCALE + 4 * UI_SCALE;
    int y = title_y + FONT_CHAR_H + 6 * UI_SCALE;
    int line_h = FONT_CHAR_H + 6 * UI_SCALE;
    y += 5 * line_h + 4 * UI_SCALE;
    return y;
}

int ui_system_handle_touch(int x, int y)
{
    int page = ui_manager_get_settings_page();
    int panel_top = STATUS_BAR_TOP + STATUS_BAR_H + 2 * UI_SCALE;
    int bar_y = SCREEN_HEIGHT - BOTTOM_BAR_H;
    if (y >= bar_y) return SYS_ACT_NONE; /* bottom nav handled elsewhere */

    if (page == SETTINGS_PAGE_MAIN) {
        int content_bottom = panel_top + (4 * (MENU_ROW_H + MENU_ROW_PAD)) + MENU_ROW_H;
        int r = hit_row(y, panel_top + FONT_CHAR_H + 12 * UI_SCALE, content_bottom);
        if (r == 0) return SYS_ACT_OPEN_TIME;
        if (r == 1) return SYS_ACT_OPEN_DISPLAY;
        if (r == 2) return SYS_ACT_OPEN_WIFI;
        if (r == 3) return SYS_ACT_OPEN_BT;
        return SYS_ACT_NONE;
    }

    if (page == SETTINGS_PAGE_TIME_DATE) {
        ensure_time_loaded();
        int title_y = panel_top + 4 * UI_SCALE;
        int y0 = title_y + FONT_CHAR_H + 6 * UI_SCALE;
        int line_h = FONT_CHAR_H + 6 * UI_SCALE;
        int btn_w = 12 * UI_SCALE;
        int left_btn_x = MENU_MARGIN + 8;
        int right_btn_x = SCREEN_WIDTH - MENU_MARGIN - 8 - btn_w;
        for (int i = 0; i < 5; i++) {
            int row_y = y0 + i * line_h;
            if (y >= row_y - 2 && y <= row_y + FONT_CHAR_H + 4) {
                if (x >= left_btn_x && x < left_btn_x + btn_w) {
                    if (i == 0) return SYS_ACT_TIME_HOUR_DOWN;
                    if (i == 1) return SYS_ACT_TIME_MIN_DOWN;
                    if (i == 2) return SYS_ACT_TIME_DAY_DOWN;
                    if (i == 3) return SYS_ACT_TIME_MONTH_DOWN;
                    return SYS_ACT_TIME_YEAR_DOWN;
                }
                if (x >= right_btn_x && x < right_btn_x + btn_w) {
                    if (i == 0) return SYS_ACT_TIME_HOUR_UP;
                    if (i == 1) return SYS_ACT_TIME_MIN_UP;
                    if (i == 2) return SYS_ACT_TIME_DAY_UP;
                    if (i == 3) return SYS_ACT_TIME_MONTH_UP;
                    return SYS_ACT_TIME_YEAR_UP;
                }
            }
        }
        int save_top = time_save_btn_top();
        if (y >= save_top && y <= save_top + BOTTOM_BAR_H - 4)
            return SYS_ACT_TIME_SAVE;
        return SYS_ACT_NONE;
    }

    if (page == SETTINGS_PAGE_DISPLAY) {
        ensure_display_loaded();
        int title_y = panel_top + 4 * UI_SCALE;
        int y0 = title_y + FONT_CHAR_H + 8 * UI_SCALE + FONT_CHAR_H + 4 * UI_SCALE;
        int bw = (SCREEN_WIDTH - 2 * MENU_MARGIN - 24 - 3 * 8) / 4;
        if (bw > 14 * FONT_STRIDE) bw = 14 * FONT_STRIDE;
        for (int i = 0; i < 4; i++) {
            int bx = MENU_MARGIN + 8 + i * (bw + 8);
            if (x >= bx && x < bx + bw && y >= y0 - 2 && y <= y0 + FONT_CHAR_H + 6) {
                if (i == 0) return SYS_ACT_DISPLAY_BRIGHT_25;
                if (i == 1) return SYS_ACT_DISPLAY_BRIGHT_50;
                if (i == 2) return SYS_ACT_DISPLAY_BRIGHT_75;
                return SYS_ACT_DISPLAY_BRIGHT_100;
            }
        }
        int toggle_y = y0 + FONT_CHAR_H + 12 * UI_SCALE;
        if (y >= toggle_y - 2 && y <= toggle_y + FONT_CHAR_H + 4 && x >= SCREEN_WIDTH - MENU_MARGIN - 8 * FONT_STRIDE - 10 * UI_SCALE && x < SCREEN_WIDTH - MENU_MARGIN)
            return SYS_ACT_DISPLAY_24H_TOGGLE;
        int save_y = toggle_y + FONT_CHAR_H + 10 * UI_SCALE;
        if (y >= save_y && y <= save_y + BOTTOM_BAR_H - 4)
            return SYS_ACT_DISPLAY_SAVE;
        return SYS_ACT_NONE;
    }

    if (page == SETTINGS_PAGE_WIFI) {
        int title_y = panel_top + 4 * UI_SCALE;
        int y0 = title_y + FONT_CHAR_H + 12 * UI_SCALE + FONT_CHAR_H + 8 * UI_SCALE;
        if (y >= y0 - 2 && y <= y0 + BOTTOM_BAR_H - 4 && x >= SCREEN_WIDTH / 2 - 8 * FONT_STRIDE && x < SCREEN_WIDTH / 2 + 8 * FONT_STRIDE)
            return SYS_ACT_WIFI_TOGGLE;
        return SYS_ACT_NONE;
    }

    if (page == SETTINGS_PAGE_BLUETOOTH) {
        int title_y = panel_top + 4 * UI_SCALE;
        int y0 = title_y + FONT_CHAR_H + 12 * UI_SCALE + FONT_CHAR_H + 8 * UI_SCALE;
        if (y >= y0 - 2 && y <= y0 + BOTTOM_BAR_H - 4 && x >= SCREEN_WIDTH / 2 - 8 * FONT_STRIDE && x < SCREEN_WIDTH / 2 + 8 * FONT_STRIDE)
            return SYS_ACT_BT_TOGGLE;
        return SYS_ACT_NONE;
    }

    return SYS_ACT_NONE;
}

void ui_system_apply_action(int action)
{
    if (action == SYS_ACT_TIME_HOUR_UP)   { s_time_edit.hour = (s_time_edit.hour + 1) % 24; return; }
    if (action == SYS_ACT_TIME_HOUR_DOWN)  { s_time_edit.hour = (s_time_edit.hour + 23) % 24; return; }
    if (action == SYS_ACT_TIME_MIN_UP)    { s_time_edit.min = (s_time_edit.min + 1) % 60; return; }
    if (action == SYS_ACT_TIME_MIN_DOWN)   { s_time_edit.min = (s_time_edit.min + 59) % 60; return; }
    if (action == SYS_ACT_TIME_DAY_UP)    { s_time_edit.day = s_time_edit.day >= 31 ? 1 : s_time_edit.day + 1; return; }
    if (action == SYS_ACT_TIME_DAY_DOWN)   { s_time_edit.day = s_time_edit.day <= 1 ? 31 : s_time_edit.day - 1; return; }
    if (action == SYS_ACT_TIME_MONTH_UP)  { s_time_edit.month = s_time_edit.month >= 12 ? 1 : s_time_edit.month + 1; return; }
    if (action == SYS_ACT_TIME_MONTH_DOWN) { s_time_edit.month = s_time_edit.month <= 1 ? 12 : s_time_edit.month - 1; return; }
    if (action == SYS_ACT_TIME_YEAR_UP)    { s_time_edit.year = s_time_edit.year >= 2099 ? 2025 : s_time_edit.year + 1; return; }
    if (action == SYS_ACT_TIME_YEAR_DOWN)  { s_time_edit.year = s_time_edit.year <= 2025 ? 2099 : s_time_edit.year - 1; return; }
    if (action == SYS_ACT_TIME_SAVE) {
        time_service_set(&s_time_edit);
        ui_manager_settings_back();
        return;
    }
    if (action == SYS_ACT_DISPLAY_BRIGHT_25)  { s_brightness_index = 0; return; }
    if (action == SYS_ACT_DISPLAY_BRIGHT_50)  { s_brightness_index = 1; return; }
    if (action == SYS_ACT_DISPLAY_BRIGHT_75)  { s_brightness_index = 2; return; }
    if (action == SYS_ACT_DISPLAY_BRIGHT_100) { s_brightness_index = 3; return; }
    if (action == SYS_ACT_DISPLAY_24H_TOGGLE) { s_24h = !s_24h; return; }
    if (action == SYS_ACT_DISPLAY_SAVE) {
        display_hal_set_brightness(BRIGHTNESS_PERCENT[s_brightness_index]);
        storage_save_settings(BRIGHTNESS_PERCENT[s_brightness_index], s_24h ? 1 : 0, s_wifi_on ? 1 : 0, s_bt_on ? 1 : 0);
        ui_manager_settings_back();
        return;
    }
    if (action == SYS_ACT_WIFI_TOGGLE) {
        s_wifi_on = !s_wifi_on;
        uint8_t b = 80, t = 0, w = 0, bt = 0;
        storage_load_settings(&b, &t, &w, &bt);
        storage_save_settings(b, t, s_wifi_on ? 1 : 0, bt);
        return;
    }
    if (action == SYS_ACT_BT_TOGGLE) {
        s_bt_on = !s_bt_on;
        uint8_t b = 80, t = 0, w = 0, bt = 0;
        storage_load_settings(&b, &t, &w, &bt);
        storage_save_settings(b, t, w, s_bt_on ? 1 : 0);
        return;
    }
}
