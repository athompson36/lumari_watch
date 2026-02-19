#include "layer_manager.h"
#include "creature_engine.h"

void layer_manager_render(uint16_t* framebuffer)
{
    creature_engine_render(framebuffer);
}
