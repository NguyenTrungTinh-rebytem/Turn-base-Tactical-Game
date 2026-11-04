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
constexpr std::size_t MAX_SHOWN = 12;
constexpr float ACTIVE_R = 22.0f;   // kim cương người đang đi (trước là 17)
constexpr float NEXT_R = 16.0f;     // chân dung các lượt sau (trước là 11)
constexpr float GAP = 12.0f;
constexpr float SLIDE_RATE = 12.0f;
constexpr float FADE_IN_RATE = 5.0f;
constexpr float FADE_OUT_RATE = 4.0f;
constexpr float FLASH_DECAY = 3.0f;

float clamp01(float value) { return std::max(0.0f, std::min(1.0f, value)); }

float approach(float current, float target, float rate, float deltaTime) {
    return current + (target - current) * (1.0f - std::exp(-rate * deltaTime));
}

Color withAlpha(Color color, float alpha) {
    color.a = static_cast<unsigned char>(color.a * clamp01(alpha));
    return color;
}

// Toa do X tam cua o thu `rank` (0 = nguoi dang di)
float centerX(float start, int rank) {
    if (rank == 0) return start + ACTIVE_R;
    return start + ACTIVE_R * 2.0f + GAP + NEXT_R + (rank - 1) * (NEXT_R * 2.0f + GAP);
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

    // ---- Nen dai mong toan chieu ngang ----
    const Rectangle strip = BattleUI::turnStripBounds();
    const float midY = strip.y + strip.height * 0.5f;
    const float startX = strip.x + 14.0f;
    UiShapes::plate(strip, UiTheme::navyPanel, UiTheme::cyanEdge, 1.0f);

    // ---- Thu tu hien thi: nguoi dang di luon dung dau, bo nguoi da chet / trung lap ----
    std::vector<const Character*> order;
    if (current && current->isAlive()) order.push_back(current);
    for (const Character* character : upcoming) {
        if (order.size() >= MAX_SHOWN) break;
        if (!character || !character->isAlive() || character == current) continue;
        order.push_back(character);
    }

    const Character* first = order.empty() ? nullptr : order.front();
    if (first != m_lastCurrent) {
        if (first) m_flash = 1.0f;
        m_lastCurrent = first;
    }
    m_flash = std::max(0.0f, m_flash - FLASH_DECAY * dt);

    for (Row& row : m_rows) {
        row.m_present = false;
        row.m_isCurrent = false;
    }

    int rank = 0;
    for (const Character* character : order) {
        auto it = std::find_if(m_rows.begin(), m_rows.end(),
            [character](const Row& row) { return row.m_character == character; });
        if (it == m_rows.end()) {
            Row fresh;
            fresh.m_character = character;
            fresh.m_y = centerX(startX, rank);
            m_rows.push_back(fresh);
            it = m_rows.end() - 1;
        }
        it->m_present = true;
        it->m_isCurrent = (rank == 0);
        it->m_targetY = centerX(startX, rank);
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
    }
    m_rows.erase(std::remove_if(m_rows.begin(), m_rows.end(),
        [](const Row& row) { return !row.m_present && row.m_alpha <= 0.0f; }), m_rows.end());

    // ---- Ve ----
    const float pulse = 0.5f + 0.5f * std::sin(m_time * 6.0f);
    const Color navy{ 12, 10, 38, 235 };
    for (const Row& row : m_rows) {
        const bool active = row.m_present && row.m_isCurrent;
        const float radius = active ? ACTIVE_R + pulse : NEXT_R;
        const Vector2 center{ row.m_y, midY };
        const Color accent = row.m_isHero ? UiTheme::cyan : UiTheme::hpEnemy;

        Rectangle portrait;
        if (active) {
            // Kim cuong lon
            DrawPoly(center, 4, radius, 0.0f, withAlpha(navy, row.m_alpha));
            DrawPolyLinesEx(center, 4, radius, 0.0f, 2.0f, withAlpha(accent, row.m_alpha));
            if (m_flash > 0.0f)
                DrawPolyLinesEx(center, 4, radius + 4.0f + pulse * 2.0f, 0.0f, 1.5f,
                                withAlpha(WHITE, 0.6f * m_flash * row.m_alpha));
            const float side = radius * 1.04f;
            portrait = Rectangle{ center.x - side * 0.5f, center.y - side * 0.5f, side, side };
        } else {
            // Chan dung tron nho
            DrawCircleV(center, radius, withAlpha(navy, row.m_alpha));
            DrawRing(center, radius - 1.5f, radius, 0.0f, 360.0f, 24,
                     withAlpha(accent, row.m_alpha * 0.85f));
            const float side = radius * 1.38f;
            portrait = Rectangle{ center.x - side * 0.5f, center.y - side * 0.5f, side, side };
        }

        const auto view = std::find_if(views.begin(), views.end(),
            [&row](const std::unique_ptr<CombatantView>& candidate) {
                return candidate && candidate->getCharacter() == row.m_character;
            });
        if (view == views.end() || !(*view)->drawPortrait(portrait, withAlpha(WHITE, row.m_alpha)))
            UiIcons::draw(elementKey(row.m_element), portrait, withAlpha(WHITE, row.m_alpha));

        const Rectangle hit{ center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f };
        const std::string tooltip = row.m_name +
            TextFormat(" · HP %d/%d · Tốc độ %d", row.m_hp, row.m_maxHp, row.m_spd);
        UiWidgets::Tooltip(hit, tooltip);
    }
}
