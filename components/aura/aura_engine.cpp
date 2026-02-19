#include "aura_engine.h"
#include "lumari_config.h"

static bool s_crafted = false;

/* Soft pastel glow color (RGB565). */
#define AURA_COLOR  0x4A49

static void draw_ring_local(
    uint16_t *fb, int cx, int cy, int r_inner, int r_outer, uint16_t color)
{
    if (r_inner < 0) r_inner = 0;
    if (r_outer <= r_inner) return;
    int r_in_sq = r_inner * r_inner;
    int r_out_sq = r_outer * r_outer;
    for (int dy = -r_outer; dy <= r_outer; dy++) {
        for (int dx = -r_outer; dx <= r_outer; dx++) {
            int d_sq = dx * dx + dy * dy;
            if (d_sq > r_out_sq || d_sq < r_in_sq) continue;
            int x = cx + dx;
            int y = cy + dy;
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
                fb[y * SCREEN_WIDTH + x] = color;
        }
    }
}

void aura_engine_init(void)
{
    s_crafted = false;
}

bool aura_crafted(void)
{
    return s_crafted;
}

bool aura_can_craft(unsigned current_xp)
{
    return !s_crafted && current_xp >= AURA_CRAFT_COST_XP;
}

void aura_do_craft(void)
{
    s_crafted = true;
}

void aura_set_crafted(bool crafted)
{
    s_crafted = crafted;
}

void aura_draw(
    uint16_t *framebuffer,
    int cx, int cy,
    uint32_t time_ms
)
{
    if (!s_crafted || !framebuffer) return;

    /* Subtle pulse: radius varies slightly over time (period ~2s). */
    int phase = (int)(time_ms / 32) % 64;
    int pulse = (phase < 32) ? phase : (64 - phase);
    int r_outer = 58 + pulse / 4;
    int r_inner = 46 - pulse / 8;
    if (r_inner < 38) r_inner = 38;
    if (r_outer > 72) r_outer = 72;

    draw_ring_local(framebuffer, cx, cy, r_inner, r_outer, AURA_COLOR);
}
