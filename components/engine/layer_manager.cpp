#include "layer_manager.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "inventory_engine.h"
#include "aura_engine.h"
#include "cutscene_engine.h"
#include "sprite_renderer.h"
#include "time_service.h"
#include "power_service.h"
#include "ui_manager.h"
#include "lumari_config.h"

#define MENU_OVERLAY_COLOR  0x3186
#define MENU_ROW_BG_COLOR   0x2104
#define MENU_TEXT_COLOR     0xFFFF
#define QUEST_COLOR         0x07E0
#define TIME_COLOR          0xAD55
#define DATE_COLOR          0x8C71
#define BATTERY_COLOR       0xAD55
/* Layout: below rounded top edge (STATUS_BAR_TOP). */
#define MENU_TITLE_Y        (STATUS_BAR_TOP + 12 * UI_SCALE)
#define MENU_CRAFT_TOP_Y    (STATUS_BAR_TOP + 40 * UI_SCALE)
#define MENU_CRAFT_BOT_Y    (MENU_CRAFT_TOP_Y + MENU_ROW_H)
#define MENU_EQUIP_TOP_Y    (MENU_CRAFT_BOT_Y + MENU_ROW_PAD)
#define MENU_EQUIP_BOT_Y    (MENU_EQUIP_TOP_Y + MENU_ROW_H)
#define MENU_NAME_TOP_Y     (MENU_EQUIP_BOT_Y + MENU_ROW_PAD)
#define MENU_NAME_BOT_Y     (MENU_NAME_TOP_Y + MENU_ROW_H)
#define MENU_LORE_TOP_Y     (MENU_NAME_BOT_Y + MENU_ROW_PAD)
#define MENU_LORE_BOT_Y     (MENU_LORE_TOP_Y + MENU_ROW_H)
#define LORE_BACK_TOP_Y     (STATUS_BAR_TOP + 40 * UI_SCALE)
#define LORE_BACK_BOT_Y     (LORE_BACK_TOP_Y + MENU_ROW_H)
#define LORE_EVOLUTION_TOP_Y  (LORE_BACK_BOT_Y + MENU_ROW_PAD)
#define LORE_EVOLUTION_BOT_Y  (LORE_EVOLUTION_TOP_Y + MENU_ROW_H)
#define LORE_AETHERON_TOP_Y   (LORE_EVOLUTION_BOT_Y + MENU_ROW_PAD)
#define LORE_AETHERON_BOT_Y   (LORE_AETHERON_TOP_Y + MENU_ROW_H)
#define LORE_PIXEL_TOP_Y      (LORE_AETHERON_BOT_Y + MENU_ROW_PAD)
#define LORE_PIXEL_BOT_Y      (LORE_PIXEL_TOP_Y + MENU_ROW_H)
#define MENU_EQUIP_Y         (MENU_EQUIP_TOP_Y + MENU_ROW_H / 2 - FONT_CHAR_H / 2)
#define MENU_NAME_Y          (MENU_NAME_TOP_Y + MENU_ROW_H / 2 - FONT_CHAR_H / 2)
#define MENU_LORE_ROW_Y      (MENU_LORE_TOP_Y + MENU_ROW_H / 2 - FONT_CHAR_H / 2)
#define MENU_FOOTER_Y        (SCREEN_HEIGHT - BOTTOM_BAR_H - 8 * UI_SCALE - FONT_CHAR_H)
#define CUTSCENE_TAP_Y       (SCREEN_HEIGHT - BOTTOM_BAR_H - 6 * UI_SCALE - FONT_CHAR_H)

static int str_width(const char *s)
{
    int n = 0;
    while (s && s[n]) n++;
    return n * FONT_STRIDE;
}

