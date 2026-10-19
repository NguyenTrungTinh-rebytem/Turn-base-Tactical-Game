#include "entities/Character.h"
#include <algorithm>
#include "data/Skill.h"

namespace
{
    constexpr float STRONG_MULTIPLIER  = 1.5f;
    constexpr float WEAK_MULTIPLIER    = 0.75f;
    constexpr float NEUTRAL_MULTIPLIER = 1.0f;
    constexpr int   MIN_DAMAGE         = 1;

    bool beats(Element attacker, Element defender)
    {
        return (attacker == Element::WATER && defender == Element::FIRE)  ||
               (attacker == Element::FIRE  && defender == Element::WIND)  ||
               (attacker == Element::WIND  && defender == Element::EARTH) ||
               (attacker == Element::EARTH && defender == Element::WATER);
    }

    float getElementMultiplier(Element attacker, Element defender)
    {
        if (beats(attacker, defender)) return STRONG_MULTIPLIER;
        if (beats(defender, attacker)) return WEAK_MULTIPLIER;
        return NEUTRAL_MULTIPLIER;
    }
}

Character::Character(const std::string& name, const Stats& stats, Element element)
    : m_name(name), m_stats(stats), m_element(element),
      m_currentHp(stats.m_hp), m_currentMp(stats.m_mp)
{
}

void Character::takeDamage(int amount, Element atkElement, bool isMagic)
{
    if (!isAlive()) return;
    const int defense = isMagic ? m_stats.m_md : m_stats.m_d;
    const float multiplier = getElementMultiplier(atkElement, m_element);
    const int finalDamage = std::max(MIN_DAMAGE, static_cast<int>(static_cast<float>(amount) * multiplier) - defense);
    m_currentHp = std::max(0, m_currentHp - finalDamage);
}

bool Character::canAct(const Skill* skill, const Character* target) const
{
    return skill && target && isAlive() && target->isAlive();
}

int Character::calcSkillDamage(const Skill* skill) const
{
    return (skill->m_isMagic ? m_stats.m_ma : m_stats.m_a) + skill->m_power;
}
