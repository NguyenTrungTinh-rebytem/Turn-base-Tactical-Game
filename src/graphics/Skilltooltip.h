#pragma once
// src/graphics/SkillTooltip.h - Tooltip mo ta skill (ve khi tro chuot vao 1 skill)
#include "raylib.h"
#include "data/Element.h"
#include "data/Skill.h"
#include "data/Stats.h"
#include <string>

// Du lieu trung gian de ve tooltip: ve duoc tu PrepSkillInfo (menu truoc tran) lan Skill (trong tran).
struct SkillTipInfo {
    std::string m_name;
    Element     m_element = Element::KIM;
    std::string m_typeText;        // "Sat thuong", "Hoi mau", "Buff", "Debuff"
    int         m_power = 0;       // DAMAGE/DEBUFF: % chi so tan cong; HEAL: luong HP hoi
    int         m_mpCost = 0;
    std::string m_effectText;      // "Chay 2 luot"; rong = khong co hieu ung
    bool        m_isHeal = false;  // true: m_power la luong HP hoi
    bool        m_isMagic = false; // true: sat thuong tinh theo MA, false: theo A
};

class SkillTooltip {
public:
    // Goi CUOI CUNG trong ham ve (sau moi thu khac) de tooltip nam tren cung.
    // caster != nullptr -> hien them sat thuong uoc tinh dua tren chi so nguoi dung skill.
    static void draw(const SkillTipInfo& info, Vector2 mousePos, const Stats* caster = nullptr);
    static void draw(const Skill& skill, Vector2 mousePos, const Stats* caster = nullptr);  // dung trong tran
    static SkillTipInfo fromSkill(const Skill& skill);
};