#include "render_engine.h"
#include "framebuffer_manager.h"
#include "layer_manager.h"
#include "display_hal.h"
#include "lumari_config.h"

static uint16_t* frame_buffer = nullptr;
static uint32_t frame_delay_ms = 1000 / TARGET_FPS_IDLE;

void render_engine_init()
{
    framebuffer_manager_init(&frame_buffer);
    display_hal_init();
}

void render_engine_frame()
{
    framebuffer_clear(0x0000);
    layer_manager_render(frame_buffer);
    display_hal_flush(frame_buffer);
    vTaskDelay(pdMS_TO_TICKS(frame_delay_ms));
}
