#pragma once
#include <memory>
#include <string>
#include "entities/Character.h"

enum class ItemKind { POTION, MANA_POTION };

// Lớp cơ sở trừu tượng cho mọi vật phẩm (đa hình qua use()/revert()).
class Item
{
public:
    Item(const std::string& name, const std::string& description);
    virtual ~Item() = default;

    virtual ItemKind getKind() const = 0;

    // Có dùng được lên target lúc này không (ví dụ HP/MP đã đầy thì không)
    virtual bool canUse(const Character& target) const = 0;

    // Dùng vật phẩm. Trả về lượng HP/MP thực sự đã hồi (0 nếu không có tác dụng).
    virtual int use(Character& target) = 0;

    // Hoàn tác use(): trả lại đúng 'amount' đã hồi (phục vụ Undo của Command Pattern).
    virtual void revert(Character& target, int amount) = 0;

    const std::string& getName() const        { return m_name; }
    const std::string& getDescription() const { return m_description; }

protected:
    std::string m_name;
    std::string m_description;
};

using ItemPtr = std::shared_ptr<Item>;

// Bình máu: hồi HP
class Potion : public Item
{
public:
    explicit Potion(int healAmount = 40);

    ItemKind getKind() const override { return ItemKind::POTION; }
    bool canUse(const Character& target) const override;
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;

    int getHealAmount() const { return m_healAmount; }

private:
    int m_healAmount;
};

// Bình mana: hồi MP
class ManaPotion : public Item
{
public:
    explicit ManaPotion(int restoreAmount = 30);

    ItemKind getKind() const override { return ItemKind::MANA_POTION; }
    bool canUse(const Character& target) const override;
    int  use(Character& target) override;
    void revert(Character& target, int amount) override;

    int getRestoreAmount() const { return m_restoreAmount; }

private:
    int m_restoreAmount;
};
