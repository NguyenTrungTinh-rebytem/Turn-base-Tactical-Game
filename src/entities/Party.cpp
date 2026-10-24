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
