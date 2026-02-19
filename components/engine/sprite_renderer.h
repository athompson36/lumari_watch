#pragma once
#include <stdint.h>

void draw_sprite(
    uint16_t* framebuffer,
    const uint16_t* sprite,
    int sprite_width,
    int sprite_height,
    int pos_x,
    int pos_y
);

void draw_fill_circle(
    uint16_t* framebuffer,
    int cx, int cy, int radius,
    uint16_t color
);

/* Draw a non-negative integer (0–9999) at (x,y), 5x7 digits, right-aligned. */
void draw_number(
    uint16_t* framebuffer,
    int x, int y, unsigned value,
    uint16_t color
);

void draw_rect(
    uint16_t* framebuffer,
    int x, int y, int w, int h,
    uint16_t color
);

/* Draw a string (5x7 uppercase + space). */
void draw_string(
    uint16_t* framebuffer,
    int x, int y, const char* str,
    uint16_t color
);
