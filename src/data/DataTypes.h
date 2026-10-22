#pragma once
#include <string>
#include <vector>

// Enum Ngũ hành chuẩn PROMPT
enum class Element { KIM = 0, MOC, THUY, HOA, THO };

// Ma trận khắc hệ 5x5 chuẩn
constexpr float ELEMENT_MATRIX[5][5] = {
    { 1.0f, 1.5f, 1.5f, 0.5f, 0.5f }, // KIM
    { 0.5f, 1.0f, 1.5f, 0.5f, 1.5f }, // MOC
    { 0.5f, 0.5f, 1.0f, 1.5f, 1.5f }, // THUY
    { 1.5f, 1.5f, 0.5f, 1.0f, 0.5f }, // HOA
    { 1.5f, 0.5f, 0.5f, 1.5f, 1.0f }  // THO
};

enum class EffectType { NONE, ATK_BOOST, DEF_BOOST, REGEN, STUN, BURN, SLOW, WEAKNESS };
enum class SkillType { DAMAGE, HEAL, BUFF, DEBUFF };
enum class SkillOwnerType { RACE, CLASS };

// Struct Stats thống nhất
struct Stats {
    int m_hp = 0;
    int m_mp = 0;
    int m_spd = 0;
    int m_a = 0;   // Physical Attack
    int m_ma = 0;  // Magic Attack
    int m_d = 0;   // Physical Defend
    int m_md = 0;  // Magic Defend

    Stats operator+(const Stats& other) const {
        return Stats{ m_hp + other.m_hp, m_mp + other.m_mp, m_spd + other.m_spd,
                      m_a + other.m_a, m_ma + other.m_ma, m_d + other.m_d, m_md + other.m_md };
    }
    bool operator<(const Stats& other) const { return m_spd < other.m_spd; }
};
