#pragma once
#include <string>
#include <vector>

// Ngu hanh: Kim(0), Moc(1), Thuy(2), Hoa(3), Tho(4)
enum class Element { KIM = 0, MOC, THUY, HOA, THO };

enum class EffectType { NONE, ATK_BOOST, DEF_BOOST, REGEN, STUN, BURN, SLOW, WEAKNESS };

enum class SkillType { DAMAGE, HEAL, BUFF, DEBUFF };

enum class SkillOwnerType { RACE, CLASS };

// Chi so co ban. Base Stats = Race Stats + Class Stats
struct Stats {
    int m_hp = 0;
    int m_mp = 0;
    int m_spd = 0;
    int m_a = 0;   // Attack (vat ly)
    int m_ma = 0;  // Magic Attack
    int m_d = 0;   // Defend (vat ly)
    int m_md = 0;  // Magic Defend

    Stats operator+(const Stats& other) const {
        return Stats{ m_hp + other.m_hp, m_mp + other.m_mp, m_spd + other.m_spd,
                      m_a + other.m_a, m_ma + other.m_ma, m_d + other.m_d,
                      m_md + other.m_md };
    }
    // So sanh theo Toc do (dung cho priority_queue cua TurnManager)
    bool operator<(const Stats& other) const { return m_spd < other.m_spd; }
};

struct SkillData {
    std::string m_id;
    std::string m_name;
    SkillOwnerType m_ownerType = SkillOwnerType::RACE;
    std::string m_owner;  // ten Race hoac Class so huu skill
    SkillType m_type = SkillType::DAMAGE;
    int m_power = 0;      // % sat thuong / luong hoi
    int m_mpCost = 0;
    EffectType m_effect = EffectType::NONE;
    int m_effectTurns = 0;
};

struct RaceData {
    std::string m_name;
    Stats m_stats;
    std::vector<std::string> m_skillIds;  // dung 2 skill
};

struct ClassData {
    std::string m_name;
    Stats m_stats;
    std::vector<std::string> m_skillIds;  // dung 2 skill
};

struct MonsterData {
    std::string m_id;
    std::string m_name;
    Element m_element = Element::KIM;
    bool m_isBoss = false;
    int m_stage = 1;
    Stats m_stats;
};
