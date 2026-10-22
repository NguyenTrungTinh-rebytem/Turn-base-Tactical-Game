#pragma once
#include <string>
#include "DataTypes.h"

struct Skill {
    std::string m_id;
    std::string m_name;
    SkillOwnerType m_ownerType = SkillOwnerType::RACE;
    std::string m_owner;
    SkillType m_type = SkillType::DAMAGE;
    int m_power = 0;
    int m_mpCost = 0;
    EffectType m_effect = EffectType::NONE;
    int m_effectTurns = 0;
    Element m_element = Element::KIM;
    bool m_isMagic = false;
};
