// src/graphics/TurnOrderBar.cpp
#include "TurnOrderBar.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiTheme.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "graphics/CombatantView.h"
#include "graphics/UiInput.h"
#include "graphics/UiWidgets.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float ACTIVE_SIZE = 43.0f, NEXT_SIZE = 30.0f, GAP = 6.0f;
constexpr int   MAX_SHOWN = 7;

// ---- Chinh toc do / cam giac tai day ----
constexpr float SLIDE_RATE   = 12.0f;   // truot len/xuong, cang lon cang nhanh
constexpr float ACTIVE_RATE  = 10.0f;   // chuyen mau/vien khi doi luot
constexpr float FADE_IN_RATE = 5.0f;    // hang moi hien ra
constexpr float FADE_OUT_RATE = 4.0f;   // hang bien mat (nhan vat chet)
constexpr float FLASH_DECAY  = 3.0f;    // do nhanh tat cua tia sang khi doi luot
constexpr float ENTER_OFFSET_X = 16.0f;

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// Tien dan toi muc tieu, khong phu thuoc FPS
float approach(float current, float target, float rate, float dt) {
    return current + (target - current) * (1.0f - std::exp(-rate * dt));
}

Color lerpColor(Color a, Color b, float t) {
    t = clamp01(t);
    return Color{ static_cast<unsigned char>(a.r + (b.r - a.r) * t),
                  static_cast<unsigned char>(a.g + (b.g - a.g) * t),
                  static_cast<unsigned char>(a.b + (b.b - a.b) * t),
                  static_cast<unsigned char>(a.a + (b.a - a.a) * t) };
}

Color withAlpha(Color c, float alpha) {
    c.a = static_cast<unsigned char>(c.a * clamp01(alpha));
    return c;
}
} // namespace

void TurnOrderBar::reset() {
    m_rows.clear();
    m_lastCurrent = nullptr;
    m_flash = 0.0f;
}

void TurnOrderBar::draw(const std::vector<Character*>& upcoming, const Character* current,
                        const std::vector<std::unique_ptr<CombatantView>>& views) {
    const float dt = std::min(GetFrameTime(), 0.05f);   // tranh nhay vot khi lag
    m_time += dt;

    const Rectangle strip = UiTheme::battleTurnStrip;
    const float x0 = strip.x + 8.0f;
    const float centerY = strip.y + strip.height * 0.5f;

    // ---- Doi luot -> tia sang ----
    if (current != m_lastCurrent) {
        if (current) m_flash = 1.0f;
        m_lastCurrent = current;
    }
    m_flash = std::max(0.0f, m_flash - FLASH_DECAY * dt);

    // ---- Dong bo danh sach hien thi voi cac hang dang co ----
    for (Row& r : m_rows) { r.m_present = false; r.m_isCurrent = false; }

    int rank = 0;
    float targetX = x0;
    for (const Character* c : upcoming) {
        if (!c || rank >= MAX_SHOWN) break;
        const float size = c == current ? ACTIVE_SIZE : NEXT_SIZE;
        auto it = std::find_if(m_rows.begin(), m_rows.end(),
                               [c](const Row& r) { return r.m_character == c; });
        if (it == m_rows.end()) {
            Row fresh;
            fresh.m_character = c;
            fresh.m_x = targetX + ENTER_OFFSET_X;
            m_rows.push_back(fresh);
            it = m_rows.end() - 1;
        }
        it->m_present = true;
        it->m_isCurrent = (c == current);
        it->m_targetX = targetX;
        it->m_rank = rank + 1;
        it->m_name = c->getName();
        it->m_spd = c->getSpeed();
        targetX += size + GAP;
        ++rank;
    }

    // ---- Animation ----
    for (Row& r : m_rows) {
        if (r.m_present) {
            r.m_x = approach(r.m_x, r.m_targetX, SLIDE_RATE, dt);
            r.m_alpha = std::min(1.0f, r.m_alpha + FADE_IN_RATE * dt);
        } else {
            r.m_alpha -= FADE_OUT_RATE * dt;                 // nhan vat het trong danh sach -> mo dan
        }
        r.m_active = approach(r.m_active, r.m_isCurrent ? 1.0f : 0.0f, ACTIVE_RATE, dt);
    }
    m_rows.erase(std::remove_if(m_rows.begin(), m_rows.end(),
                                [](const Row& r) { return !r.m_present && r.m_alpha <= 0.0f; }),
                 m_rows.end());

    // ---- Draw compact horizontal turn medallions ----
    UiShapeEffects panelEffects;
    panelEffects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(strip, UiTheme::radiusLarge, UiTheme::panel,
                          UiTheme::border, 1.0f, panelEffects);
    const float pulse = 0.5f + 0.5f * std::sin(m_time * 6.0f);
    for (const Row& r : m_rows) {
        const float act = r.m_active;
        if (!r.m_character) continue;
        const float size = r.m_isCurrent ? ACTIVE_SIZE : NEXT_SIZE;
        const Rectangle bounds{ r.m_x, centerY - size * 0.5f, size, size };
        const bool hovered = CheckCollisionPointRec(UiInput::mousePosition(), bounds);
        const bool isHero = dynamic_cast<const Player*>(r.m_character) != nullptr;
        const Color team = r.m_isCurrent ? UiTheme::accent :
            (isHero ? UiTheme::verdigris : UiTheme::danger);
        const Vector2 center{ bounds.x + size * 0.5f, bounds.y + size * 0.5f };
        const float radius = size * 0.47f;
        DrawPoly(center, 6, radius, 30.0f,
                 withAlpha(r.m_isCurrent ? UiTheme::panelRaised : UiTheme::obsidian, r.m_alpha));
        DrawPolyLinesEx(center, 6, radius, 30.0f, 1.4f,
                        withAlpha(team, r.m_alpha * (r.m_isCurrent ? 1.0f : 0.8f)));
        if (act > 0.02f)
            DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y),
                radius + 1.0f + pulse * 1.5f,
                withAlpha(UiTheme::ember, std::min(1.0f,
                    (0.18f + 0.30f * pulse) * act + 0.35f * m_flash) * r.m_alpha));
        const Rectangle portrait{ center.x - size * 0.32f, center.y - size * 0.32f,
                                  size * 0.64f, size * 0.64f };
        const auto view = std::find_if(views.begin(), views.end(),
            [&r](const std::unique_ptr<CombatantView>& candidate) {
                return candidate && candidate->getCharacter() == r.m_character;
            });
        if (view != views.end()) (*view)->drawPortrait(portrait, withAlpha(WHITE, r.m_alpha));
        else UiElementSigil::draw(r.m_character->getElement(), portrait, withAlpha(WHITE, r.m_alpha));
        UiElementSigil::draw(r.m_character->getElement(),
            Rectangle{ bounds.x + size * 0.63f, bounds.y + size * 0.63f,
                       size * 0.28f, size * 0.28f }, withAlpha(WHITE, r.m_alpha));
        if (hovered) {
            const std::string tip = Strings::localizedName(r.m_name) +
                TextFormat(" · HP %d/%d · SPD %d",
                    r.m_character->getCurrentHp(), r.m_character->getStats().m_hp, r.m_spd);
            UiWidgets::Tooltip(bounds, tip);
            UiInput::setCursorType(UiInput::CursorType::POINTER);
        }
    }
}