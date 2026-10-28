#include "entities/Item.h"
#include <algorithm>
#include <string>

namespace {

std::string pct(int v) { return std::to_string(v) + "%"; }

std::string describe(const ItemData& d)
{
    switch (d.m_effect)
    {
    case ItemEffect::HEAL_HP_PCT:         return "Hoi ngay " + pct(d.m_value) + " HP toi da";
    case ItemEffect::RESTORE_MP_PCT:      return "Phuc hoi " + pct(d.m_value) + " MP toi da";
    case ItemEffect::REVIVE_PCT:          return "Hoi sinh 1 dong doi voi " + pct(d.m_value) + " HP";
    case ItemEffect::BOOST_DAMAGE_UP:     return "Tang " + pct(d.m_value) + " sat thuong trong " + std::to_string(d.m_duration) + " luot";
    case ItemEffect::BOOST_DAMAGE_REDUCE: return "Giam " + pct(d.m_value) + " sat thuong nhan trong " + std::to_string(d.m_duration) + " luot";
    case ItemEffect::BOOST_EXTRA_TURN:    return "Them " + std::to_string(d.m_value) + " luot hanh dong ngay lap tuc";
    case ItemEffect::BOOST_CRIT_RATE:     return "Tang " + pct(d.m_value) + " ty le chi mang cho don ke tiep";
    case ItemEffect::HELD_REGEN:          return "Hoi " + pct(d.m_value) + " HP toi da cuoi moi luot";
    case ItemEffect::HELD_LIFESTEAL:      return "Hoi HP bang " + pct(d.m_value) + " sat thuong gay ra";
    case ItemEffect::HELD_SURVIVE:        return "Dang day HP ma nhan don chi mang: giu lai " + std::to_string(d.m_value) + " HP (1 lan/tran)";
    case ItemEffect::HELD_DAMAGE_REDUCE:  return "Giam " + pct(d.m_value) + " moi sat thuong nhan vao";
    case ItemEffect::HELD_BERSERK:        return "Tang " + pct(d.m_value) + " sat thuong, mat " + pct(d.m_value2) + " HP toi da sau khi danh";
    case ItemEffect::HELD_EXECUTE:        return "Them " + pct(d.m_value) + " sat thuong neu muc tieu duoi " + pct(d.m_value2) + " HP";
    case ItemEffect::HELD_CRIT_DAMAGE:    return "Tang " + pct(d.m_value) + " sat thuong chi mang";
    case ItemEffect::HELD_COUNTER:        return "Phan lai " + pct(d.m_value) + " sat thuong khi bi danh can chien";
    }
    return "";
}

} // namespace

Item::Item(const ItemData& data)
    : m_data(data),
      m_description(data.m_descriptionVi.empty() ? describe(data) : data.m_descriptionVi)
{
}

// ---------------- Potion (hồi HP) ----------------

bool Potion::canUse(const Character& target) const
{
    return target.isAlive() && target.getCurrentHp() < target.getMaxHp();
}

int Potion::use(Character& target)
{
    if (!canUse(target)) return 0;

    const int amount = std::max(1, target.getMaxHp() * m_data.m_value / 100);
    const int before = target.getCurrentHp();
    target.heal(amount);
    return target.getCurrentHp() - before;
}

void Potion::revert(Character& target, int amount)
{
    if (amount > 0) target.takeDamage(amount);   // trừ thẳng HP, bỏ qua giáp/ngũ hành
}

// ---------------- ManaPotion (hồi MP) ----------------

bool ManaPotion::canUse(const Character& target) const
{
    return target.isAlive() && target.getCurrentMp() < target.getMaxMp();
}

int ManaPotion::use(Character& target)
{
    if (!canUse(target)) return 0;
    return target.restoreMp(std::max(1, target.getMaxMp() * m_data.m_value / 100));
}

void ManaPotion::revert(Character& target, int amount)
{
    if (amount > 0) target.consumeMp(amount);
}

// ---------------- ReviveItem (hồi sinh) ----------------

bool ReviveItem::canUse(const Character& target) const
{
    return !target.isAlive();
}

