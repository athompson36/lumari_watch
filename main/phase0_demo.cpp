#include "lumari_config.h"
#include "framebuffer_manager.h"
#include "display_hal.h"
#include "input_hal.h"
#include "imu_service.h"
#include "time_service.h"
#include "sprite_renderer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "phase0";

static const uint16_t s_colors[] = {
    0xF800,  /* red */
    0x07E0,  /* green */
    0x001F,  /* blue */
    0xFFE0,  /* yellow */
};
static const int s_num_colors = sizeof(s_colors) / sizeof(s_colors[0]);

static const uint8_t s_brightness[] = { 0, 50, 100 };
static const int s_num_brightness = sizeof(s_brightness) / sizeof(s_brightness[0]);

void phase0_demo_run(void)
{
    ESP_LOGI(TAG, "Phase 0 demo: circle, tap=color, button=brightness, shake=log");

    uint16_t *fb = nullptr;
    framebuffer_manager_init(&fb);
    if (!fb) {
        ESP_LOGE(TAG, "No framebuffer");
        return;
    }

    display_hal_init();
    input_hal_init();
    imu_service_init();
    time_service_init();

    int color_index = 0;
    int brightness_index = 1;  /* start at 50% */
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;
    const int radius = 60;
    bool prev_button = false;
    bool prev_touch = false;

    display_hal_set_brightness(s_brightness[brightness_index]);

    while (1) {
        /* Touch -> cycle circle color on touch-down only */
        int tx = 0, ty = 0;
        bool touch = input_hal_touch_read(&tx, &ty);
        if (touch && !prev_touch) {
            color_index = (color_index + 1) % s_num_colors;
        }
        prev_touch = touch;

        /* Button -> cycle brightness on press (edge) */
        bool btn = input_hal_button_read();
        if (btn && !prev_button) {
            brightness_index = (brightness_index + 1) % s_num_brightness;
            display_hal_set_brightness(s_brightness[brightness_index]);
        }
        prev_button = btn;

        /* Shake -> debug log */
        if (imu_service_shake_detected()) {
            ESP_LOGI(TAG, "Shake!");
        }

        framebuffer_clear(0x0000);
        draw_fill_circle(fb, cx, cy, radius, s_colors[color_index]);
        display_hal_flush(fb);

        vTaskDelay(pdMS_TO_TICKS(1000 / TARGET_FPS_IDLE));
    }
}
