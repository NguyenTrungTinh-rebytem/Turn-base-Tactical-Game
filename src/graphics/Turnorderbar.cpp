#include "TurnOrderBar.h"
#include "BattleUI.h"
#include "CombatantView.h"
#include "graphics/UiIcons.h"
#include "graphics/UiWidgets.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "data/Element.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr int MAX_SHOWN = 7;
constexpr float ACTIVE_SIZE = 64.0f / 1.5f;
constexpr float NEXT_SIZE = 44.0f / 1.5f;
constexpr float SLOT_GAP = 8.0f / 1.5f;
constexpr float SLIDE_RATE = 12.0f;
constexpr float ACTIVE_RATE = 10.0f;
constexpr float FADE_IN_RATE = 5.0f;
constexpr float FADE_OUT_RATE = 4.0f;
constexpr float POP_OUT = 8.0f;
constexpr float FLASH_DECAY = 3.0f;

float clamp01(float value) { return std::max(0.0f, std::min(1.0f, value)); }

float approach(float current, float target, float rate, float deltaTime) {
    return current + (target - current) * (1.0f - std::exp(-rate * deltaTime));
}

Color withAlpha(Color color, float alpha) {
    color.a = static_cast<unsigned char>(color.a * clamp01(alpha));
    return color;
}

float slotX(float start, int rank) {
    if (rank == 0) return start;
    return start + ACTIVE_SIZE + SLOT_GAP + (rank - 1) * (NEXT_SIZE + SLOT_GAP);
}

std::string elementKey(Element element) {
    switch (element) {
        case Element::KIM: return "element:kim";
        case Element::MOC: return "element:moc";
        case Element::THUY: return "element:thuy";
        case Element::HOA: return "element:hoa";
        case Element::THO: return "element:tho";
    }
    return "element:kim";
}
}

void TurnOrderBar::reset() {
    m_rows.clear();
    m_lastCurrent = nullptr;
    m_flash = 0.0f;
}

