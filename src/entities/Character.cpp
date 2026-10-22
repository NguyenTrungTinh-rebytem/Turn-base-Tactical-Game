#include "Character.h"
#include "data/Skill.h"
#include "data/DataTypes.h"
#include <algorithm>

Character::Character(const std::string& name, const Stats& stats, Element element)
    : m_name(name), m_stats(stats), m_element(element), 
      m_currentHp(stats.m_hp), m_currentMp(stats.m_mp) {}

void Character::takeDamage(int amount, Element atkElement, bool isMagic) {
    // Lấy hệ số khắc hệ từ ma trận Ngũ Hành
    float elementMult = ELEMENT_MATRIX[static_cast<int>(atkElement)][static_cast<int>(m_element)];
    int def = isMagic ? m_stats.m_md : m_stats.m_d;
    
    int finalDamage = static_cast<int>(amount * elementMult) - def;
    if (finalDamage < 1) finalDamage = 1;

    m_currentHp -= finalDamage;
    if (m_currentHp < 0) m_currentHp = 0;
}
