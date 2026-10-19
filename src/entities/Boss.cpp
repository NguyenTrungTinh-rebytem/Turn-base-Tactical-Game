#include "entities/Boss.h"
#include "data/Skill.h"

Boss::Boss(const std::string& name, const Stats& stats, Element element)
    : Character(name, stats, element)
{
}

void Boss::takeDamage(int amount, Element atkElement, bool isMagic)
{
    const int reducedAmount = static_cast<int>(static_cast<float>(amount) * DAMAGE_TAKEN_RATE);
    Character::takeDamage(reducedAmount, atkElement, isMagic);
}

void Boss::executeSkill(Skill* skill, Character* target)
{
    if (!canAct(skill, target)) return;
    const int damage = static_cast<int>(static_cast<float>(calcSkillDamage(skill)) * DAMAGE_DEALT_RATE);
    target->takeDamage(damage, skill->m_element, skill->m_isMagic);
}
