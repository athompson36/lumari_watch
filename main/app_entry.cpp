#include "lumari_config.h"
#include "render_engine.h"
#include "creature_engine.h"
#include "quest_engine.h"
#include "inventory_engine.h"
#include "aura_engine.h"
#include "cutscene_engine.h"
#include "storage_engine.h"
#include "layer_manager.h"
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
    storage_init();
    render_engine_init();
    input_hal_init();
    time_service_init();
    imu_service_init();
    creature_engine_init();
    quest_engine_init();
    inventory_init();
    aura_engine_init();
    cutscene_init();

    /* Load persisted state */
    {
        uint32_t xp = 0, momentum = 0;
        if (storage_load_creature(&xp, &momentum)) {
            creature_engine_set_state((unsigned)xp, (unsigned)momentum);
        }
        unsigned qidx = 0;
        uint32_t qprog = 0;
        if (storage_load_quest(&qidx, &qprog)) {
            quest_engine_set_state(qidx, qprog);
        }
        uint8_t equip = 0;
        uint32_t unl = 0;
        if (storage_load_inventory(&equip, &unl)) {
            inventory_set_unlocked_bitfield(unl);
            inventory_check_unlocks((uint32_t)creature_engine_get_xp());
            inventory_set_equipped(equip);
        }
        bool aura_crafted_val = false;
        if (storage_load_aura(&aura_crafted_val)) {
            aura_set_crafted(aura_crafted_val);
        }
        uint32_t lore_bf = 0;
        if (storage_load_lore(&lore_bf)) {
            cutscene_lore_set_bitfield(lore_bf);
        }
    }

    bool menu_open = false;
    bool lore_menu_open = false;
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
                else if (menu_open) {
                    menu_open = false;
                    lore_menu_open = false;
                    storage_save_inventory(inventory_get_equipped(), inventory_get_unlocked_bitfield());
                }
            }
            btn_prev = false;
        }
        btn_prev = btn;

        int tx = 0, ty = 0;
        if (input_hal_touch_read(&tx, &ty)) {
            if (cutscene_is_active()) {
                cutscene_advance();
            } else if (menu_open) {
                int action = layer_manager_menu_handle_touch(tx, ty, SCREEN_WIDTH, SCREEN_HEIGHT, lore_menu_open);
                if (action == LAYER_ACTION_CRAFT && creature_engine_spend_xp(AURA_CRAFT_COST_XP)) {
                    aura_do_craft();
                    storage_save_creature((uint32_t)creature_engine_get_xp(), (uint32_t)creature_engine_get_momentum());
                    storage_save_aura(true);
                } else if (action == LAYER_ACTION_PLAY_CUTSCENE) {
                    cutscene_start(CUTSCENE_ID_EVOLUTION);
                    menu_open = false;
                    lore_menu_open = false;
                } else if (action == LAYER_ACTION_OPEN_LORE) {
                    lore_menu_open = true;
                } else if (action == LAYER_ACTION_CLOSE_LORE) {
                    lore_menu_open = false;
                }
            } else {
                (void)tx;
                (void)ty;
            }
        }

        /* First-time evolution cutscene: unlock and play once when XP >= 100 */
        if (!cutscene_is_active() && creature_engine_get_xp() >= 100 && !cutscene_lore_unlocked(CUTSCENE_ID_EVOLUTION)) {
            cutscene_lore_set_unlocked(CUTSCENE_ID_EVOLUTION);
            storage_save_lore(cutscene_lore_get_bitfield());
            cutscene_start(CUTSCENE_ID_EVOLUTION);
        }

        unsigned step_delta = imu_service_get_step_delta();
        if (step_delta > 0) {
            bool completed = quest_engine_just_completed();
            quest_engine_add_progress(QUEST_TYPE_STEPS, step_delta);
            if (quest_engine_just_completed())
                completed = true;
            creature_engine_add_steps(step_delta);
            if (completed) {
                storage_save_creature((uint32_t)creature_engine_get_xp(), (uint32_t)creature_engine_get_momentum());
                storage_save_quest(quest_engine_get_current_quest_id() > 0 ? (unsigned)quest_engine_get_current_quest_id() - 1u : 0u, quest_engine_get_progress());
                storage_save_inventory(inventory_get_equipped(), inventory_get_unlocked_bitfield());
            }
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
        render_engine_frame(menu_open, now_ms, lore_menu_open);
    }
#endif
}
