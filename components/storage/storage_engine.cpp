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
        ESP_LOGE(TAG, "nvs_open failed %s", esp_err_to_name(err));
        s_handle = 0;
    }
}

void storage_save_creature(uint32_t xp, uint32_t momentum)
{
    if (s_handle == 0) return;
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
    if (s_handle == 0) return;
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
    if (s_handle == 0) return;
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
    if (s_handle == 0) return;
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
    if (s_handle == 0) return;
    nvs_set_u32(s_handle, KEY_LORE, lore_bitfield);
    nvs_commit(s_handle);
}

bool storage_load_lore(uint32_t *lore_bitfield)
{
    if (s_handle == 0 || lore_bitfield == NULL) return false;
    if (nvs_get_u32(s_handle, KEY_LORE, lore_bitfield) != ESP_OK) return false;
    return true;
}
