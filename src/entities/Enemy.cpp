#include "entities/Enemy.h"
#include "data/Skill.h"

Enemy::Enemy(const std::string& name, const Stats& stats, Element element)
    : Character(name, stats, element)
{
}

// Enemy thường không tiêu hao MP, dùng skill là gây sát thương luôn.
void Enemy::executeSkill(Skill* skill, Character* target)
{
    if (!canAct(skill, target)) return;
    dealSkillDamage(skill, target, calcSkillDamage(skill));
}