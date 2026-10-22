#pragma once
#include <string>
#include <vector>
#include "data/Stats.h"
#include "data/Element.h"

enum class EffectType { NONE, ATK_BOOST, DEF_BOOST, REGEN, STUN, BURN, SLOW, WEAKNESS };

enum class SkillType { DAMAGE, HEAL, BUFF, DEBUFF };

enum class SkillOwnerType { RACE, CLASS };

struct SkillData {
    std::string m_id;
    std::string m_name;
    SkillOwnerType m_ownerType = SkillOwnerType::RACE;
    std::string m_owner;
    SkillType m_type = SkillType::DAMAGE;
    int m_power = 0;
    int m_mpCost = 0;
    EffectType m_effect = EffectType::NONE;
    int m_effectTurns = 0;
};

struct RaceData {
    std::string m_name;
    Stats m_stats;
    std::vector<std::string> m_skillIds;
};

struct ClassData {
    std::string m_name;
    Stats m_stats;
    std::vector<std::string> m_skillIds;
};

struct MonsterData {
    std::string m_id;
    std::string m_name;
    Element m_element = Element::KIM;
    bool m_isBoss = false;
    int m_stage = 1;
    Stats m_stats;
};