void layer_manager_render(uint16_t* framebuffer, bool menu_open, uint32_t time_ms, bool lore_menu_open)
{
    if (cutscene_is_active()) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0000);
        const char *line = cutscene_get_current_line();
        int line_w = str_width(line);
        int box_pad = 16 * UI_SCALE;
        int box_w = line_w + box_pad * 2;
        if (box_w > SCREEN_WIDTH - box_pad) box_w = SCREEN_WIDTH - box_pad;
        int box_h = FONT_CHAR_H + 4 * UI_SCALE + box_pad;
        int box_x = (SCREEN_WIDTH - box_w) / 2;
        int box_y = SCREEN_HEIGHT / 2 - box_h / 2 - 4 * UI_SCALE;
        if (box_x < MENU_MARGIN) box_x = MENU_MARGIN;
        draw_rect(framebuffer, box_x, box_y, box_w, box_h, 0x2104);
        draw_rect(framebuffer, box_x, box_y, box_w, UI_SCALE, MENU_TEXT_COLOR);
        draw_rect(framebuffer, box_x, box_y + box_h - UI_SCALE, box_w, UI_SCALE, MENU_TEXT_COLOR);
        int tx = (SCREEN_WIDTH - line_w) / 2;
        if (tx < MENU_MARGIN) tx = MENU_MARGIN;
        draw_string(framebuffer, tx, box_y + box_pad / 2, line, MENU_TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (3 * FONT_STRIDE) / 2, CUTSCENE_TAP_Y, "TAP", MENU_TEXT_COLOR);
        return;
    }

    /* UI shell: Home / Quests / Inventory / Crafting / Lore / System */
    ui_manager_render(framebuffer, time_ms);

    if (menu_open) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MENU_OVERLAY_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - (4 * FONT_STRIDE) / 2, MENU_TITLE_Y, "MENU", MENU_TEXT_COLOR);
        if (lore_menu_open) {
            draw_rect(framebuffer, MENU_MARGIN, LORE_BACK_TOP_Y - MENU_ROW_PAD,
                     SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (4 * FONT_STRIDE) / 2, LORE_BACK_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "BACK", MENU_TEXT_COLOR);
            if (cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION)) {
                draw_rect(framebuffer, MENU_MARGIN, LORE_EVOLUTION_TOP_Y - MENU_ROW_PAD,
                          SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
                draw_string(framebuffer, SCREEN_WIDTH / 2 - (9 * FONT_STRIDE) / 2, LORE_EVOLUTION_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "EVOLUTION", MENU_TEXT_COLOR);
            }
            if (cutscene_lore_unlocked(CUTSCENE_ID_AETHERON_INTRO)) {
                draw_rect(framebuffer, MENU_MARGIN, LORE_AETHERON_TOP_Y - MENU_ROW_PAD,
                          SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
                draw_string(framebuffer, SCREEN_WIDTH / 2 - (8 * FONT_STRIDE) / 2, LORE_AETHERON_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "AETHERON", MENU_TEXT_COLOR);
            }
            if (cutscene_lore_unlocked(CUTSCENE_ID_PIXEL_MODE)) {
                draw_rect(framebuffer, MENU_MARGIN, LORE_PIXEL_TOP_Y - MENU_ROW_PAD,
                          SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
                draw_string(framebuffer, SCREEN_WIDTH / 2 - (10 * FONT_STRIDE) / 2, LORE_PIXEL_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "PIXEL MODE", MENU_TEXT_COLOR);
            }
        } else {
            draw_rect(framebuffer, MENU_MARGIN, MENU_CRAFT_TOP_Y - MENU_ROW_PAD,
                     SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
            if (aura_crafted())
                draw_string(framebuffer, SCREEN_WIDTH / 2 - (9 * FONT_STRIDE) / 2, MENU_CRAFT_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "AURA CALM", MENU_TEXT_COLOR);
            else if (aura_can_craft((unsigned)creature_engine_get_xp()))
                draw_string(framebuffer, SCREEN_WIDTH / 2 - (14 * FONT_STRIDE) / 2, MENU_CRAFT_TOP_Y + (MENU_ROW_H - FONT_CHAR_H) / 2, "CRAFT AURA 200", MENU_TEXT_COLOR);
            draw_rect(framebuffer, MENU_MARGIN, MENU_EQUIP_TOP_Y - MENU_ROW_PAD,
                     SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (5 * FONT_STRIDE) / 2, MENU_EQUIP_Y, "EQUIP", MENU_TEXT_COLOR);
            draw_rect(framebuffer, MENU_MARGIN, MENU_NAME_TOP_Y - MENU_ROW_PAD,
                     SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
            uint8_t eq = inventory_get_equipped();
            const accessory_def_t *def = inventory_get_def(eq);
            const char *name = (eq == 0) ? "NONE" : (def ? def->name : "???");
            int name_len = 0;
            while (name && name_len < 16 && name[name_len]) name_len++;
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (name_len * FONT_STRIDE) / 2, MENU_NAME_Y, name, MENU_TEXT_COLOR);
            draw_rect(framebuffer, MENU_MARGIN, MENU_LORE_TOP_Y - MENU_ROW_PAD,
                     SCREEN_WIDTH - 2 * MENU_MARGIN, MENU_ROW_H + 2 * MENU_ROW_PAD, MENU_ROW_BG_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (4 * FONT_STRIDE) / 2, MENU_LORE_ROW_Y, "LORE", MENU_TEXT_COLOR);
            draw_string(framebuffer, MENU_MARGIN, MENU_FOOTER_Y, "L R", MENU_TEXT_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (15 * FONT_STRIDE) / 2, MENU_FOOTER_Y, "LONG PRESS CLOSE", MENU_TEXT_COLOR);
        }
    }
}

/* Returns: 0=cycled equip, 1=craft, 2=play evolution, 3=open lore, 4=close lore, 5=play Aetheron, 6=play Pixel mode. */
int layer_manager_menu_handle_touch(int touch_x, int touch_y, int screen_width, int screen_height, bool lore_menu_open)
{
    (void)touch_x;
    (void)screen_width;
    if (lore_menu_open) {
        if (touch_y >= LORE_BACK_TOP_Y && touch_y <= LORE_BACK_BOT_Y)
            return 4;
        if (touch_y >= LORE_EVOLUTION_TOP_Y && touch_y <= LORE_EVOLUTION_BOT_Y && cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION))
            return 2;
        if (touch_y >= LORE_AETHERON_TOP_Y && touch_y <= LORE_AETHERON_BOT_Y && cutscene_lore_unlocked(CUTSCENE_ID_AETHERON_INTRO))
            return 5;
        if (touch_y >= LORE_PIXEL_TOP_Y && touch_y <= LORE_PIXEL_BOT_Y && cutscene_lore_unlocked(CUTSCENE_ID_PIXEL_MODE))
            return 6;
        return 0;
    }
    if (touch_y >= MENU_LORE_TOP_Y - MENU_ROW_PAD && touch_y <= MENU_LORE_BOT_Y + MENU_ROW_PAD)
        return 3;
    if (touch_y >= MENU_CRAFT_TOP_Y - MENU_ROW_PAD && touch_y <= MENU_CRAFT_BOT_Y + MENU_ROW_PAD &&
        aura_can_craft((unsigned)creature_engine_get_xp()))
        return 1;
    uint8_t cur = inventory_get_equipped();
    uint8_t next_id;
    if (touch_x < screen_width / 2)
        next_id = inventory_prev_unlocked(cur);
    else
        next_id = inventory_next_unlocked(cur);
    inventory_equip(next_id);
    return 0;
}