int ReviveItem::use(Character& target)
{
    if (!canUse(target)) return 0;

    target.heal(std::max(1, target.getMaxHp() * m_data.m_value / 100));
    return target.getCurrentHp();
}

void ReviveItem::revert(Character& target, int amount)
{
    if (amount > 0) target.takeDamage(amount);   // về lại trạng thái gục
}

// ---------------- BoosterItem ----------------

BuffType BoosterItem::getBuffType() const
{
    switch (m_data.m_effect)
    {
    case ItemEffect::BOOST_DAMAGE_UP:     return BuffType::DAMAGE_UP;
    case ItemEffect::BOOST_DAMAGE_REDUCE: return BuffType::DAMAGE_REDUCE;
    case ItemEffect::BOOST_EXTRA_TURN:    return BuffType::EXTRA_TURN;
    default:                              return BuffType::CRIT_RATE;
    }
}

bool BoosterItem::canUse(const Character& target) const
{
    // Không dùng chồng cùng loại buff: nhờ vậy Undo (xóa buff) luôn trả về đúng trạng thái trước đó
    return target.isAlive() && !target.modifiers().hasBuff(getBuffType());
}

int BoosterItem::use(Character& target)
{
    if (!canUse(target)) return 0;
    target.modifiers().applyBuff(getBuffType(), m_data.m_value, m_data.m_duration);
    return 1;
}

void BoosterItem::revert(Character& target, int amount)
{
    if (amount > 0) target.modifiers().removeBuff(getBuffType());
}

// ---------------- HeldGear ----------------

int HeldGear::modifyOutgoing(const Character& /*owner*/, const Character& target, int damage) const
{
    switch (m_data.m_effect)
    {
    case ItemEffect::HELD_BERSERK:
        return damage * (100 + m_data.m_value) / 100;
    case ItemEffect::HELD_EXECUTE:
        // HP mục tiêu thấp hơn ngưỡng (ví dụ < 30% HP tối đa)
        if (target.getCurrentHp() * 100 < target.getMaxHp() * m_data.m_value2)
            return damage * (100 + m_data.m_value) / 100;
        return damage;
    default:
        return damage;
    }
}

int HeldGear::modifyIncoming(const Character& owner, int damage)
{
    switch (m_data.m_effect)
    {
    case ItemEffect::HELD_DAMAGE_REDUCE:
        return std::max(1, damage * (100 - m_data.m_value) / 100);

    case ItemEffect::HELD_SURVIVE:
        // Đang đầy HP mà nhận đòn dứt điểm -> giữ lại đúng m_value HP, 1 lần/trận
        if (!m_surviveUsed &&
            owner.getCurrentHp() == owner.getMaxHp() &&
            damage >= owner.getCurrentHp())
        {
            m_surviveUsed = true;
            return std::max(0, owner.getCurrentHp() - m_data.m_value);
        }
        return damage;

    default:
        return damage;
    }
}

void HeldGear::afterDealing(Character& owner, int dealt)
{
    switch (m_data.m_effect)
    {
    case ItemEffect::HELD_LIFESTEAL:
        owner.heal(dealt * m_data.m_value / 100);
        break;

    case ItemEffect::HELD_BERSERK:
    {
        // Mất % HP tối đa sau khi đánh, nhưng không tự giết chính mình (chừa lại 1 HP)
        const int cost = std::max(1, owner.getMaxHp() * m_data.m_value2 / 100);
        const int payable = std::min(cost, owner.getCurrentHp() - 1);
        if (payable > 0) owner.takeDamage(payable);
        break;
    }
    default:
        break;
    }
}

void HeldGear::onTurnEnd(Character& owner)
{
    if (m_data.m_effect == ItemEffect::HELD_REGEN && owner.isAlive())
        owner.heal(std::max(1, owner.getMaxHp() * m_data.m_value / 100));
}

int HeldGear::reflectDamage(int received) const
{
    return m_data.m_effect == ItemEffect::HELD_COUNTER ? received * m_data.m_value / 100 : 0;
}

int HeldGear::getCritDamageBonus() const
{
    return m_data.m_effect == ItemEffect::HELD_CRIT_DAMAGE ? m_data.m_value : 0;
}
