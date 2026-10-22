#pragma once
#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include "data/Element.h"
#include "data/Stats.h"

struct Skill;  // Khai báo trước (Forward declaration)

// Lớp cha trừu tượng cho Player, Enemy, Boss.
class Character
{
public:
    Character(const std::string& name, const Stats& stats, Element element);

    // Bắt buộc virtual để xóa đối tượng con qua con trỏ lớp cha an toàn.
    virtual ~Character() = default;

    // Nhận sát thương tính theo Ma trận Khắc hệ Ngũ hành & Phòng thủ
    virtual void takeDamage(int amount, Element atkElement, bool isMagic);

    // Nạp chồng takeDamage nhận 1 tham số (cho BURN / Sát thương chuẩn không qua thủ)
    virtual void takeDamage(int amount) {
        m_currentHp = std::max(0, m_currentHp - amount);
    }

    // Hàm hồi máu cho REGEN / Skill Heal
    virtual void heal(int amount) {
        m_currentHp = std::min(m_stats.m_hp, m_currentHp + amount);
    }

    // Quản lý trạng thái Stun (Cho TurnManager & StatusEffect)
    bool isStunned() const { return m_isStunned; }
    void setStunned(bool stunned) { m_isStunned = stunned; }

    // Quản lý thay đổi Speed tạm thời (Cho hiệu ứng SLOW)
    void setSpeed(int spd) { m_stats.m_spd = spd; }

    // Dùng skill lên mục tiêu. Thuần ảo: mỗi lớp con tự định nghĩa cách xử lý.
    virtual void executeSkill(Skill* skill, Character* target) = 0;

    // Getters
    const std::string& getName() const      { return m_name; }
    const Stats&       getStats() const     { return m_stats; }
    Element            getElement() const   { return m_element; }
    int                getCurrentHp() const { return m_currentHp; }
    int                getCurrentMp() const { return m_currentMp; }
    int                getSpeed() const     { return m_stats.m_spd; }
    bool               isAlive() const      { return m_currentHp > 0; }

protected:
    // Kiểm tra skill/mục tiêu hợp lệ và cả hai bên còn sống.
    bool canAct(const Skill* skill, const Character* target) const;

    // Sát thương gốc của skill = chỉ số tấn công (vật lý/phép) + sức mạnh skill.
    int  calcSkillDamage(const Skill* skill) const;

    std::string m_name;
    Stats       m_stats;       // Chỉ số tối đa
    Element     m_element;
    int         m_currentHp;   // Máu hiện tại
    int         m_currentMp;   // Mana hiện tại
    bool        m_isStunned = false; // Trạng thái bị choáng/bỏ lượt
};

// Smart pointer quản lý vòng đời nhân vật
using CharacterPtr = std::unique_ptr<Character>;

// Hàm khởi tạo FactoryHelper
template <typename T, typename... Args>
CharacterPtr makeCharacter(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
