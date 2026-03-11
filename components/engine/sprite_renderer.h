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

/* Panel: filled rect with 1px border (for content areas). */
void draw_panel(
    uint16_t* framebuffer,
    int x, int y, int w, int h,
    uint16_t bg_color,
    uint16_t border_color
);

/* 1px outline of rectangle (border only). */
void draw_rect_outline(
    uint16_t* framebuffer,
    int x, int y, int w, int h,
    uint16_t color
);

/* Button: filled rect + outline + centered label. Label must fit in w. */
void draw_button(
    uint16_t* framebuffer,
    int x, int y, int w, int h,
    const char* label,
    uint16_t bg_color,
    uint16_t text_color
);

/* Bottom nav bar: two equal buttons that fit screen. Short labels (e.g. "NEXT", "HOME").
 * If time_ms is non-zero, outline pulses for subtle animation. */
void draw_bottom_nav(
    uint16_t* framebuffer,
    const char* left_label,
    const char* right_label,
    uint16_t bg_color,
    uint16_t text_color,
    uint16_t outline_color,
    uint32_t time_ms
);

/* Hit test: 0 = left button, 1 = right button, -1 = neither. */
int bottom_nav_hit_test(int x, int y);

/* Draw a string (5x7 uppercase + space). */
void draw_string(
    uint16_t* framebuffer,
    int x, int y, const char* str,
    uint16_t color
);

/* Isosceles triangle, apex at top: center (cx,cy), half_width and height. */
void draw_fill_triangle(
    uint16_t* framebuffer,
    int cx, int cy, int half_width, int height,
    uint16_t color
);

/* Ring (annulus): inner radius r_inner, outer radius r_outer. */
void draw_ring(
    uint16_t* framebuffer,
    int cx, int cy, int r_inner, int r_outer,
    uint16_t color
);

/* Draw time as "H:MM" or "HH:MM" (12-hour). */
void draw_time(
    uint16_t* framebuffer,
    int x, int y, uint8_t hour, uint8_t min,
    uint16_t color
);

/* Draw two digits 00–99 with leading zero. */
void draw_two_digits(
    uint16_t* framebuffer,
    int x, int y, unsigned value,
    uint16_t color
);

/* Draw short date "M/D" or "MM/D" (month, day 1–31). */
void draw_short_date(
    uint16_t* framebuffer,
    int x, int y, uint8_t month, uint8_t day,
    uint16_t color
);

/* Return pixel width of string (for centering). */
int draw_string_width(const char* str);
