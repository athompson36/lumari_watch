#include "lumari_config.h"
#include "render_engine.h"
#include "creature_engine.h"
#include "input_hal.h"
#include "time_service.h"
#include "imu_service.h"
#include "phase0_demo.h"

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
    for (;;) {
        creature_engine_update();
        render_engine_frame();
    }
#endif
}
