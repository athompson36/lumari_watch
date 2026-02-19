#include "inventory_engine.h"
#include "sprite_renderer.h"
#include "lumari_config.h"

#define COLOR_GOLD   0xFEC0
#define COLOR_YELLOW 0xFFE0
#define COLOR_RED    0xF800
#define COLOR_BLUE   0x001F

static const accessory_def_t s_defs[] = {
    { 1, "CROWN",  50,  ACCESSORY_TYPE_HAT  },
    { 2, "HALO",   100, ACCESSORY_TYPE_HAT  },
    { 3, "SCARF",  150, ACCESSORY_TYPE_BODY },
    { 4, "SHIELD", 250, ACCESSORY_TYPE_BODY },
};
static const unsigned s_count = sizeof(s_defs) / sizeof(s_defs[0]);

static uint32_t s_unlocked = 0;
static uint8_t s_equipped = 0;

void inventory_init(void)
{
    s_unlocked = 0;
    s_equipped = 0;
}

void inventory_check_unlocks(uint32_t xp)
{
    for (unsigned i = 0; i < s_count; i++) {
        if (xp >= s_defs[i].unlock_xp)
            s_unlocked |= (1u << i);
    }
}

void inventory_equip(uint8_t id)
{
    if (id == 0) {
        s_equipped = 0;
        return;
    }
    for (unsigned i = 0; i < s_count; i++) {
        if (s_defs[i].id == id && (s_unlocked & (1u << i)))
            s_equipped = id;
    }
}

void inventory_unequip(void)
{
    s_equipped = 0;
}

uint8_t inventory_get_equipped(void)
{
    return s_equipped;
}

bool inventory_get_unlocked(uint8_t id)
{
    if (id == 0) return true;
    for (unsigned i = 0; i < s_count; i++)
        if (s_defs[i].id == id)
            return (s_unlocked & (1u << i)) != 0;
    return false;
}

unsigned inventory_get_count(void)
{
    return s_count;
}

const accessory_def_t *inventory_get_def(uint8_t id)
{
    for (unsigned i = 0; i < s_count; i++)
        if (s_defs[i].id == id)
            return &s_defs[i];
    return nullptr;
}

uint8_t inventory_next_unlocked(uint8_t current)
{
    /* Cycle: 0 -> first unlocked -> ... -> last unlocked -> 0 */
    bool found_current = (current == 0);
    for (unsigned i = 0; i < s_count; i++) {
        if (!(s_unlocked & (1u << i))) continue;
        if (found_current)
            return s_defs[i].id;
        if (s_defs[i].id == current)
            found_current = true;
    }
    return 0;
}

uint8_t inventory_prev_unlocked(uint8_t current)
{
    /* Cycle backwards: 0 -> last unlocked -> ... -> first unlocked -> 0 */
    if (current == 0) {
        for (int i = (int)s_count - 1; i >= 0; i--) {
            if (s_unlocked & (1u << (unsigned)i))
                return s_defs[i].id;
        }
        return 0;
    }
    uint8_t prev_id = 0;
    for (unsigned i = 0; i < s_count; i++) {
        if (!(s_unlocked & (1u << i))) continue;
        if (s_defs[i].id >= current) break;
        prev_id = s_defs[i].id;
    }
    return prev_id;
}

void inventory_set_unlocked_bitfield(uint32_t bitfield)
{
    s_unlocked = bitfield;
}

void inventory_set_equipped(uint8_t id)
{
    s_equipped = id;
}

uint32_t inventory_get_unlocked_bitfield(void)
{
    return s_unlocked;
}

static void draw_crown(uint16_t *fb, int cx, int head_cy, int head_r)
{
    int apex_y = head_cy - head_r - 12;
    draw_fill_triangle(fb, cx, apex_y + 8, 10, 16, COLOR_GOLD);
}

static void draw_halo(uint16_t *fb, int cx, int head_cy, int head_r)
{
    int halo_y = head_cy - head_r - 10;
    draw_fill_circle(fb, cx, halo_y, 10, COLOR_YELLOW);
}

static void draw_scarf(uint16_t *fb, int cx, int head_cy, int head_r)
{
    int scarf_y = head_cy - head_r - 4;
    draw_rect(fb, cx - 22, scarf_y, 44, 10, COLOR_RED);
}

static void draw_shield(uint16_t *fb, int cx, int body_cy, int body_r)
{
    draw_fill_circle(fb, cx + body_r + 8, body_cy, 12, COLOR_BLUE);
}

void inventory_draw_accessory(
    uint16_t *framebuffer,
    int cx, int head_cy, int body_cy, int body_r, int head_r
)
{
    if (s_equipped == 0) return;
    for (unsigned i = 0; i < s_count; i++) {
        if (s_defs[i].id != s_equipped) continue;
        switch (s_defs[i].type) {
            case ACCESSORY_TYPE_HAT:
                if (s_defs[i].id == 1) draw_crown(framebuffer, cx, head_cy, head_r);
                else if (s_defs[i].id == 2) draw_halo(framebuffer, cx, head_cy, head_r);
                break;
            case ACCESSORY_TYPE_BODY:
                if (s_defs[i].id == 3) draw_scarf(framebuffer, cx, head_cy, head_r);
                else if (s_defs[i].id == 4) draw_shield(framebuffer, cx, body_cy, body_r);
                break;
            default:
                break;
        }
        break;
    }
}
