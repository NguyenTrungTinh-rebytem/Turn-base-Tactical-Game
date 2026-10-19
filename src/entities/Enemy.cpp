#include "entities/Enemy.h"
#include "data/Skill.h"

Enemy::Enemy(const std::string& name, const Stats& stats, Element element)
    : Character(name, stats, element)
{
}

void Enemy::executeSkill(Skill* skill, Character* target)
{
    if (!canAct(skill, target)) return;
    target->takeDamage(calcSkillDamage(skill), skill->m_element, skill->m_isMagic);
}
