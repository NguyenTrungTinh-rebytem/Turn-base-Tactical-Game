#pragma once
#include <memory>
#include <string>
#include <utility>
#include "data/Element.h"
#include "data/Stats.h"

struct Skill;  // Khai báo trước: header chỉ dùng con trỏ nên không cần include đầy đủ

// Lớp cha trừu tượng cho Player, Enemy, Boss.
class Character
{
public:
    Character(const std::string& name, const Stats& stats, Element element);

    // Bắt buộc virtual để xóa đối tượng con qua con trỏ lớp cha vẫn an toàn.
    virtual ~Character() = default;

    // Nhận sát thương. Lớp con có thể ghi đè (VD: Boss giảm sát thương nhận vào).
    virtual void takeDamage(int amount, Element atkElement, bool isMagic);

    // Dùng skill lên mục tiêu. Thuần ảo: mỗi lớp con tự định nghĩa cách xử lý.
    virtual void executeSkill(Skill* skill, Character* target) = 0;

    const std::string& getName() const      { return m_name; }
    const Stats&       getStats() const     { return m_stats; }
    Element            getElement() const   { return m_element; }
    int                getCurrentHp() const { return m_currentHp; }
    int                getCurrentMp() const { return m_currentMp; }
    int                getSpeed() const     { return m_stats.m_spd; }  // Cho TurnManager
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
};

// unique_ptr: một chủ sở hữu duy nhất, tự giải phóng bộ nhớ, tránh rò rỉ.
using CharacterPtr = std::unique_ptr<Character>;

// Hàm tạo nhân vật: makeCharacter<Boss>("Rồng", stats, Element::FIRE).
// std::forward chuyển nguyên tham số vào constructor của T.
template <typename T, typename... Args>
CharacterPtr makeCharacter(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}