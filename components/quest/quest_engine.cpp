#include "quest_engine.h"
#include "creature_engine.h"

typedef struct {
    quest_type_t type;
    uint32_t goal;
    uint32_t reward_xp;
} quest_def_t;

static const quest_def_t s_quests[] = {
    { QUEST_TYPE_STEPS,  50,  10 },
    { QUEST_TYPE_STEPS, 100,  25 },
    { QUEST_TYPE_STEPS, 200,  50 },
};
static const unsigned s_quest_count = sizeof(s_quests) / sizeof(s_quests[0]);

static unsigned s_current = 0;
static uint32_t s_progress = 0;
static bool s_just_completed = false;

void quest_engine_init(void)
{
    s_current = 0;
    s_progress = 0;
    s_just_completed = false;
}

void quest_engine_add_progress(quest_type_t type, uint32_t amount)
{
    s_just_completed = false;
    if (s_current >= s_quest_count)
        return;
    const quest_def_t *q = &s_quests[s_current];
    if (q->type != type)
        return;
    s_progress += amount;
    if (s_progress >= q->goal) {
        creature_engine_add_xp(q->reward_xp);
        s_just_completed = true;
        s_progress = 0;
        s_current++;
        if (s_current >= s_quest_count)
            s_current = 0;
    }
}

uint32_t quest_engine_get_current_quest_id(void)
{
    return s_current + 1;
}

uint32_t quest_engine_get_progress(void)
{
    return s_progress;
}

uint32_t quest_engine_get_goal(void)
{
    if (s_current >= s_quest_count)
        return 0;
    return s_quests[s_current].goal;
}

bool quest_engine_just_completed(void)
{
    return s_just_completed;
}
