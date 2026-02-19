#include "lumari_config.h"
#include "render_engine.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "input_hal.h"
#include "time_service.h"
#include "imu_service.h"
#include "display_hal.h"
#include "phase0_demo.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LONG_PRESS_MS    800
#define WRIST_DOWN_G     (-7.0f)
#define WRIST_UP_G       (2.0f)
#define WRIST_DEBOUNCE_MS 800

extern "C" void app_entry_start(void)
{
#if LUMARI_RUN_PHASE0
    phase0_demo_run();
#else
    render_engine_init();
    input_hal_init();
    time_service_init();
    imu_service_init();
    creature_engine_init();
    quest_engine_init();

    bool menu_open = false;
    bool btn_prev = false;
    uint32_t btn_down_ms = 0;
    bool display_off = false;
    uint32_t wrist_down_ms = 0;
    uint32_t wrist_up_ms = 0;

    for (;;) {
        uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
        bool btn = input_hal_button_read();

        if (btn) {
            if (!btn_prev)
                btn_down_ms = now_ms;
            if ((now_ms - btn_down_ms) >= LONG_PRESS_MS)
                menu_open = true;
        } else {
            if (btn_prev) {
                uint32_t held = now_ms - btn_down_ms;
                if (held < LONG_PRESS_MS)
                    menu_open = !menu_open;
            }
            btn_prev = false;
        }
        btn_prev = btn;

        int tx = 0, ty = 0;
        if (input_hal_touch_read(&tx, &ty)) {
            if (menu_open)
                menu_open = false;
        }

        unsigned step_delta = imu_service_get_step_delta();
        if (step_delta > 0) {
            quest_engine_add_progress(QUEST_TYPE_STEPS, step_delta);
            creature_engine_add_steps(step_delta);
        }

        int16_t ax = 0, ay = 0, az = 0;
        imu_service_read_accel(&ax, &ay, &az);
        float az_g = (float)az * 0.01f;
        if (az_g < WRIST_DOWN_G) {
            wrist_down_ms = (wrist_down_ms == 0) ? now_ms : wrist_down_ms;
            wrist_up_ms = 0;
            if ((now_ms - wrist_down_ms) >= WRIST_DEBOUNCE_MS && !display_off) {
                display_hal_sleep();
                display_off = true;
            }
        } else if (az_g > WRIST_UP_G) {
            wrist_up_ms = (wrist_up_ms == 0) ? now_ms : wrist_up_ms;
            wrist_down_ms = 0;
            if (display_off) {
                display_hal_wake();
                display_off = false;
            }
        } else {
            wrist_down_ms = 0;
            wrist_up_ms = 0;
        }

        creature_engine_update();
        render_engine_frame(menu_open);
    }
#endif
}
