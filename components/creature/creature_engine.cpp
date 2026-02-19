#include "creature_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

/* Phase 1: Lumari Seedling – pastel blob + XP stub */
static unsigned s_xp = 0;
#define SEEDLING_COLOR_BODY  0x7FE0
#define SEEDLING_COLOR_HEAD  0xB7E0
#define XP_COLOR             0xFFFF

void creature_engine_init(void)
{
    s_xp = 0;
}

void creature_engine_update(void)
{
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

    /* XP counter (top-right), stub value */
    draw_number(framebuffer, SCREEN_WIDTH - 36, 10, s_xp, XP_COLOR);
}
