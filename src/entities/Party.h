#pragma once
#include <cstddef>
#include <memory>
#include <string>
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

    // ----- Túi đồ dùng chung -----
    // Thêm vật phẩm, tôn trọng giới hạn mang theo (maxCarry) của từng vật phẩm.
    // Trả về false nếu vật phẩm null hoặc đã đạt giới hạn.
    bool addItem(const ItemPtr& item);

    // Số vật phẩm cùng id đang có trong túi
    std::size_t countItem(const std::string& itemId) const;

    // Danh sách vật phẩm theo nhóm (cho màn hình Túi đồ)
    std::vector<ItemPtr> getItemsByCategory(ItemCategory category) const;

    // ----- Trang bị -----
    // Lấy Held Gear từ túi đồ trang bị cho hero; món cũ (nếu có) được cất lại vào túi.
    // Trả về false nếu hero không tồn tại, vật phẩm không phải Held Gear hoặc không có trong túi.
    bool equipGear(std::size_t memberIndex, const ItemPtr& item);
    // Tháo trang bị của hero về túi đồ. Trả về false nếu hero không có trang bị.
    bool unequipGear(std::size_t memberIndex);

    Inventory<ItemPtr>&       getInventory()       { return m_inventory; }
    const Inventory<ItemPtr>& getInventory() const { return m_inventory; }

    // Xóa toàn bộ thành viên và túi đồ
    void clear();

private:
    std::vector<std::shared_ptr<Player>> m_members;
    Inventory<ItemPtr> m_inventory;
};
