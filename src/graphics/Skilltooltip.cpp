// src/graphics/SkillTooltip.cpp
#include "SkillTooltip.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <vector>

namespace {
constexpr int PAD = 10;
constexpr int TITLE_SIZE = 18;
constexpr int TEXT_SIZE = 16;
constexpr int LINE_H = 24;
constexpr int MIN_W = 300;
constexpr int MAX_W = 520;
constexpr int MARGIN = 16;

struct TipLine {
    std::string m_text;
    Color m_color;
};

const char* elementText(Element e) {
    switch (e) {
        case Element::KIM:  return Strings::elements[0];
        case Element::MOC:  return Strings::elements[1];
        case Element::THUY: return Strings::elements[2];
        case Element::HOA:  return Strings::elements[3];
        case Element::THO:  return Strings::elements[4];
    }
    return "?";
}

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return UiTheme::kim;
        case Element::MOC:  return UiTheme::moc;
        case Element::THUY: return UiTheme::thuy;
        case Element::HOA:  return UiTheme::hoa;
        case Element::THO:  return UiTheme::tho;
    }
    return WHITE;
}

const char* typeText(SkillType t) {
    switch (t) {
        case SkillType::DAMAGE: return Strings::skillTypeDamage;
        case SkillType::HEAL:   return Strings::skillTypeHeal;
        case SkillType::BUFF:   return Strings::skillTypeBuff;
        case SkillType::DEBUFF: return Strings::skillTypeDebuff;
    }
    return "?";
}

const char* effectName(EffectType e) {
    switch (e) {
        case EffectType::ATK_BOOST: return Strings::effectAttackBoost;
        case EffectType::DEF_BOOST: return Strings::effectDefenseBoost;
        case EffectType::REGEN:     return Strings::effectRegeneration;
        case EffectType::STUN:      return Strings::effectStun;
        case EffectType::BURN:      return Strings::effectBurn;
        case EffectType::SLOW:      return Strings::effectSlow;
        case EffectType::WEAKNESS:  return Strings::effectWeakness;
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
    const Color white = UiTheme::text;
    const Color gray = UiTheme::ash;
    const Color gold = UiTheme::currency;
    const Color blue = UiTheme::mpTop;
    const Color green = UiTheme::heal;
    const Color orange = UiTheme::ember;
    const Color red = UiTheme::danger;

    const bool dealsDamage = !s.m_isHeal && s.m_power > 0;

    std::vector<TipLine> lines;
    std::string header = s.m_typeText + Strings::skillHeader + elementText(s.m_element);
    if (dealsDamage) header += s.m_isMagic ? Strings::magicDamageType : Strings::physicalDamageType;
    lines.push_back({ header, gray });
    lines.push_back({ TextFormat(Strings::skillCostTemplate, s.m_mpCost), blue });

    if (dealsDamage) {
        lines.push_back({ TextFormat(Strings::skillDamageTemplate, s.m_power,
                                     s.m_isMagic ? Strings::magicAttack : Strings::physicalAttack), white });
        if (caster) {
            const int atk = s.m_isMagic ? caster->m_ma : caster->m_a;
            lines.push_back({ TextFormat(Strings::estimatedDamageTemplate, atk * s.m_power / 100), gold });
        }
    } else if (s.m_isHeal) {
        lines.push_back({ TextFormat(Strings::healingTemplate, s.m_power), green });
    }

    if (!s.m_effectText.empty()) lines.push_back({ std::string(Strings::effectPrefix) + s.m_effectText, orange });

    if (dealsDamage) {
        lines.push_back({ std::string(Strings::strongAgainstPrefix) + elementList(s.m_element, true), green });
        lines.push_back({ std::string(Strings::weakAgainstPrefix) + elementList(s.m_element, false), red });
    }
    return lines;
}
} // namespace

