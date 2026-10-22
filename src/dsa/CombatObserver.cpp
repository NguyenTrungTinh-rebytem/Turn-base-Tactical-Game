#include "CombatObserver.h"
void SubjectCombat::addObserver(ICombatObserver* observer)
{
    if (observer == nullptr)
    {
        return;
    }
    for (ICombatObserver* item : m_observers)
    {
        if (item == observer)
        {
            return;
        }
    }

    m_observers.push_back(observer);
}
void SubjectCombat::removeObserver(ICombatObserver* observer)
{
    m_observers.erase(
        std::remove(
            m_observers.begin(),
            m_observers.end(),
            observer
        ),
        m_observers.end()
    );
}
void SubjectCombat::notifyHealthChanged(
    Character* character,
    int newHealth
)
{
    for (ICombatObserver* observer : m_observers)
    {
        if (observer != nullptr)
        {
            observer->onHealthChanged(character, newHealth);
        }
    }
}
void SubjectCombat::notifyCharacterDied(Character* character)
{
    for (ICombatObserver* observer : m_observers)
    {
        if (observer != nullptr)
        {
            observer->onCharacterDied(character);
        }
    }
}
void SubjectCombat::clearObservers()
{
    m_observers.clear();
}
