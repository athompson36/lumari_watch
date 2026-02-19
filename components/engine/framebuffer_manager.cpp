#include "framebuffer_manager.h"
#include "lumari_config.h"
#include "esp_heap_caps.h"
#include <string.h>

#define FRAMEBUFFER_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define FRAMEBUFFER_SIZE   (FRAMEBUFFER_PIXELS * sizeof(uint16_t))

static uint16_t* frame_buffer = nullptr;

void framebuffer_manager_init(uint16_t** out_buffer)
{
    frame_buffer = (uint16_t*) heap_caps_malloc(
        FRAMEBUFFER_SIZE,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );

    if (!frame_buffer)
        abort();

    memset(frame_buffer, 0x00, FRAMEBUFFER_SIZE);

    if (out_buffer)
        *out_buffer = frame_buffer;
}

uint16_t* framebuffer_manager_get()
{
    return frame_buffer;
}

void framebuffer_clear(uint16_t color)
{
    for (int i = 0; i < FRAMEBUFFER_PIXELS; i++)
        frame_buffer[i] = color;
}
