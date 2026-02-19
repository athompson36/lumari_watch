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
