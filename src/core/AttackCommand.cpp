#include "AttackCommand.h"
#include "Element.h"
#include <iostream>
#include <algorithm>

AttackCommand::AttackCommand(Character* attacker, Character* target)
    : m_attacker(attacker), m_target(target), m_damageDealt(0) {}

void AttackCommand::execute() {
    if (!m_attacker || !m_target || !m_attacker->isAlive()) return;
    // takeDamage() da tu tru giap + nhan he so khac he -> truyen thang ATK va he cua nguoi tan cong.
    int hpBefore = m_target->getCurrentHp();
    m_target->takeDamage(m_attacker->getStats().m_a, m_attacker->getElement(), false);
    m_damageDealt = hpBefore - m_target->getCurrentHp();   // luong HP that su mat (de undo chinh xac)

    std::cout << m_attacker->getName() << " tan cong "
              << m_target->getName() << " gay " << m_damageDealt << " sat thuong!\n";
}

void AttackCommand::undo() {
    if (m_target && m_damageDealt > 0) {
        m_target->heal(m_damageDealt);
    }
}