// src/graphics/SkillTooltip.cpp
#include "SkillTooltip.h"
#include <algorithm>
#include <vector>

namespace {
constexpr int PAD = 10;
constexpr int TITLE_SIZE = 18;
constexpr int TEXT_SIZE = 14;
constexpr int LINE_H = 20;
constexpr int MIN_W = 250;
constexpr int MARGIN = 8;

struct TipLine {
    std::string m_text;
    Color m_color;
};

const char* elementText(Element e) {
    switch (e) {
        case Element::KIM:  return "Kim";
        case Element::MOC:  return "Moc";
        case Element::THUY: return "Thuy";
        case Element::HOA:  return "Hoa";
        case Element::THO:  return "Tho";
    }
    return "?";
}

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return Color{ 235, 220, 140, 255 };
        case Element::MOC:  return Color{ 120, 220, 120, 255 };
        case Element::THUY: return Color{ 100, 170, 255, 255 };
        case Element::HOA:  return Color{ 255, 120, 90, 255 };
        case Element::THO:  return Color{ 210, 160, 100, 255 };
    }
    return WHITE;
}

const char* typeText(SkillType t) {
    switch (t) {
        case SkillType::DAMAGE: return "Sat thuong";
        case SkillType::HEAL:   return "Hoi mau";
        case SkillType::BUFF:   return "Buff";
        case SkillType::DEBUFF: return "Debuff";
    }
    return "?";
}

const char* effectName(EffectType e) {
    switch (e) {
        case EffectType::ATK_BOOST: return "Tang tan cong";
        case EffectType::DEF_BOOST: return "Tang phong thu";
        case EffectType::REGEN:     return "Hoi mau moi luot";
        case EffectType::STUN:      return "Choang (mat luot)";
        case EffectType::BURN:      return "Chay (mat mau moi luot)";
        case EffectType::SLOW:      return "Lam cham";
        case EffectType::WEAKNESS:  return "Suy yeu";
        case EffectType::NONE:      return "";
    }
    return "";
}

// Liet ke cac he ma skill nay manh hon (strong = true) hoac yeu hon (false), lay tu ELEMENT_MATRIX.
std::string elementList(Element atk, bool strong) {
    const int row = static_cast<int>(atk);
    std::string out;
    for (int col = 0; col < 5; ++col) {
        const float f = ELEMENT_MATRIX[row][col];
        if ((strong && f > 1.0f) || (!strong && f < 1.0f)) {
            if (!out.empty()) out += ", ";
            out += elementText(static_cast<Element>(col));
        }
    }
    return out.empty() ? "-" : out;
}

std::vector<TipLine> buildLines(const SkillTipInfo& s, const Stats* caster) {
    const Color white{ 235, 235, 245, 255 };
    const Color gray{ 170, 170, 190, 255 };
    const Color gold{ 255, 215, 90, 255 };
    const Color blue{ 120, 190, 255, 255 };
    const Color green{ 120, 230, 140, 255 };
    const Color orange{ 255, 170, 90, 255 };
    const Color red{ 255, 130, 130, 255 };

    const bool dealsDamage = !s.m_isHeal && s.m_power > 0;

    std::vector<TipLine> lines;
    std::string header = s.m_typeText + " - He " + elementText(s.m_element);
    if (dealsDamage) header += s.m_isMagic ? " (Phep)" : " (Vat ly)";
    lines.push_back({ header, gray });
    lines.push_back({ TextFormat("Ton MP: %d", s.m_mpCost), blue });

    if (dealsDamage) {
        lines.push_back({ TextFormat("Sat thuong: %d%% %s", s.m_power,
                                     s.m_isMagic ? "Phep cong (MA)" : "Tan cong (A)"), white });
        if (caster) {
            const int atk = s.m_isMagic ? caster->m_ma : caster->m_a;
            lines.push_back({ TextFormat("Uoc tinh: ~%d (chua tinh khac he)", atk * s.m_power / 100), gold });
        }
    } else if (s.m_isHeal) {
        lines.push_back({ TextFormat("Hoi phuc: %d HP", s.m_power), green });
    }

    if (!s.m_effectText.empty()) lines.push_back({ "Hieu ung: " + s.m_effectText, orange });

    if (dealsDamage) {
        lines.push_back({ "Manh vs: " + elementList(s.m_element, true), green });
        lines.push_back({ "Yeu vs: " + elementList(s.m_element, false), red });
    }
    return lines;
}
} // namespace

SkillTipInfo SkillTooltip::fromSkill(const Skill& skill) {
    SkillTipInfo t;
    t.m_name = skill.m_name;
    t.m_element = skill.m_element;
    t.m_typeText = typeText(skill.m_type);
    t.m_power = skill.m_power;
    t.m_mpCost = skill.m_mpCost;
    t.m_isHeal = (skill.m_type == SkillType::HEAL);
    t.m_isMagic = skill.m_isMagic;
    if (skill.m_effect != EffectType::NONE && skill.m_effectTurns > 0)
        t.m_effectText = std::string(effectName(skill.m_effect)) + " (" + std::to_string(skill.m_effectTurns) + " luot)";
    return t;
}

void SkillTooltip::draw(const Skill& skill, Vector2 mousePos, const Stats* caster) {
    draw(fromSkill(skill), mousePos, caster);
}

void SkillTooltip::draw(const SkillTipInfo& info, Vector2 mousePos, const Stats* caster) {
    const std::vector<TipLine> lines = buildLines(info, caster);
    const Color accent = elementColor(info.m_element);

    int w = std::max(MIN_W, MeasureText(info.m_name.c_str(), TITLE_SIZE) + 2 * PAD);
    for (const TipLine& l : lines) w = std::max(w, MeasureText(l.m_text.c_str(), TEXT_SIZE) + 2 * PAD);
    const int titleH = TITLE_SIZE + 8;
    const int h = PAD * 2 + titleH + static_cast<int>(lines.size()) * LINE_H;

    // Dat canh con tro; tran man hinh thi lat sang trai / day len.
    const int sw = GetScreenWidth(), sh = GetScreenHeight();
    int x = static_cast<int>(mousePos.x) + 18;
    int y = static_cast<int>(mousePos.y) + 20;
    if (x + w > sw - MARGIN) x = static_cast<int>(mousePos.x) - w - 18;
    if (y + h > sh - MARGIN) y = sh - h - MARGIN;
    x = std::max(MARGIN, x);
    y = std::max(MARGIN, y);

    DrawRectangle(x + 3, y + 3, w, h, Color{ 0, 0, 0, 90 });
    DrawRectangle(x, y, w, h, Color{ 20, 18, 34, 245 });
    DrawRectangleLinesEx(Rectangle{ static_cast<float>(x), static_cast<float>(y),
                                    static_cast<float>(w), static_cast<float>(h) }, 2.0f, accent);

    DrawText(info.m_name.c_str(), x + PAD, y + PAD, TITLE_SIZE, accent);
    DrawLine(x + PAD, y + PAD + titleH - 4, x + w - PAD, y + PAD + titleH - 4, Fade(accent, 0.5f));

    int ty = y + PAD + titleH;
    for (const TipLine& l : lines) {
        DrawText(l.m_text.c_str(), x + PAD, ty, TEXT_SIZE, l.m_color);
        ty += LINE_H;
    }
}