#include "cutscene_engine.h"
#include "lumari_config.h"

static uint32_t s_lore_bitfield = 0;
static bool s_playing = false;
static unsigned s_cutscene_id = 0;
static unsigned s_slide_index = 0;

/* Evolution cutscene: 4 slides, tap to advance. */
static const char *s_evolution_slides[] = {
    "YOUR LUMARI GROWS...",
    "SOMETHING STIRS.",
    "A NEW LIGHT AWAKENS.",
    "EVOLUTION.",
};
static const unsigned s_evolution_slide_count = sizeof(s_evolution_slides) / sizeof(s_evolution_slides[0]);

#define CUTSCENE_BG_COLOR    0x0000
#define CUTSCENE_TEXT_COLOR  0xFFFF
#define CUTSCENE_TAP_Y       (SCREEN_HEIGHT - 24)

static unsigned cutscene_slide_count(unsigned id)
{
    if (id == CUTSCENE_ID_EVOLUTION) return s_evolution_slide_count;
    return 0;
}

static const char *cutscene_slide_line(unsigned id, unsigned index)
{
    if (id == CUTSCENE_ID_EVOLUTION && index < s_evolution_slide_count)
        return s_evolution_slides[index];
    return "";
}

void cutscene_lore_set_bitfield(uint32_t bitfield)
{
    s_lore_bitfield = bitfield;
}

uint32_t cutscene_lore_get_bitfield(void)
{
    return s_lore_bitfield;
}

bool cutscene_lore_unlocked(unsigned cutscene_id)
{
    if (cutscene_id >= 32) return false;
    return (s_lore_bitfield & (1u << cutscene_id)) != 0;
}

void cutscene_lore_set_unlocked(unsigned cutscene_id)
{
    if (cutscene_id < 32)
        s_lore_bitfield |= (1u << cutscene_id);
}

void cutscene_init(void)
{
    s_playing = false;
    s_cutscene_id = 0;
    s_slide_index = 0;
}

bool cutscene_is_active(void)
{
    return s_playing;
}

unsigned cutscene_get_id(void)
{
    return s_cutscene_id;
}

void cutscene_start(unsigned cutscene_id)
{
    if (cutscene_slide_count(cutscene_id) == 0) return;
    s_playing = true;
    s_cutscene_id = cutscene_id;
    s_slide_index = 0;
}

bool cutscene_advance(void)
{
    if (!s_playing) return false;
    unsigned count = cutscene_slide_count(s_cutscene_id);
    if (s_slide_index + 1 >= count) {
        s_playing = false;
        return false;
    }
    s_slide_index++;
    return true;
}

const char *cutscene_get_current_line(void)
{
    if (!s_playing) return "";
    unsigned count = cutscene_slide_count(s_cutscene_id);
    if (s_slide_index >= count) return "";
    return cutscene_slide_line(s_cutscene_id, s_slide_index);
}