SkillTipInfo SkillTooltip::fromSkill(const Skill& skill) {
    SkillTipInfo t;
    t.m_name = Strings::localizedName(skill.m_name);
    t.m_element = skill.m_element;
    t.m_typeText = typeText(skill.m_type);
    t.m_power = skill.m_power;
    t.m_mpCost = skill.m_mpCost;
    t.m_isHeal = (skill.m_type == SkillType::HEAL);
    t.m_isMagic = skill.m_isMagic;
    if (skill.m_effect != EffectType::NONE && skill.m_effectTurns > 0)
        t.m_effectText = std::string(effectName(skill.m_effect)) + " (" +
                         std::to_string(skill.m_effectTurns) + Strings::turnsSuffix;
    return t;
}

void SkillTooltip::draw(const Skill& skill, Vector2 mousePos, const Stats* caster, Rectangle trigger) {
    draw(fromSkill(skill), mousePos, caster, trigger);
}

void SkillTooltip::draw(const SkillTipInfo& info, Vector2 mousePos, const Stats* caster,
                        Rectangle trigger) {
    const std::vector<TipLine> lines = buildLines(info, caster);
    const Color accent = elementColor(info.m_element);

    int w = std::max(MIN_W, UiText::measure(info.m_name.c_str(), TITLE_SIZE) + 3 * PAD + 20);
    for (const TipLine& l : lines)
        w = std::max(w, std::min(MAX_W, UiText::measure(l.m_text.c_str(), TEXT_SIZE) + 2 * PAD));
    w = std::min(MAX_W, w);
    const int titleH = TITLE_SIZE + 8;
    const int h = PAD * 2 + titleH + static_cast<int>(lines.size()) * LINE_H;

    const int sw = UiCanvas::width(), sh = UiCanvas::height();
    if (trigger.width <= 0.0f || trigger.height <= 0.0f)
        trigger = Rectangle{ mousePos.x, mousePos.y, 1.0f, 1.0f };
    const Rectangle candidates[] = {
        { trigger.x + trigger.width + 12.0f, trigger.y, static_cast<float>(w), static_cast<float>(h) },
        { trigger.x - w - 12.0f, trigger.y, static_cast<float>(w), static_cast<float>(h) },
        { trigger.x, trigger.y + trigger.height + 12.0f, static_cast<float>(w), static_cast<float>(h) },
        { trigger.x, trigger.y - h - 12.0f, static_cast<float>(w), static_cast<float>(h) }
    };
    Rectangle bounds{};
    bool placed = false;
    for (const Rectangle& candidate : candidates) {
        Rectangle clamped = candidate;
        clamped.x = std::clamp(clamped.x, static_cast<float>(MARGIN),
                               static_cast<float>(sw - MARGIN) - clamped.width);
        clamped.y = std::clamp(clamped.y, static_cast<float>(MARGIN),
                               static_cast<float>(sh - MARGIN) - clamped.height);
        if (CheckCollisionRecs(clamped, trigger)) continue;
        bounds = clamped;
        placed = true;
        break;
    }
    if (!placed) return;
    const int x = static_cast<int>(bounds.x);
    const int y = static_cast<int>(bounds.y);

    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    effects.shadowBlur = UiTheme::floatingShadowSize;
    UiShapes::roundedRect(bounds, UiTheme::radiusTooltip,
                          Color{ UiTheme::panel.r, UiTheme::panel.g, UiTheme::panel.b, 240 },
                          accent, 1.0f, effects);

    UiElementSigil::draw(info.m_element,
                         Rectangle{ static_cast<float>(x + PAD), static_cast<float>(y + PAD),
                                    22.0f, 22.0f });
    UiText::drawEllipsis(info.m_name, x + PAD + 28, y + PAD, w - 3 * PAD - 28,
                         TITLE_SIZE, accent, UiFontRole::HUD);
    UiShapes::roundedRect(Rectangle{ static_cast<float>(x + PAD),
                                     static_cast<float>(y + PAD + titleH - 4),
                                     static_cast<float>(w - 2 * PAD), 2.0f },
                          UiTheme::radiusBar, Fade(accent, 0.5f));

    int ty = y + PAD + titleH;
    for (const TipLine& l : lines) {
        UiText::drawEllipsis(l.m_text, x + PAD, ty, w - 2 * PAD, TEXT_SIZE, l.m_color, UiFontRole::BODY);
        ty += LINE_H;
    }
}