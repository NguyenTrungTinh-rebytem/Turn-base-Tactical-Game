#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "entities/Player.h"
#include "entities/Inventory.h"
#include "entities/Item.h"

// Đội hình Hero (tối đa 3) + túi đồ dùng chung của cả đội.
// Dùng shared_ptr để Engine (danh sách combatants) và Party cùng quan sát một Hero an toàn.
class Party
{
public:
    static constexpr std::size_t MAX_SIZE = 3;

    // Thêm Hero. Trả về false nếu đội đã đầy (3), hero null, hoặc hero đã có trong đội.
    bool addMember(std::shared_ptr<Player> hero);

    // Rút Hero khỏi đội theo vị trí. Trả về nullptr nếu index sai.
    std::shared_ptr<Player> removeMember(std::size_t index);

    // nullptr nếu index ngoài phạm vi
    Player* getMember(std::size_t index) const;
    Player* getLeader() const { return getMember(0); }

    const std::vector<std::shared_ptr<Player>>& getMembers() const { return m_members; }

    std::size_t size() const    { return m_members.size(); }
    bool isEmpty() const        { return m_members.empty(); }
    bool isFull() const         { return m_members.size() >= MAX_SIZE; }

    // Cả đội gục (hoặc đội rỗng)
    bool isDefeated() const;

    Inventory<ItemPtr>&       getInventory()       { return m_inventory; }
    const Inventory<ItemPtr>& getInventory() const { return m_inventory; }

    // Xóa toàn bộ thành viên và túi đồ
    void clear();

private:
    std::vector<std::shared_ptr<Player>> m_members;
    Inventory<ItemPtr> m_inventory;
};
