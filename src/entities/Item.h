#pragma once
#include <memory>
#include <string>
#include "entities/Character.h"
#include "entities/ItemData.h"

// Phân loại theo cách dùng (engine/UI dùng để tìm đúng loại vật phẩm)
enum class ItemKind { POTION, MANA_POTION, REVIVE, BOOSTER, HELD_GEAR };

// Lớp cơ sở trừu tượng cho mọi vật phẩm. Mọi thông số lấy từ ItemData (data/items.csv).
class Item
{
public:
    explicit Item(const ItemData& data);
    virtual ~Item() = default;

    virtual ItemKind getKind() const = 0;

    // Có dùng được lên target lúc này không
    virtual bool canUse(const Character& target) const = 0;

    // Dùng vật phẩm. Trả về lượng HP/MP đã hồi (Booster: 1 nếu áp dụng được, 0 nếu không).
    virtual int use(Character& target) = 0;

    // Hoàn tác use(): phục vụ Undo của Command Pattern.
    virtual void revert(Character& target, int amount) = 0;

    // Getter cho màn hình khác
    const ItemData&    getData() const        { return m_data; }
    const std::string& getId() const          { return m_data.m_id; }
    const std::string& getName() const        { return m_data.m_name; }
    const std::string& getNameVi() const      { return m_data.m_nameVi; }
    const std::string& getDescription() const { return m_description; }
    ItemCategory       getCategory() const    { return m_data.m_category; }
    ItemEffect         getEffect() const      { return m_data.m_effect; }
    int                getValue() const       { return m_data.m_value; }
    int                getDuration() const    { return m_data.m_duration; }
    int                getMaxCarry() const    { return m_data.m_maxCarry; }

protected:
    ItemData    m_data;
    std::string m_description;
};

using ItemPtr = std::shared_ptr<Item>;

// ---------------- CONSUMABLE ----------------

// Bình máu: hồi value% HP tối đa
class Potion : public Item
{
public:
    explicit Potion(const ItemData& data) : Item(data) {}
    ItemKind getKind() const override { return ItemKind::POTION; }
    bool canUse(const Character& target) const override;
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;
};

// Bình năng lượng: hồi value% MP tối đa
class ManaPotion : public Item
{
public:
    explicit ManaPotion(const ItemData& data) : Item(data) {}
    ItemKind getKind() const override { return ItemKind::MANA_POTION; }
    bool canUse(const Character& target) const override;
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;
};

// Lông Phượng Hoàng: hồi sinh đồng đội đã gục với value% HP tối đa
class ReviveItem : public Item
{
public:
    explicit ReviveItem(const ItemData& data) : Item(data) {}
    ItemKind getKind() const override { return ItemKind::REVIVE; }
    bool canUse(const Character& target) const override;   // chỉ dùng được lên người đã gục
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;
};

// ---------------- BOOSTER (dùng trong trận, tốn 1 lượt) ----------------

class BoosterItem : public Item
{
public:
    explicit BoosterItem(const ItemData& data) : Item(data) {}
    ItemKind getKind() const override { return ItemKind::BOOSTER; }
    bool canUse(const Character& target) const override;   // không dùng chồng cùng loại buff
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;

    BuffType getBuffType() const;
};

// ---------------- HELD GEAR (bị động, trang bị trước trận) ----------------

class HeldGear : public Item
{
public:
    explicit HeldGear(const ItemData& data) : Item(data) {}
    ItemKind getKind() const override { return ItemKind::HELD_GEAR; }
    bool canUse(const Character&) const override { return false; }   // trang bị, không "dùng"
    int  use(Character&) override { return 0; }
    void revert(Character&, int) override {}

    // ---- Hook chiến đấu (Character gọi) ----
    // Sát thương gây ra: Berserk Core, Executioner's Ring
    int  modifyOutgoing(const Character& owner, const Character& target, int damage) const;
    // Sát thương nhận vào: Guardian Armor, Survival Sash
    int  modifyIncoming(const Character& owner, int damage);
    // Sau khi gây sát thương: Vampiric Charm, giá HP của Berserk Core
    void afterDealing(Character& owner, int dealt);
    // Cuối lượt: Leftover Band
    void onTurnEnd(Character& owner);
    // Sát thương phản lại khi bị đánh cận chiến: Counter Helmet
    int  reflectDamage(int received) const;
    // % sát thương chí mạng cộng thêm: Heavy Strike Stone
    int  getCritDamageBonus() const;
    // Nạp lại hiệu ứng "1 lần/trận" (Survival Sash)
    void resetBattleState() { m_surviveUsed = false; }

    bool isSurviveUsed() const { return m_surviveUsed; }

private:
    bool m_surviveUsed = false;
};
