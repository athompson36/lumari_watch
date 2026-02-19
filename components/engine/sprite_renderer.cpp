#include "sprite_renderer.h"
#include "lumari_config.h"

void draw_sprite(
    uint16_t* framebuffer,
    const uint16_t* sprite,
    int sprite_width,
    int sprite_height,
    int pos_x,
    int pos_y
)
{
    for (int y = 0; y < sprite_height; y++)
    {
        for (int x = 0; x < sprite_width; x++)
        {
            int fb_x = pos_x + x;
            int fb_y = pos_y + y;

            if (fb_x < 0 || fb_x >= SCREEN_WIDTH ||
                fb_y < 0 || fb_y >= SCREEN_HEIGHT)
                continue;

            framebuffer[fb_y * SCREEN_WIDTH + fb_x] =
                sprite[y * sprite_width + x];
        }
    }
}

void draw_fill_circle(
    uint16_t* framebuffer,
    int cx, int cy, int radius,
    uint16_t color
)
{
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy > radius * radius) continue;
            int x = cx + dx;
            int y = cy + dy;
            if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) continue;
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

/* 5x7 digit font (0–9), one column per row, 5 LSBs. */
static const uint8_t s_digit5x7[10][7] = {
    { 0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F },
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E },
    { 0x1F, 0x01, 0x01, 0x1F, 0x10, 0x10, 0x1F },
    { 0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F },
    { 0x11, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x01 },
    { 0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F },
    { 0x1F, 0x10, 0x10, 0x1F, 0x11, 0x11, 0x1F },
    { 0x1F, 0x01, 0x01, 0x02, 0x04, 0x04, 0x04 },
    { 0x1F, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x1F },
    { 0x1F, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x1F },
};

#define DIGIT_W  5
#define DIGIT_H  7

void draw_number(
    uint16_t* framebuffer,
    int x, int y, unsigned value,
    uint16_t color
)
{
    if (value > 9999) value = 9999;
    unsigned v = value;
    int num_digits = (v == 0) ? 1 : 0;
    for (unsigned t = v; t; t /= 10) num_digits++;
    int cx = x + (num_digits - 1) * (DIGIT_W + 1);
    if (v == 0) {
        int d = 0;
        for (int row = 0; row < DIGIT_H; row++) {
            uint8_t bits = s_digit5x7[d][row];
            for (int col = 0; col < DIGIT_W; col++) {
                if (bits & (1 << (DIGIT_W - 1 - col))) {
                    int px = cx - col;
                    int py = y + row;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                        framebuffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
        return;
    }
    while (v) {
        int d = (int)(v % 10);
        v /= 10;
        for (int row = 0; row < DIGIT_H; row++) {
            uint8_t bits = s_digit5x7[d][row];
            for (int col = 0; col < DIGIT_W; col++) {
                if (bits & (1 << (DIGIT_W - 1 - col))) {
                    int px = cx - col;
                    int py = y + row;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                        framebuffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
        cx -= (DIGIT_W + 1);
    }
}
