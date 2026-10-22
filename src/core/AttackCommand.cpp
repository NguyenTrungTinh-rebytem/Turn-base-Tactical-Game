#include "AttackCommand.h"
#include "Element.h"
#include <iostream>
#include <algorithm>

AttackCommand::AttackCommand(Character* attacker, Character* target)
    : m_attacker(attacker), m_target(target), m_damageDealt(0) {}

void AttackCommand::execute() {
    if (!m_attacker || !m_target || !m_attacker->isAlive()) return;
    int atk = m_attacker->getStats().m_a;
    int def = m_target->getStats().m_d;
    m_damageDealt = std::max(1, atk - def);
    m_target->takeDamage(m_damageDealt, Element::KIM, false);

    std::cout << m_attacker->getName() << " tan cong " 
              << m_target->getName() << " gay " << m_damageDealt << " sat thuong!\n";
}

void AttackCommand::undo() {
    if (m_target && m_damageDealt > 0) {
        m_target->heal(m_damageDealt);
    }
}