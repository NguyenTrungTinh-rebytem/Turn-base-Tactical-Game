#pragma once
#include <vector>
#include <algorithm>
class Character;
class ICombatObserver
{
public:
    virtual ~ICombatObserver() = default;
    virtual void onHealthChanged(Character* character, int newHealth) = 0;
    virtual void onCharacterDied(Character* character) = 0;
};

class SubjectCombat
{
private:
    std::vector<ICombatObserver*> m_observers;

public:
    void addObserver(ICombatObserver* observer);
    void removeObserver(ICombatObserver* observer);
    void notifyHealthChanged(Character* character, int newHealth);
    void notifyCharacterDied(Character* character);
    void clearObservers();
};
