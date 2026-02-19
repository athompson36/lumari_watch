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

void draw_rect(
    uint16_t* framebuffer,
    int x, int y, int w, int h,
    uint16_t color
)
{
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                framebuffer[py * SCREEN_WIDTH + px] = color;
        }
    }
}

/* 5x7 font: index 0 = space, 1–26 = A–Z. */
static const uint8_t s_alpha5x7[27][7] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11 },
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E },
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E },
    { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E },
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F },
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 },
    { 0x0E, 0x11, 0x10, 0x13, 0x11, 0x11, 0x0F },
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 },
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E },
    { 0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E },
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 },
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F },
    { 0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11 },
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 },
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 },
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D },
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 },
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E },
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 },
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11 },
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 },
    { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 },
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F },
};

#define CHAR_W  (DIGIT_W)
#define CHAR_H  (DIGIT_H)

static void draw_char5x7(uint16_t* fb, int x, int y, uint8_t c, uint16_t color)
{
    if (c > 26) c = 0;
    const uint8_t* rows = s_alpha5x7[c];
    for (int row = 0; row < CHAR_H; row++) {
        uint8_t bits = rows[row];
        for (int col = 0; col < CHAR_W; col++) {
            if (bits & (1 << (CHAR_W - 1 - col))) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                    fb[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

void draw_fill_triangle(
    uint16_t* framebuffer,
    int cx, int cy, int half_width, int height,
    uint16_t color
)
{
    int y_top = cy - height / 2;
    int y_bot = cy + height / 2;
    for (int y = y_top; y <= y_bot; y++) {
        /* Linear interpolation: at y_top width=0, at y_bot width=2*half_width */
        int dy = y - y_top;
        int denom = height;
        int half_w = (denom <= 0) ? 0 : (half_width * dy) / denom;
        if (half_w > half_width) half_w = half_width;
        int x_left = cx - half_w;
        int x_right = cx + half_w;
        for (int x = x_left; x <= x_right; x++) {
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
                framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

void draw_string(
    uint16_t* framebuffer,
    int x, int y, const char* str,
    uint16_t color
)
{
    if (!str) return;
    int cx = x;
    for (; *str; str++) {
        char c = *str;
        if (c == ' ') {
            draw_char5x7(framebuffer, cx, y, 0, color);
        } else if (c >= 'A' && c <= 'Z') {
            draw_char5x7(framebuffer, cx, y, (uint8_t)(c - 'A' + 1), color);
        } else if (c >= 'a' && c <= 'z') {
            draw_char5x7(framebuffer, cx, y, (uint8_t)(c - 'a' + 1), color);
        }
        cx += CHAR_W + 1;
    }
}

void draw_ring(
    uint16_t* framebuffer,
    int cx, int cy, int r_inner, int r_outer,
    uint16_t color
)
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
                framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}
