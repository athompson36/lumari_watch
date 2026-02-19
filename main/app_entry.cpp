#include "render_engine.h"
#include "creature_engine.h"

extern "C" void app_entry_start()
{
    render_engine_init();
    creature_engine_init();

    while (true)
    {
        creature_engine_update();
        render_engine_frame();
    }
}
