#include "AttackCommand.h"
#include "data/Skill.h"
#include <cmath>
#include <iostream>

AttackCommand::AttackCommand(Character* attacker, Character* target, float damageMultiplier)
    : m_attacker(attacker), m_target(target), m_damageDealt(0), m_damageMultiplier(damageMultiplier) {}

void AttackCommand::execute()
{
    if (!m_attacker || !m_target || !m_attacker->isAlive() || !m_target->isAlive()) return;
    m_attackerBefore = std::make_unique<CharacterSnapshot>(*m_attacker);
    m_targetBefore = std::make_unique<CharacterSnapshot>(*m_target);

    Skill basic;
    basic.m_id = "BASIC_ATTACK";
    basic.m_name = "Tan cong";
    basic.m_element = m_attacker->getElement();
    basic.m_type = SkillType::DAMAGE;
    basic.m_mpCost = 0;
    basic.m_isMagic = false;
    basic.m_power = static_cast<int>(std::lround(m_attacker->getAttack() * (m_damageMultiplier - 1.0f)));
    const int before = m_target->getCurrentHp();
    m_attacker->executeSkill(&basic, m_target);
    m_damageDealt = before - m_target->getCurrentHp();
    std::cout << m_attacker->getName() << " tan cong " << m_target->getName()
              << " gay " << m_damageDealt << " sat thuong!\n";
}

void AttackCommand::undo()
{
    if (m_target && m_targetBefore) m_targetBefore->restore(*m_target);
    if (m_attacker && m_attackerBefore) m_attackerBefore->restore(*m_attacker);
}
