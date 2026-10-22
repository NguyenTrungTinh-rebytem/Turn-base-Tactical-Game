#include "entities/Character.h"
#include <algorithm>
#include "data/Skill.h"

namespace  // Chỉ dùng trong file này
{
    constexpr float STRONG_MULTIPLIER  = 1.5f;   // Hệ khắc hệ đối thủ
    constexpr float WEAK_MULTIPLIER    = 0.75f;  // Bị hệ đối thủ khắc
    constexpr float NEUTRAL_MULTIPLIER = 1.0f;   // Không khắc nhau
    constexpr int   MIN_DAMAGE         = 1;      // Đòn đánh luôn gây ít nhất 1 sát thương

    // Vòng khắc chế: Nước > Lửa > Gió > Đất > Nước. NONE không khắc ai.
    bool beats(Element attacker, Element defender)
    {
        return (attacker == Element::WATER && defender == Element::FIRE)  ||
               (attacker == Element::FIRE  && defender == Element::WIND)  ||
               (attacker == Element::WIND  && defender == Element::EARTH) ||
               (attacker == Element::EARTH && defender == Element::WATER);
    }

    // Trả về hệ số sát thương theo quan hệ khắc chế giữa hai hệ.
    float getElementMultiplier(Element attacker, Element defender)
    {
        if (beats(attacker, defender)) return STRONG_MULTIPLIER;
        if (beats(defender, attacker)) return WEAK_MULTIPLIER;
        return NEUTRAL_MULTIPLIER;
    }
}

// HP/MP hiện tại khởi tạo bằng giá trị tối đa trong stats.
// Thứ tự trong initializer list khớp thứ tự khai báo trong header.
Character::Character(const std::string& name, const Stats& stats, Element element)
    : m_name(name), m_stats(stats), m_element(element),
      m_currentHp(stats.m_hp), m_currentMp(stats.m_mp)
{
}

// Sát thương cuối = amount * hệ số hệ - phòng thủ, tối thiểu MIN_DAMAGE.
void Character::takeDamage(int amount, Element atkElement, bool isMagic)
{
    if (!isAlive()) return;  // Đã chết thì không nhận thêm sát thương

    const int defense = isMagic ? m_stats.m_md : m_stats.m_d;  // Phép dùng MD, vật lý dùng D
    const float multiplier = getElementMultiplier(atkElement, m_element);
    const int finalDamage = std::max(MIN_DAMAGE, static_cast<int>(static_cast<float>(amount) * multiplier) - defense);

    m_currentHp = std::max(0, m_currentHp - finalDamage);  // HP không xuống âm
}

bool Character::canAct(const Skill* skill, const Character* target) const
{
    return skill && target && isAlive() && target->isAlive();
}

int Character::calcSkillDamage(const Skill* skill) const
{
    return (skill->m_isMagic ? m_stats.m_ma : m_stats.m_a) + skill->m_power;
}