#include "entities/Player.h"
#include "data/Skill.h"

Player::Player(const std::string& name, const Stats& stats, Element element,
               const std::string& raceName, const std::string& className)
    : Character(name, stats, element), m_raceName(raceName), m_className(className)
{
}

// Player phải đủ MP mới dùng được skill.
void Player::executeSkill(Skill* skill, Character* target)
{
    if (!canAct(skill, target)) return;
    if (m_currentMp < skill->m_mpCost) return;  // Không đủ MP: bỏ qua

    m_currentMp -= skill->m_mpCost;
    dealSkillDamage(skill, target, calcSkillDamage(skill));
}