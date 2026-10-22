#include "TurnManager.h"
#include "Character.h"
#include "CommandStack.h"
#include "AttackCommand.h"
#include <algorithm>
#include <iostream>

TurnManager::TurnManager() {}

void TurnManager::addCharacter(Character* character) {
    if (character) {
        m_characters.push_back(character);
    }
}

void TurnManager::removeCharacter(Character* character) {
    m_characters.erase(
        std::remove(m_characters.begin(), m_characters.end(), character),
        m_characters.end()
    );
}

Character* TurnManager::getNextTurn() {
    if (m_characters.empty()) return nullptr;
    
    std::sort(m_characters.begin(), m_characters.end(), [](Character* a, Character* b) {
        return a->getStats().m_spd > b->getStats().m_spd;
    });

    for (auto* c : m_characters) {
        if (c && c->isAlive()) {
            m_currentCharacter = c;
            return c;
        }
    }
    return nullptr;
}

Character* TurnManager::getCurrentCharacter() const {
    return m_currentCharacter;
}

void TurnManager::rebuildQueue() {}

void TurnManager::clear() {
    m_characters.clear();
    m_currentCharacter = nullptr;
}

bool TurnManager::isEmpty() const {
    return m_characters.empty();
}

const std::vector<Character*>& TurnManager::getCharacters() const {
    return m_characters;
}

void TurnManager::processTurn(CommandStack& commandStack) {
    Character* current = getNextTurn();
    if (!current || !current->isAlive()) return;

    if (current->isStunned()) {
        current->setStunned(false);
        return;
    }

    // Tự động tìm mục tiêu còn sống khác bản thân trong trận đấu để tấn công
    Character* target = nullptr;
    for (auto* c : m_characters) {
        if (c && c->isAlive() && c != current) {
            target = c;
            break;
        }
    }

    if (target) {
        // Tạo unique_ptr và dùng std::move để khớp tham số CommandStack
        auto cmd = std::make_unique<AttackCommand>(current, target);
        commandStack.executeCommand(std::move(cmd));
    }
}