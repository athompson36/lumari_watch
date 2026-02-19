#include "creature_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

/* Phase 1: Lumari Seedling – pastel blob + XP stub. Phase 2: momentum meter */
static unsigned s_xp = 0;
static unsigned s_momentum = 0; /* 0–100 */
#define SEEDLING_COLOR_BODY  0x7FE0
#define SEEDLING_COLOR_HEAD  0xB7E0
#define XP_COLOR             0xFFFF
#define MOMENTUM_BAR_COLOR   0x07E0
#define MOMENTUM_BG_COLOR    0x3186

void creature_engine_init(void)
{
    s_xp = 0;
    s_momentum = 0;
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

void creature_engine_add_xp(unsigned n)
{
    const unsigned cap = 9999;
    if (s_xp + n > cap)
        s_xp = cap;
    else
        s_xp += n;
}

void creature_engine_render(uint16_t *framebuffer)
{
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    /* Seedling: two overlapping circles (body + head) */
    draw_fill_circle(framebuffer, cx, cy, 40, SEEDLING_COLOR_BODY);
    draw_fill_circle(framebuffer, cx, cy - 28, 24, SEEDLING_COLOR_HEAD);

    /* XP counter (top-right) */
    draw_number(framebuffer, SCREEN_WIDTH - 36, 10, s_xp, XP_COLOR);

    /* Momentum bar (bottom center): 120x8, fill by s_momentum % */
    {
        int bar_w = 120;
        int bar_h = 8;
        int bx = (SCREEN_WIDTH - bar_w) / 2;
        int by = SCREEN_HEIGHT - bar_h - 10;
        draw_rect(framebuffer, bx, by, bar_w, bar_h, MOMENTUM_BG_COLOR);
        int fill_w = (bar_w * s_momentum) / 100;
        if (fill_w > 0)
            draw_rect(framebuffer, bx, by, fill_w, bar_h, MOMENTUM_BAR_COLOR);
    }
}
