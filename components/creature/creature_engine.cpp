#include "creature_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

static uint16_t dummy_sprite[32 * 32];

void creature_engine_init()
{
    for (int i = 0; i < 32 * 32; i++)
        dummy_sprite[i] = 0xFFFF;
}

void creature_engine_update()
{
}

void creature_engine_render(uint16_t* framebuffer)
{
    draw_sprite(
        framebuffer,
        dummy_sprite,
        32,
        32,
        (SCREEN_WIDTH / 2) - 16,
        (SCREEN_HEIGHT / 2) - 16
    );
}
