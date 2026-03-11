#include "creature_engine.h"
#include "sprite_renderer.h"
#include "inventory_engine.h"
#include "lumari_config.h"

/* Resolution-scaled sizes: creature fills ~40% of width on 410×502. */
#define BODY_RADIUS   (SCREEN_WIDTH / 5)
#define HEAD_RADIUS   (SCREEN_WIDTH / 8)
#define HEAD_OFFSET_Y (SCREEN_HEIGHT / 16)

/* Phase 1: Lumari Seedling – pastel blob + XP stub. Phase 2: momentum meter + mood + animation */
static unsigned s_xp = 0;
static unsigned s_momentum = 0; /* 0–100 */
static creature_mood_t s_mood = CREATURE_MOOD_IDLE;
static uint32_t s_happy_until_ms = 0;

#define SEEDLING_COLOR_BODY  0x7FE0
#define SEEDLING_COLOR_HEAD  0xB7E0
#define EVOLVED_COLOR_BODY   0x07FF
#define EVOLVED_COLOR_HEAD   0x5DDF
#define XP_COLOR             0xFFFF
#define MOMENTUM_BAR_COLOR   0x07E0
#define MOMENTUM_BG_COLOR    0x3186
#define HAPPY_GLOW_COLOR     0xFFE0
#define EVOLVED_XP_THRESHOLD 100

void creature_engine_init(void)
{
    s_xp = 0;
    s_momentum = 0;
    s_mood = CREATURE_MOOD_IDLE;
    s_happy_until_ms = 0;
}

void creature_engine_add_steps(unsigned n)
{
    s_momentum += 2 * n;
    if (s_momentum > 100)
        s_momentum = 100;
}

void creature_engine_update(void)
{
    if (s_momentum > 0)
        s_momentum--;
}

void creature_engine_set_mood(creature_mood_t mood)
{
    s_mood = mood;
}

void creature_engine_set_happy_until_ms(uint32_t end_time_ms)
{
    s_mood = CREATURE_MOOD_HAPPY;
    s_happy_until_ms = end_time_ms;
}

void creature_engine_tick(uint32_t time_ms)
{
    if (s_mood == CREATURE_MOOD_HAPPY && time_ms >= s_happy_until_ms)
        s_mood = CREATURE_MOOD_IDLE;
}

creature_mood_t creature_engine_get_mood(void)
{
    return s_mood;
}

/* 0 = seedling, 1 = evolved (100+ XP). */
int creature_engine_evolved_stage(void)
{
    return (s_xp >= EVOLVED_XP_THRESHOLD) ? 1 : 0;
}

void creature_engine_add_xp(unsigned n)
{
    const unsigned cap = 9999;
    if (s_xp + n > cap)
        s_xp = cap;
    else
        s_xp += n;
}

bool creature_engine_spend_xp(unsigned amount)
{
    if (s_xp < amount)
        return false;
    s_xp -= amount;
    return true;
}

unsigned creature_engine_get_xp(void)
{
    return s_xp;
}

unsigned creature_engine_get_momentum(void)
{
    return s_momentum;
}

void creature_engine_set_state(unsigned xp, unsigned momentum)
{
    s_xp = (xp > 9999u) ? 9999u : xp;
    s_momentum = (momentum > 100u) ? 100u : momentum;
}

/* Simple integer sine approximation: returns -128..127 for phase 0..255. */
static int isin(int phase256)
{
    int p = phase256 & 0xFF;
    if (p < 64)  return p * 2;
    if (p < 128) return 127 - (p - 64) * 2;
    if (p < 192) return -(p - 128) * 2;
    return -127 + (p - 192) * 2;
}

void creature_engine_render(uint16_t *framebuffer, uint32_t time_ms)
{
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    int body_r = BODY_RADIUS;
    int head_r = HEAD_RADIUS;
    int head_offset_y = HEAD_OFFSET_Y;

    /* Idle bob: gentle Y offset (period ~2.5s), scaled amplitude */
    int bob_amp = 4 * UI_SCALE;
    int bob_phase = (int)((time_ms / 10) % 256);
    int bob_y = (s_mood == CREATURE_MOOD_SLEEP) ? 0 : (isin(bob_phase) * bob_amp / 128);
    int draw_cy = cy + bob_y;
    int head_cy = draw_cy - head_offset_y;

    uint16_t body_color = SEEDLING_COLOR_BODY;
    uint16_t head_color = SEEDLING_COLOR_HEAD;

    int stage = creature_engine_evolved_stage();
    if (stage >= 1) {
        body_color = EVOLVED_COLOR_BODY;
        head_color = EVOLVED_COLOR_HEAD;
        body_r = BODY_RADIUS + SCREEN_WIDTH / 41;
        head_r = HEAD_RADIUS + SCREEN_WIDTH / 41;
        head_offset_y = HEAD_OFFSET_Y + SCREEN_HEIGHT / 84;
        head_cy = draw_cy - head_offset_y;
    }

    if (s_mood == CREATURE_MOOD_SLEEP) {
        body_r = body_r * 9 / 10;
        head_r = head_r * 9 / 10;
    }

    /* Happy glow: subtle ring around creature, scaled */
    if (s_mood == CREATURE_MOOD_HAPPY) {
        int glow_phase = (int)((time_ms / 50) % 64);
        int g = (glow_phase < 32) ? glow_phase : (64 - glow_phase);
        int r_outer = body_r + 12 * UI_SCALE + g / 4;
        int r_inner = body_r + 4 * UI_SCALE - g / 8;
        if (r_inner < body_r) r_inner = body_r;
        draw_ring(framebuffer, cx, draw_cy, r_inner, r_outer, HAPPY_GLOW_COLOR);
    }

    draw_fill_circle(framebuffer, cx, draw_cy, body_r, body_color);
    draw_fill_circle(framebuffer, cx, head_cy, head_r, head_color);

    /* Simple eyes (two small circles on head; skip when sleeping) */
    if (s_mood != CREATURE_MOOD_SLEEP) {
        int eye_r = head_r / 4;
        if (eye_r < 2) eye_r = 2;
        int eye_off_x = head_r / 2;
        int eye_y = head_cy - head_r / 4;
        uint16_t eye_color = (stage >= 1) ? 0x001F : 0x3186;
        draw_fill_circle(framebuffer, cx - eye_off_x, eye_y, eye_r, eye_color);
        draw_fill_circle(framebuffer, cx + eye_off_x, eye_y, eye_r, eye_color);
    }

    inventory_draw_accessory(framebuffer, cx, head_cy, draw_cy, body_r, head_r);

    /* XP top-right: below rounded top edge */
    draw_number(framebuffer, SCREEN_WIDTH - MENU_MARGIN, STATUS_BAR_TOP + STATUS_BAR_H / 2 - FONT_CHAR_H / 2, s_xp, XP_COLOR);

    {
        int bar_w = SCREEN_WIDTH * 6 / 10;
        int bar_h = 8 * UI_SCALE;
        int bx = (SCREEN_WIDTH - bar_w) / 2;
        int by = SCREEN_HEIGHT - BOTTOM_BAR_H - bar_h - 6 * UI_SCALE;
        draw_rect(framebuffer, bx, by, bar_w, bar_h, MOMENTUM_BG_COLOR);
        int fill_w = (bar_w * s_momentum) / 100;
        if (fill_w > 0)
            draw_rect(framebuffer, bx, by, fill_w, bar_h, MOMENTUM_BAR_COLOR);
    }
}
