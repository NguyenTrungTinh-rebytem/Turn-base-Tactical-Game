#include "entities/Item.h"

Item::Item(const std::string& name, const std::string& description)
    : m_name(name), m_description(description)
{
}

// ---------------- Potion (hồi HP) ----------------

Potion::Potion(int healAmount)
    : Item("Potion", "Hoi " + std::to_string(healAmount) + " HP"),
      m_healAmount(healAmount)
{
}

bool Potion::canUse(const Character& target) const
{
    return target.isAlive() && target.getCurrentHp() < target.getMaxHp();
}

int Potion::use(Character& target)
{
    if (!canUse(target)) return 0;

    const int before = target.getCurrentHp();
    target.heal(m_healAmount);
    return target.getCurrentHp() - before;
}

void Potion::revert(Character& target, int amount)
{
    if (amount > 0) target.takeDamage(amount);   // trừ thẳng HP, bỏ qua giáp/ngũ hành
}

// ---------------- ManaPotion (hồi MP) ----------------

ManaPotion::ManaPotion(int restoreAmount)
    : Item("Mana Potion", "Hoi " + std::to_string(restoreAmount) + " MP"),
      m_restoreAmount(restoreAmount)
{
}

bool ManaPotion::canUse(const Character& target) const
{
    return target.isAlive() && target.getCurrentMp() < target.getMaxMp();
}

int ManaPotion::use(Character& target)
{
    if (!canUse(target)) return 0;
    return target.restoreMp(m_restoreAmount);
}

void ManaPotion::revert(Character& target, int amount)
{
    if (amount > 0) target.consumeMp(amount);
}
