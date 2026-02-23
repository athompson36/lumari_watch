# Lumari Watch — Audit action items

From **LumARI Watch Full Project Audit (February 2026)**. Track fixes and follow-ups here.

## Priority 1 — Before hardware validation

| ID | Issue | Status | Notes |
|----|--------|--------|--------|
| R1 | **BUG-01:** `LUMARI_RUN_PHASE0` default 1 | ✅ | Set to 0 so production Lumari loop runs by default |
| R2 | **BUG-03:** Touch coordinate scaling (0–4095 wrong for FT3168) | ✅ | FT3168 reports panel coords; map/clamp to screen |
| R3 | **BUG-02:** Quest index off-by-one on save | ✅ | Use `quest_engine_get_current_quest_index()` (0-based) for saves |

## Priority 2 — Fix soon (before Phase 4)

| ID | Issue | Status | Notes |
|----|--------|--------|--------|
| R4 | **BUG-04:** Triple IMU reads per frame | ✅ | `imu_service_update()` caches one read; shake/step/accel use cache |
| R5 | **BUG-06:** `btn_prev` logic | ✅ | Set `btn_prev = btn` once at end of loop |
| R6 | **BUG-05:** NULL framebuffer | ✅ | Guard in `render_engine_frame` before draw |
| R7 | Menu coordinates absolute pixels | ✅ | Express as fractions of `SCREEN_HEIGHT` for QEMU 198×240 |
| R8 | LAYER_ACTION_* magic ints | ✅ | Already in `layer_manager.h` |
| R9 | Duplicate `draw_ring` in aura_engine | ⏸️ | Left as local copy to avoid engine↔aura circular dep; comment added |

## Priority 3 — Later (Phase 4+)

| Item | Status | Notes |
|------|--------|--------|
| Split main loop into render task and sensor/BLE task | ✅ | Two FreeRTOS tasks: `sensor` (input, IMU, creature, quest, storage, 20 ms period) and `render` (menu_open/lore_menu_open via mutex, render_engine_frame). Fallback to single-threaded if mutex create fails. |
| AXP2101 driver: battery, rail gating, charge state | ✅ | `components/services/power_service`: init, battery mV, charge state, battery %, VBUS present, ALDO mask for rail gating. Waveshare I2C 0x34; stubbed on QEMU. Init called from app_entry. |
| Lightweight test harness (e.g. quest_engine, inventory) | ✅ | In-app Unity tests in `main/test_harness.c`; enable **CONFIG_LUMARI_RUN_UNIT_TESTS** in menuconfig, then build and run (e.g. `idf.py qemu`). Tests run once at startup. Optional separate `test/` project (QEMU config) for standalone test binary; in-app runner preferred. |
| Optional CHANGELOG.md | — | |

## Storage / robustness

| ID | Issue | Status | Notes |
|----|--------|--------|--------|
| — | **storage_init** nvs_open fail → silent no-op | ✅ | Log on fail; one retry (erase + init + open); one-time LOGW on first save when handle invalid |
