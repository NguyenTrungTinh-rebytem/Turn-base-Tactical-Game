#pragma once
#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include "data/DataTypes.h"

struct Skill;

class Character
{
public:
    Character(const std::string& name, const Stats& stats, Element element);
    virtual ~Character() = default;

    // Nhận sát thương tính theo Ma trận Ngũ Hành và Phòng thủ
    virtual void takeDamage(int amount, Element atkElement, bool isMagic);

    // Nhận sát thương trực tiếp (dùng cho hiệu ứng BURN/Mất HP)
    virtual void takeDamage(int amount) {
        m_currentHp = std::max(0, m_currentHp - amount);
    }

    // Hồi máu (dùng cho hiệu ứng REGEN/Kỹ năng hồi HP)
    virtual void heal(int amount) {
        m_currentHp = std::min(m_stats.m_hp, m_currentHp + amount);
    }

    // Trạng thái Choáng (dùng cho hiệu ứng STUN)
    bool isStunned() const { return m_isStunned; }
    void setStunned(bool stunned) { m_isStunned = stunned; }

    // Thay đổi Tốc độ (dùng cho hiệu ứng SLOW)
    void setSpeed(int spd) { m_stats.m_spd = spd; }

    // Dùng kỹ năng
    virtual void executeSkill(Skill* skill, Character* target) = 0;

    // Getters
    const std::string& getName() const      { return m_name; }
    const Stats&       getStats() const     { return m_stats; }
    Element            getElement() const   { return m_element; }
    int                getCurrentHp() const { return m_currentHp; }
    int                getCurrentMp() const { return m_currentMp; }
    int                getSpeed() const     { return m_stats.m_spd; }
    bool               isAlive() const      { return m_currentHp > 0; }

    // Nạp chồng toán tử < so sánh Tốc độ để hỗ trợ Priority Queue cho TurnManager
    bool operator<(const Character& other) const {
        return m_stats.m_spd < other.m_stats.m_spd;
    }

protected:
    bool canAct(const Skill* skill, const Character* target) const;
    int  calcSkillDamage(const Skill* skill) const;

    std::string m_name;
    Stats       m_stats;
    Element     m_element;
    int         m_currentHp;
    int         m_currentMp;
    bool        m_isStunned = false;
};

using CharacterPtr = std::unique_ptr<Character>;

template <typename T, typename... Args>
CharacterPtr makeCharacter(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
