#include "entities/CombatModifiers.h"
#include <algorithm>
#include "data/Skill.h"
#include "entities/Character.h"
#include "entities/Item.h"

bool CombatModifiers::isTimed(BuffType type)
{
    return type == BuffType::DAMAGE_UP || type == BuffType::DAMAGE_REDUCE;
}

// ---------------- Buff ----------------

void CombatModifiers::applyBuff(BuffType type, int value, int turns)
{
    if (type == BuffType::COUNT) return;
    BuffState& s = state(type);
    s.m_value = std::max(0, value);
    s.m_turnsLeft = isTimed(type) ? std::max(0, turns) : 0;
    s.m_fresh = isTimed(type);
}

void CombatModifiers::removeBuff(BuffType type)
{
    if (type == BuffType::COUNT) return;
    state(type) = BuffState{};
}

bool CombatModifiers::hasBuff(BuffType type) const
{
    if (type == BuffType::COUNT) return false;
    const BuffState& s = state(type);
    if (s.m_value <= 0) return false;
    return !isTimed(type) || s.m_turnsLeft > 0;
}

int CombatModifiers::getBuffValue(BuffType type) const
{
    return hasBuff(type) ? state(type).m_value : 0;
}

int CombatModifiers::getBuffTurnsLeft(BuffType type) const
{
    return hasBuff(type) ? state(type).m_turnsLeft : 0;
}

bool CombatModifiers::consumeExtraTurn()
{
    if (!hasBuff(BuffType::EXTRA_TURN)) return false;
    BuffState& s = state(BuffType::EXTRA_TURN);
    if (--s.m_value <= 0) s = BuffState{};
    return true;
}

int CombatModifiers::getCritRate() const
{
    return getBuffValue(BuffType::CRIT_RATE);
}

int CombatModifiers::getCritDamagePercent() const
{
    return BASE_CRIT_DAMAGE_PERCENT + (m_gear ? m_gear->getCritDamageBonus() : 0);
}

// ---------------- Trang bị ----------------

std::shared_ptr<HeldGear> CombatModifiers::equipGear(std::shared_ptr<HeldGear> gear)
{
    std::shared_ptr<HeldGear> previous = std::move(m_gear);
    m_gear = std::move(gear);
    return previous;
}

std::shared_ptr<HeldGear> CombatModifiers::unequipGear()
{
    return equipGear(nullptr);
}

// ---------------- Hook chiến đấu ----------------

int CombatModifiers::applyIncoming(const Character& owner, int damage)
{
    // Iron Shielding: giảm % sát thương nhận vào
    if (hasBuff(BuffType::DAMAGE_REDUCE))
        damage = std::max(1, damage * (100 - getBuffValue(BuffType::DAMAGE_REDUCE)) / 100);

    // Guardian Armor, Survival Sash
    if (m_gear)
        damage = m_gear->modifyIncoming(owner, damage);

    return damage;
}

int CombatModifiers::dealDamage(Character& attacker, Character& target, const Skill& skill, int baseDamage)
{
    int damage = baseDamage;

    // 1) Power Serum: + % sát thương
    if (hasBuff(BuffType::DAMAGE_UP))
        damage = damage * (100 + getBuffValue(BuffType::DAMAGE_UP)) / 100;

    // 2) Trang bị: Berserk Core, Executioner's Ring
    if (m_gear)
        damage = m_gear->modifyOutgoing(attacker, target, damage);

    // 3) Chí mạng (Precision Lens): đòn đánh kế tiếp chắc chắn chí mạng, rồi hết hiệu lực
    if (getCritRate() >= CRIT_GUARANTEED_RATE) {
        damage = damage * getCritDamagePercent() / 100;
        removeBuff(BuffType::CRIT_RATE);
    }

    const int hpBefore = target.getCurrentHp();
    target.takeDamage(damage, skill.m_element, skill.m_isMagic);
    const int dealt = hpBefore - target.getCurrentHp();

    if (dealt > 0) {
        // 4) Vampiric Charm / giá HP của Berserk Core
        if (m_gear)
            m_gear->afterDealing(attacker, dealt);

        // 5) Counter Helmet: chỉ phản lại đòn cận chiến (vật lý), trừ thẳng HP kẻ tấn công
        const auto& targetGear = target.modifiers().getGear();
        if (!skill.m_isMagic && targetGear) {
            const int reflected = targetGear->reflectDamage(dealt);
            if (reflected > 0 && attacker.isAlive())
                attacker.takeDamage(reflected);
        }
    }
    return dealt;
}

void CombatModifiers::onTurnEnd(Character& owner)
{
    if (!owner.isAlive()) return;

    for (BuffType type : { BuffType::DAMAGE_UP, BuffType::DAMAGE_REDUCE })
    {
        if (!hasBuff(type)) continue;
        BuffState& s = state(type);
        if (s.m_fresh) {              // lượt dùng thuốc không tính vào thời hạn
            s.m_fresh = false;
            continue;
        }
        if (--s.m_turnsLeft <= 0) s = BuffState{};
    }

    if (m_gear)
        m_gear->onTurnEnd(owner);
}

void CombatModifiers::reset()
{
    for (auto& s : m_buffs) s = BuffState{};
    if (m_gear) m_gear->resetBattleState();
}
