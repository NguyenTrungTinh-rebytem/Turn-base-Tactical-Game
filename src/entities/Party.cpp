#include "entities/Party.h"
#include <algorithm>

bool Party::addMember(std::shared_ptr<Player> hero)
{
    if (!hero || isFull()) return false;
    if (std::find(m_members.begin(), m_members.end(), hero) != m_members.end()) return false;

    m_members.push_back(std::move(hero));
    return true;
}

std::shared_ptr<Player> Party::removeMember(std::size_t index)
{
    if (index >= m_members.size()) return nullptr;

    std::shared_ptr<Player> removed = m_members[index];
    m_members.erase(m_members.begin() + static_cast<std::ptrdiff_t>(index));
    return removed;
}

bool Party::addItem(const ItemPtr& item)
{
    if (!item) return false;

    const int limit = item->getMaxCarry();
    if (limit > 0 && countItem(item->getId()) >= static_cast<std::size_t>(limit))
        return false;

    m_inventory.addItem(item);
    return true;
}

std::size_t Party::countItem(const std::string& itemId) const
{
    return m_inventory.countIf([&itemId](const ItemPtr& item) { return item->getId() == itemId; });
}

std::vector<ItemPtr> Party::getItemsByCategory(ItemCategory category) const
{
    std::vector<ItemPtr> result;
    for (const auto& item : m_inventory.getItems())
        if (item->getCategory() == category) result.push_back(item);
    return result;
}

bool Party::equipGear(std::size_t memberIndex, const ItemPtr& item)
{
    Player* hero = getMember(memberIndex);
    auto gear = std::dynamic_pointer_cast<HeldGear>(item);
    if (!hero || !gear) return false;
    if (!m_inventory.removeItem(item)) return false;   // phải đang nằm trong túi

    gear->resetBattleState();
    if (auto previous = hero->modifiers().equipGear(gear))
        m_inventory.addItem(previous);
    return true;
}

bool Party::unequipGear(std::size_t memberIndex)
{
    Player* hero = getMember(memberIndex);
    if (!hero) return false;

    auto gear = hero->modifiers().unequipGear();
    if (!gear) return false;
    m_inventory.addItem(gear);
    return true;
}

Player* Party::getMember(std::size_t index) const
{
    return index < m_members.size() ? m_members[index].get() : nullptr;
}

bool Party::isDefeated() const
{
    return std::none_of(m_members.begin(), m_members.end(),
                        [](const std::shared_ptr<Player>& p) { return p->isAlive(); });
}

void Party::clear()
{
    m_members.clear();
    m_inventory.clear();
}
