#include "TurnManager.h"
#include "../src/entities/Character.h"
#include <algorithm>

bool CharacterCompare::operator()(
    Character* lhs,
    Character* rhs
) const
{
    if (lhs == nullptr)
        return true;

    if (rhs == nullptr)
        return false;

    return lhs->getStats().spd < rhs->getStats().spd;
}

TurnManager::TurnManager()
    : m_currentCharacter(nullptr)
{
}
void TurnManager::addCharacter(Character* character)
{
    if (character == nullptr)
        return;

    if (character->getCurrentHp() <= 0)
        return;

    auto it = std::find(
        m_characters.begin(),
        m_characters.end(),
        character
    );

    if (it != m_characters.end())
        return;

    m_characters.push_back(character);
    m_turnQueue.push(character);
}
Character* TurnManager::getNextTurn()
{
    while (true)
    {
        if (m_turnQueue.empty())
        {
            rebuildQueue();

            if (m_turnQueue.empty())
            {
                m_currentCharacter = nullptr;
                return nullptr;
            }
        }

        Character* character = m_turnQueue.top();
        m_turnQueue.pop();

        if (character == nullptr)
            continue;

        if (character->getCurrentHp() <= 0)
            continue;

        if (character->isStunned())
        {
            character->setStunned(false);
            continue;
        }

        m_currentCharacter = character;
        return m_currentCharacter;
    }
}
void TurnManager::rebuildQueue()
{
    decltype(m_turnQueue) emptyQueue;
    m_turnQueue.swap(emptyQueue);

    for (Character* character : m_characters)
    {
        if (character != nullptr &&
            character->getCurrentHp() > 0)
        {
            m_turnQueue.push(character);
        }
    }
}
void TurnManager::removeCharacter(Character* character)
{
    if (character == nullptr)
        return;

    m_characters.erase(
        std::remove(
            m_characters.begin(),
            m_characters.end(),
            character
        ),
        m_characters.end()
    );

    rebuildQueue();

    if (m_currentCharacter == character)
        m_currentCharacter = nullptr;
}
void TurnManager::clear()
{
    decltype(m_turnQueue) emptyQueue;
    m_turnQueue.swap(emptyQueue);

    m_characters.clear();
    m_currentCharacter = nullptr;
}
bool TurnManager::isEmpty() const
{
    return m_characters.empty();
}
const std::vector<Character*>& TurnManager::getCharacters() const
{
    return m_characters;
}

Character* TurnManager::getCurrentCharacter() const
{
    return m_currentCharacter;
}
