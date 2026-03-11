#include "storage_engine.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "storage";
static nvs_handle_t s_handle = 0;

static const char *KEY_XP = "xp";
static const char *KEY_MOMENTUM = "mom";
static const char *KEY_QUEST_IDX = "qi";
static const char *KEY_QUEST_PROG = "qp";
static const char *KEY_EQUIP_ID = "eq";
static const char *KEY_UNLOCKED = "unl";
static const char *KEY_AURA = "aura";
static const char *KEY_LORE = "lore";
static const char *KEY_BRIGHT = "br";
static const char *KEY_24H = "24h";
static const char *KEY_WIFI = "wifi";
static const char *KEY_BT = "bt";

static bool s_storage_unavailable_logged = false;

static void storage_warn_if_unavailable(void)
{
    if (s_handle != 0) return;
    if (s_storage_unavailable_logged) return;
    s_storage_unavailable_logged = true;
    ESP_LOGW(TAG, "NVS unavailable — saves and loads are no-op; progress will not persist");
}

void storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed %s", esp_err_to_name(err));
        return;
    }
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &s_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed %s; retrying after erase", esp_err_to_name(err));
        nvs_flash_erase();
        if (nvs_flash_init() == ESP_OK)
            err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &s_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_open failed %s — persistence disabled", esp_err_to_name(err));
            s_handle = 0;
        }
    }
}

void storage_save_creature(uint32_t xp, uint32_t momentum)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u32(s_handle, KEY_XP, xp);
    nvs_set_u32(s_handle, KEY_MOMENTUM, momentum);
    nvs_commit(s_handle);
}

bool storage_load_creature(uint32_t *xp, uint32_t *momentum)
{
    if (s_handle == 0 || xp == NULL || momentum == NULL) return false;
    if (nvs_get_u32(s_handle, KEY_XP, xp) != ESP_OK) return false;
    if (nvs_get_u32(s_handle, KEY_MOMENTUM, momentum) != ESP_OK) return false;
    return true;
}

void storage_save_quest(unsigned quest_index, uint32_t progress)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u32(s_handle, KEY_QUEST_IDX, (uint32_t)quest_index);
    nvs_set_u32(s_handle, KEY_QUEST_PROG, progress);
    nvs_commit(s_handle);
}

bool storage_load_quest(unsigned *quest_index, uint32_t *progress)
{
    if (s_handle == 0 || quest_index == NULL || progress == NULL) return false;
    uint32_t qi, qp;
    if (nvs_get_u32(s_handle, KEY_QUEST_IDX, &qi) != ESP_OK) return false;
    if (nvs_get_u32(s_handle, KEY_QUEST_PROG, &qp) != ESP_OK) return false;
    *quest_index = (unsigned)qi;
    *progress = qp;
    return true;
}

void storage_save_inventory(uint8_t equipped_id, uint32_t unlocked_bitfield)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u8(s_handle, KEY_EQUIP_ID, equipped_id);
    nvs_set_u32(s_handle, KEY_UNLOCKED, unlocked_bitfield);
    nvs_commit(s_handle);
}

bool storage_load_inventory(uint8_t *equipped_id, uint32_t *unlocked_bitfield)
{
    if (s_handle == 0 || equipped_id == NULL || unlocked_bitfield == NULL) return false;
    if (nvs_get_u8(s_handle, KEY_EQUIP_ID, equipped_id) != ESP_OK) return false;
    if (nvs_get_u32(s_handle, KEY_UNLOCKED, unlocked_bitfield) != ESP_OK) return false;
    return true;
}

void storage_save_aura(bool crafted)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u8(s_handle, KEY_AURA, crafted ? 1 : 0);
    nvs_commit(s_handle);
}

bool storage_load_aura(bool *crafted)
{
    if (s_handle == 0 || crafted == NULL) return false;
    uint8_t v = 0;
    if (nvs_get_u8(s_handle, KEY_AURA, &v) != ESP_OK) return false;
    *crafted = (v != 0);
    return true;
}

void storage_save_lore(uint32_t lore_bitfield)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u32(s_handle, KEY_LORE, lore_bitfield);
    nvs_commit(s_handle);
}

/* Bit 0 = evolution, bit 1 = Aetheron intro, bit 2 = Pixel mode. Aetheron + Pixel unlocked by default (first run). */
#define LORE_DEFAULT_FIRST_RUN  ((1u << 1) | (1u << 2))

bool storage_load_lore(uint32_t *lore_bitfield)
{
    if (s_handle == 0 || lore_bitfield == NULL) return false;
    esp_err_t err = nvs_get_u32(s_handle, KEY_LORE, lore_bitfield);
    if (err == ESP_OK) return true;
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *lore_bitfield = LORE_DEFAULT_FIRST_RUN;
        return true;
    }
    return false;
}

void storage_save_settings(uint8_t brightness, uint8_t time_24h, uint8_t wifi_on, uint8_t bt_on)
{
    if (s_handle == 0) { storage_warn_if_unavailable(); return; }
    nvs_set_u8(s_handle, KEY_BRIGHT, brightness);
    nvs_set_u8(s_handle, KEY_24H, time_24h);
    nvs_set_u8(s_handle, KEY_WIFI, wifi_on);
    nvs_set_u8(s_handle, KEY_BT, bt_on);
    nvs_commit(s_handle);
}

bool storage_load_settings(uint8_t *brightness, uint8_t *time_24h, uint8_t *wifi_on, uint8_t *bt_on)
{
    if (s_handle == 0) return false;
    uint8_t b = 80, t = 0, w = 0, bt = 0;
    if (nvs_get_u8(s_handle, KEY_BRIGHT, &b) != ESP_OK) b = 80;
    if (b < 30) b = 30; /* never load a black-screen value */
    if (nvs_get_u8(s_handle, KEY_24H, &t) != ESP_OK) t = 0;
    if (nvs_get_u8(s_handle, KEY_WIFI, &w) != ESP_OK) w = 0;
    if (nvs_get_u8(s_handle, KEY_BT, &bt) != ESP_OK) bt = 0;
    if (brightness) *brightness = b;
    if (time_24h) *time_24h = t;
    if (wifi_on) *wifi_on = w;
    if (bt_on) *bt_on = bt;
    return true;
}
