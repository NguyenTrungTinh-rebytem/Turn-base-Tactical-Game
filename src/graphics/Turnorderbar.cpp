// src/graphics/TurnOrderBar.cpp
#include "TurnOrderBar.h"
#include "entities/Character.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float BOX_W = 170.0f, BOX_H = 30.0f, GAP = 4.0f, MARGIN = 10.0f;
constexpr int   MAX_SHOWN = 8;

// ---- Chinh toc do / cam giac tai day ----
constexpr float SLIDE_RATE   = 12.0f;   // truot len/xuong, cang lon cang nhanh
constexpr float ACTIVE_RATE  = 10.0f;   // chuyen mau/vien khi doi luot
constexpr float FADE_IN_RATE = 5.0f;    // hang moi hien ra
constexpr float FADE_OUT_RATE = 4.0f;   // hang bien mat (nhan vat chet)
constexpr float POP_OUT_PX   = 14.0f;   // nguoi dang di nhich ra ben trai bay nhieu px
constexpr float FLASH_DECAY  = 3.0f;    // do nhanh tat cua tia sang khi doi luot
constexpr float ENTER_OFFSET_Y = 16.0f; // hang moi truot tu duoi len

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

void TurnOrderBar::draw(const std::vector<Character*>& upcoming, const Character* current) {
    const float dt = std::min(GetFrameTime(), 0.05f);   // tranh nhay vot khi lag
    m_time += dt;

    const float x0 = GetScreenWidth() - BOX_W - MARGIN;
    const float y0 = MARGIN + 22.0f;
    const float step = BOX_H + GAP;

    // ---- Doi luot -> tia sang ----
    if (current != m_lastCurrent) {
        if (current) m_flash = 1.0f;
        m_lastCurrent = current;
    }
    m_flash = std::max(0.0f, m_flash - FLASH_DECAY * dt);

    // ---- Dong bo danh sach hien thi voi cac hang dang co ----
    for (Row& r : m_rows) { r.m_present = false; r.m_isCurrent = false; }

    int rank = 0;
    for (const Character* c : upcoming) {
        if (!c || rank >= MAX_SHOWN) break;
        auto it = std::find_if(m_rows.begin(), m_rows.end(),
                               [c](const Row& r) { return r.m_character == c; });
        if (it == m_rows.end()) {
            Row fresh;
            fresh.m_character = c;
            fresh.m_y = y0 + rank * step + ENTER_OFFSET_Y;   // truot tu duoi len
            m_rows.push_back(fresh);
            it = m_rows.end() - 1;
        }
        it->m_present = true;
        it->m_isCurrent = (c == current);
        it->m_targetY = y0 + rank * step;
        it->m_rank = rank + 1;
        it->m_name = c->getName();
        it->m_spd = c->getSpeed();
        ++rank;
    }

    // ---- Animation ----
    for (Row& r : m_rows) {
        if (r.m_present) {
            r.m_y = approach(r.m_y, r.m_targetY, SLIDE_RATE, dt);
            r.m_alpha = std::min(1.0f, r.m_alpha + FADE_IN_RATE * dt);
        } else {
            r.m_alpha -= FADE_OUT_RATE * dt;                 // nhan vat het trong danh sach -> mo dan
        }
        r.m_active = approach(r.m_active, r.m_isCurrent ? 1.0f : 0.0f, ACTIVE_RATE, dt);
        r.m_x = approach(r.m_x, r.m_isCurrent ? -POP_OUT_PX : 0.0f, SLIDE_RATE, dt);
    }
    m_rows.erase(std::remove_if(m_rows.begin(), m_rows.end(),
                                [](const Row& r) { return !r.m_present && r.m_alpha <= 0.0f; }),
                 m_rows.end());

    // ---- Ve ----
    DrawText("TURN ORDER", static_cast<int>(x0), static_cast<int>(MARGIN), 16, GOLD);

    const float pulse = 0.5f + 0.5f * std::sin(m_time * 6.0f);
    for (const Row& r : m_rows) {
        const float act = r.m_active;
        const Rectangle box = { x0 + r.m_x, r.m_y, BOX_W, BOX_H };

        DrawRectangleRec(box, withAlpha(lerpColor(Color{ 25, 25, 40, 200 }, Color{ 90, 70, 20, 230 }, act), r.m_alpha));
        if (r.m_isCurrent && m_flash > 0.0f)
            DrawRectangleRec(box, withAlpha(WHITE, 0.45f * m_flash * r.m_alpha));   // tia sang khi vua doi luot
        if (act > 0.01f)
            DrawRectangleLinesEx(Rectangle{ box.x - 3.0f, box.y - 3.0f, box.width + 6.0f, box.height + 6.0f },
                                 1.0f, withAlpha(GOLD, (0.15f + 0.30f * pulse) * act * r.m_alpha));   // quang nhap nhay
        DrawRectangleLinesEx(box, 1.0f + 1.5f * act, withAlpha(lerpColor(GRAY, GOLD, act), r.m_alpha));

        DrawText(TextFormat("%d. %s", r.m_rank, r.m_name.c_str()),
                 static_cast<int>(box.x) + 8, static_cast<int>(box.y) + 8, 14, withAlpha(RAYWHITE, r.m_alpha));
        const char* spd = TextFormat("SPD %d", r.m_spd);
        DrawText(spd, static_cast<int>(box.x + BOX_W - MeasureText(spd, 12) - 8), static_cast<int>(box.y) + 9, 12,
                 withAlpha(SKYBLUE, r.m_alpha));
    }
}