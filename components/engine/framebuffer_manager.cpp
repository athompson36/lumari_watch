#include "framebuffer_manager.h"
#include "lumari_config.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "framebuffer_mgr";

#define FRAMEBUFFER_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define FRAMEBUFFER_SIZE   (FRAMEBUFFER_PIXELS * sizeof(uint16_t))

static uint16_t* frame_buffer = nullptr;

void framebuffer_manager_init(uint16_t** out_buffer)
{
#if LUMARI_BOARD_QEMU
    /* QEMU has no PSRAM; use internal RAM only */
    frame_buffer = (uint16_t*) heap_caps_malloc(
        FRAMEBUFFER_SIZE,
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    );
#else
    frame_buffer = (uint16_t*) heap_caps_malloc(
        FRAMEBUFFER_SIZE,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );
    if (!frame_buffer) {
        frame_buffer = (uint16_t*) heap_caps_malloc(
            FRAMEBUFFER_SIZE,
            MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
        );
    }
#endif
    if (!frame_buffer) {
        ESP_LOGE(TAG, "framebuffer alloc failed (need %u bytes)", (unsigned) FRAMEBUFFER_SIZE);
        if (out_buffer) {
            *out_buffer = nullptr;
        }
        return;
    }

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
    if (!frame_buffer) return;
    for (int i = 0; i < FRAMEBUFFER_PIXELS; i++)
        frame_buffer[i] = color;
}