void TurnOrderBar::draw(const std::vector<Character*>& upcoming, const Character* current,
                        const std::vector<std::unique_ptr<CombatantView>>& views) {
    const float dt = std::min(GetFrameTime(), 0.05f);
    m_time += dt;
    const Rectangle strip = BattleUI::turnStripBounds();
    const float startX = strip.x + 8.0f;
    const float startY = strip.y + strip.height * 0.5f;
    DrawRectangleGradientH(static_cast<int>(strip.x), static_cast<int>(startY),
        static_cast<int>(strip.width), 1, Color{ 141, 135, 148, 105 },
        Color{ 141, 135, 148, 0 });

    if (current != m_lastCurrent) {
        if (current) m_flash = 1.0f;
        m_lastCurrent = current;
    }
    m_flash = std::max(0.0f, m_flash - FLASH_DECAY * dt);

    for (Row& row : m_rows) {
        row.m_present = false;
        row.m_isCurrent = false;
    }

    int rank = 0;
    for (const Character* character : upcoming) {
        if (!character || rank >= MAX_SHOWN) break;
        auto it = std::find_if(m_rows.begin(), m_rows.end(),
            [character](const Row& row) { return row.m_character == character; });
        if (it == m_rows.end()) {
            Row fresh;
            fresh.m_character = character;
            fresh.m_y = slotX(startX, rank);
            m_rows.push_back(fresh);
            it = m_rows.end() - 1;
        }
        it->m_present = true;
        it->m_isCurrent = character == current;
        it->m_targetY = slotX(startX, rank);
        it->m_rank = rank + 1;
        it->m_name = Strings::localizedName(character->getName());
        it->m_hp = std::max(0, character->getCurrentHp());
        it->m_maxHp = character->getStats().m_hp;
        it->m_spd = character->getSpeed();
        it->m_element = character->getElement();
        it->m_isHero = dynamic_cast<const Player*>(character) != nullptr;
        ++rank;
    }

    for (Row& row : m_rows) {
        if (row.m_present) {
            row.m_y = approach(row.m_y, row.m_targetY, SLIDE_RATE, dt);
            row.m_alpha = std::min(1.0f, row.m_alpha + FADE_IN_RATE * dt);
        } else {
            row.m_alpha -= FADE_OUT_RATE * dt;
        }
        row.m_active = approach(row.m_active, row.m_isCurrent ? 1.0f : 0.0f, ACTIVE_RATE, dt);
        row.m_x = approach(row.m_x, row.m_isCurrent ? -POP_OUT : 0.0f, SLIDE_RATE, dt);
    }
    m_rows.erase(std::remove_if(m_rows.begin(), m_rows.end(),
        [](const Row& row) { return !row.m_present && row.m_alpha <= 0.0f; }), m_rows.end());

    const float pulse = 0.5f + 0.5f * std::sin(m_time * 6.0f);
    for (const Row& row : m_rows) {
        const bool active = row.m_rank == 1;
        const float baseSize = active ? ACTIVE_SIZE : NEXT_SIZE;
        const float size = baseSize + (active ? pulse * 1.0f : 0.0f);
        const Rectangle box{ row.m_y + row.m_x, startY - size * 0.5f, size, size };
        const Vector2 center{ box.x + box.width * 0.5f, box.y + box.height * 0.5f };
        const Color ring = active ? UiTheme::crimson : UiTheme::ash;
        DrawPoly(center, 6, size * 0.5f, 30.0f,
                 withAlpha(UiTheme::ink, row.m_alpha));
        const Rectangle portrait{ box.x + size * 0.17f, box.y + size * 0.12f,
                                  size * 0.66f, size * 0.74f };
        const auto view = std::find_if(views.begin(), views.end(),
            [&row](const std::unique_ptr<CombatantView>& candidate) {
                return candidate && candidate->getCharacter() == row.m_character;
            });
        if (view == views.end() || !(*view)->drawPortrait(portrait, withAlpha(WHITE, row.m_alpha)))
            UiIcons::draw(elementKey(row.m_element), portrait,
                          withAlpha(WHITE, row.m_alpha));
        DrawPolyLines(center, 6, size * 0.5f, 30.0f,
                      withAlpha(ring, row.m_alpha));
        DrawPolyLines(center, 6, size * 0.5f - 2.0f, 30.0f,
                      withAlpha(Color{ 141, 135, 148, 155 }, row.m_alpha));
        if (active) {
            for (int tick = 0; tick < 4; ++tick) {
                const float angle = (30.0f + tick * 60.0f) * DEG2RAD;
                const Vector2 inner{ center.x + std::cos(angle) * size * 0.50f,
                                     center.y + std::sin(angle) * size * 0.50f };
                const Vector2 outer{ center.x + std::cos(angle) * (size * 0.50f + 4.0f),
                                     center.y + std::sin(angle) * (size * 0.50f + 4.0f) };
                DrawLineEx(inner, outer, 1.0f, withAlpha(UiTheme::crimsonGlow, row.m_alpha));
            }
        }
        if (row.m_isCurrent && m_flash > 0.0f)
            DrawPolyLines(center, 6, size * 0.5f + pulse * 2.0f, 30.0f,
                          withAlpha(WHITE, 0.5f * m_flash * row.m_alpha));
        const float badgeSize = active ? 12.0f : 9.0f;
        const Rectangle badge{ box.x + size - badgeSize, box.y + size - badgeSize,
                               badgeSize, badgeSize };
        UiIcons::draw(elementKey(row.m_element), badge,
                      withAlpha(WHITE, row.m_alpha));
        const std::string tooltip = row.m_name +
            TextFormat(u8" · HP %d/%d · Tốc độ %d", row.m_hp, row.m_maxHp, row.m_spd);
        UiWidgets::Tooltip(box, tooltip);
    }
}
