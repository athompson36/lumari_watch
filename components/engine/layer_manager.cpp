#include "layer_manager.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "inventory_engine.h"
#include "aura_engine.h"
#include "cutscene_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

#define MENU_OVERLAY_COLOR  0x3186
#define MENU_TEXT_COLOR     0xFFFF
#define QUEST_COLOR        0x07E0
#define MENU_CRAFT_TOP_Y   40
#define MENU_CRAFT_BOT_Y   78
#define MENU_LORE_TOP_Y    118
#define MENU_LORE_BOT_Y    148
#define LORE_BACK_TOP_Y    40
#define LORE_BACK_BOT_Y    70
#define LORE_EVOLUTION_TOP_Y  82
#define LORE_EVOLUTION_BOT_Y  108
#define CUTSCENE_TAP_Y     (SCREEN_HEIGHT - 24)
#define CHAR_W_PLUS_1     6

static int str_width(const char *s)
{
    int n = 0;
    while (s && s[n]) n++;
    return n * CHAR_W_PLUS_1;
}

void layer_manager_render(uint16_t* framebuffer, bool menu_open, uint32_t time_ms, bool lore_menu_open)
{
    if (cutscene_is_active()) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0000);
        const char *line = cutscene_get_current_line();
        int line_w = str_width(line);
        int x = (SCREEN_WIDTH - line_w) / 2;
        if (x < 8) x = 8;
        draw_string(framebuffer, x, SCREEN_HEIGHT / 2 - 8, line, MENU_TEXT_COLOR);
        draw_string(framebuffer, SCREEN_WIDTH / 2 - 9, CUTSCENE_TAP_Y, "TAP", MENU_TEXT_COLOR);
        return;
    }

    inventory_check_unlocks((uint32_t)creature_engine_get_xp());
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;
    if (aura_crafted())
        aura_draw(framebuffer, cx, cy, time_ms);
    creature_engine_render(framebuffer);

    /* Quest progress HUD (top-left): "Q progress/goal" */
    {
        uint32_t prog = quest_engine_get_progress();
        uint32_t goal = quest_engine_get_goal();
        if (goal > 0) {
            draw_string(framebuffer, 8, 10, "Q", QUEST_COLOR);
            draw_number(framebuffer, 22, 10, (unsigned)prog, QUEST_COLOR);
            draw_string(framebuffer, 52, 10, " ", QUEST_COLOR);
            draw_number(framebuffer, 58, 10, (unsigned)goal, QUEST_COLOR);
        }
    }

    if (menu_open) {
        draw_rect(framebuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MENU_OVERLAY_COLOR);
        if (lore_menu_open) {
            draw_string(framebuffer, SCREEN_WIDTH / 2 - 12, LORE_BACK_TOP_Y + 8, "BACK", MENU_TEXT_COLOR);
            if (cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION))
                draw_string(framebuffer, SCREEN_WIDTH / 2 - 27, LORE_EVOLUTION_TOP_Y + 4, "EVOLUTION", MENU_TEXT_COLOR);
        } else {
            if (aura_crafted())
                draw_string(framebuffer, SCREEN_WIDTH / 2 - 27, MENU_CRAFT_TOP_Y, "AURA CALM", MENU_TEXT_COLOR);
            else if (aura_can_craft((unsigned)creature_engine_get_xp()))
                draw_string(framebuffer, SCREEN_WIDTH / 2 - 42, MENU_CRAFT_TOP_Y, "CRAFT AURA 200", MENU_TEXT_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - 12, 60, "EQUIP", MENU_TEXT_COLOR);
            uint8_t eq = inventory_get_equipped();
            const accessory_def_t *def = inventory_get_def(eq);
            const char *name = (eq == 0) ? "NONE" : (def ? def->name : "???");
            draw_string(framebuffer, SCREEN_WIDTH / 2 - (int)(name[0] ? 12 : 0), 95, name, MENU_TEXT_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - 12, MENU_LORE_TOP_Y + 8, "LORE", MENU_TEXT_COLOR);
            draw_string(framebuffer, 20, SCREEN_HEIGHT - 30, "L R", MENU_TEXT_COLOR);
            draw_string(framebuffer, SCREEN_WIDTH / 2 - 45, SCREEN_HEIGHT - 30, "BTN CLOSE", MENU_TEXT_COLOR);
        }
    }
}

/* Returns: 0=cycled equip, 1=craft, 2=play evolution cutscene, 3=open lore menu, 4=close lore menu. */
int layer_manager_menu_handle_touch(int touch_x, int touch_y, int screen_width, int screen_height, bool lore_menu_open)
{
    (void)touch_x;
    (void)screen_width;
    if (lore_menu_open) {
        if (touch_y >= LORE_BACK_TOP_Y && touch_y <= LORE_BACK_BOT_Y)
            return 4;
        if (touch_y >= LORE_EVOLUTION_TOP_Y && touch_y <= LORE_EVOLUTION_BOT_Y && cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION))
            return 2;
        return 0;
    }
    if (touch_y >= MENU_LORE_TOP_Y && touch_y <= MENU_LORE_BOT_Y)
        return 3;
    if (touch_y >= MENU_CRAFT_TOP_Y && touch_y <= MENU_CRAFT_BOT_Y &&
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
