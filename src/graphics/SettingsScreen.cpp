#include "graphics/SettingsScreen.h"
#include "data/GameSettings.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
constexpr Color COL_TEXT = UiTheme::silver;
constexpr Color COL_DIM = UiTheme::ashText;
constexpr Color COL_GOLD = UiTheme::gold;
constexpr Color COL_PANEL = UiTheme::panel;

// Chon chuoi theo ngon ngu hien tai
const char* tr(const char* vi, const char* en) {
    return GameSettings::get().isEnglish() ? en : vi;
}

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

void drawCentered(const std::string& text, const Rectangle& r, int size, Color color) {
    const int w = UiText::measure(text, size);
    UiText::draw(text, static_cast<int>(r.x + (r.width - w) * 0.5f),
                 static_cast<int>(r.y + (r.height - size) * 0.5f - 1), size, color);
}

// Ve doan van tu dong xuong dong theo do rong; tra ve y ngay sau doan
int drawWrapped(const std::string& text, int x, int y, int width, int size, int lineH, Color color) {
    std::string line, word;
    auto flush = [&]() {
        if (!line.empty()) { UiText::draw(line, x, y, size, color); y += lineH; line.clear(); }
    };
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == ' ') {
            if (!word.empty()) {
                const std::string candidate = line.empty() ? word : line + " " + word;
                if (!line.empty() && UiText::measure(candidate, size) > width) { flush(); line = word; }
                else line = candidate;
                word.clear();
            }
        } else word += text[i];
    }
    flush();
    return y;
}

struct Member {
    const char* m_name;
    const char* m_id;
    const char* m_roleVi;
    const char* m_roleEn;
    const char* m_workVi;
    const char* m_workEn;
};

// ---- SUA THONG TIN THANH VIEN / CO TRUYEN TAI DAY ----
const Member MEMBERS[] = {
    { u8"Nguyễn Trung Tỉnh", "6651071077", u8"Trưởng nhóm (Director)", "Team Leader (Director)",
      u8"Quản lý dự án, thiết kế nhân vật & quái, Data Engine, Audio Engine, UI Menu",
      "Project lead, character & monster design, Data Engine, Audio Engine, Menu UI" },
    { u8"Võ Tấn Tiến", "6651071074", "Core Engine & FSM", "Core Engine & FSM",
      u8"Game Loop, máy trạng thái FSM, Command Pattern (đánh, skill, item, hoàn tác)",
      "Game loop, FSM, Command Pattern (attack, skill, item, undo)" },
    { u8"Nguyễn Quang Nhân Kiệt", "6651071035", "OOP Entities & Factory", "OOP Entities & Factory",
      u8"Cây kế thừa Character, công thức sát thương Ngũ Hành, Factory Method, Inventory<T>",
      "Character hierarchy, Five-Element damage formula, Factory Method, Inventory<T>" },
    { u8"Nguyễn Quốc Đạt", "6651071014", "DSA & Turn System", "DSA & Turn System",
      u8"TurnManager (priority_queue), 7 hiệu ứng Buff/Debuff, Observer Pattern, AI Boss",
      "TurnManager (priority_queue), 7 buff/debuff effects, Observer Pattern, Boss AI" },
    { u8"Huỳnh Quang Khải", "6651071030", "UI/UX & Graphics", "UI/UX & Graphics",
      u8"Sprite Animation 2D, bản đồ chiến trường, HUD thanh máu, danh sách lượt",
      "2D sprite animation, battle map, HP/MP HUD, turn order bar" },
};

struct StoryBlock { bool m_heading; const char* m_vi; const char* m_en; };

const StoryBlock STORY[] = {
    { true,  u8"Bối cảnh", "Setting" },
    { false, u8"Đại lục được dựng nên từ Ngũ Hành: Kim, Mộc, Thủy, Hỏa, Thổ. Năm Phong Ấn cổ xưa trấn giữ năm nguồn sức mạnh nguyên thủy, giữ cho thế giới cân bằng và ngăn bóng tối tràn qua biên giới.",
             "The land was shaped by the Five Elements: Metal, Wood, Water, Fire and Earth. Five ancient Seals guard those primal powers, keeping the world in balance and the darkness at bay." },
    { true,  u8"Cốt truyện", "Story" },
    { false, u8"Nhưng các Phong Ấn đã rạn nứt. Quái vật tràn ra từ vùng đất hoang, mang theo sức mạnh của từng hệ; ở nơi sâu nhất, Rồng Lửa đã thức giấc, quyết phá nốt những Phong Ấn còn lại.",
             "But the Seals have cracked. Monsters pour out of the wastelands carrying the power of each element, and in the deepest place the Fire Dragon has awoken, bent on breaking the remaining Seals." },
    { false, u8"Bạn dẫn dắt đội tối đa ba anh hùng, mỗi người một hệ. Hãy nắm quy luật tương khắc, vượt qua hai làn sóng quái vật, chuẩn bị trang bị ở cửa hàng rồi đối mặt Boss - đó là bước đầu tiên để khôi phục các Phong Ấn.",
             "You lead a party of up to three heroes, each of a different element. Master the clash of elements, survive two waves of monsters, gear up at the shop and face the Boss - the first step toward restoring the Seals." },
};
} // namespace

// ---------------- Bo cuc (khung 1280x720) ----------------
Rectangle SettingsScreen::panelRect() {
    const float w = 900.0f, h = 540.0f;
    return { (UiCanvas::width() - w) * 0.5f, (UiCanvas::height() - h) * 0.5f, w, h };
}
Rectangle SettingsScreen::tabRect(int index) {
    const Rectangle p = panelRect();
    return { p.x + 20.0f, p.y + 86.0f + index * 62.0f, 210.0f, 52.0f };
}
Rectangle SettingsScreen::closeRect() {
    const Rectangle p = panelRect();
    return { p.x + p.width - 56.0f, p.y + 16.0f, 40.0f, 40.0f };
}
Rectangle SettingsScreen::contentRect() {
    const Rectangle p = panelRect();
    return { p.x + 250.0f, p.y + 86.0f, p.width - 270.0f, p.height - 106.0f };
}
Rectangle SettingsScreen::sliderTrack(int row) {
    const Rectangle c = contentRect();
    return { c.x + 24.0f, c.y + 78.0f + row * 96.0f, c.width - 150.0f, 10.0f };
}
Rectangle SettingsScreen::toggleRect() {
    const Rectangle c = contentRect();
    return { c.x + c.width - 100.0f, c.y + 78.0f + 2 * 96.0f - 14.0f, 76.0f, 34.0f };
}
Rectangle SettingsScreen::languageRect(int index) {
    const Rectangle c = contentRect();
    return { c.x + 24.0f + index * 190.0f, c.y + 80.0f, 170.0f, 90.0f };
}
Rectangle SettingsScreen::menuButtonRect() {
    return { UiCanvas::width() - 200.0f, UiCanvas::height() - 70.0f, 170.0f, 44.0f };
}

void SettingsScreen::drawMenuButton(bool hover) {
    const Rectangle r = menuButtonRect();
    UiShapes::plate(r, hover ? Color{ 70, 56, 30, 230 } : Color{ 18, 16, 28, 200 },
                    hover ? COL_GOLD : UiTheme::ash, 1.0f);
    drawCentered(std::string(tr(u8"Cài đặt", "Settings")) + "  [F1]", r, 17, hover ? COL_GOLD : COL_TEXT);
}

// ---------------- Logic ----------------
void SettingsScreen::open() {
    m_open = true;
    m_time = 0.0f;
    m_drag = -1;
    m_row = 0;
}

void SettingsScreen::update(float deltaTime) {
    if (m_open) m_time += deltaTime;
}

SettingsAction SettingsScreen::handleInput() {
    if (!m_open) return SettingsAction::NONE;
    GameSettings& s = GameSettings::get();
    const Vector2 mouse = UiCanvas::mousePosition();
    SettingsAction result = SettingsAction::NONE;

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_F1)) { m_open = false; return SettingsAction::CLOSE; }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, closeRect())) {
        m_open = false;
        return SettingsAction::CLOSE;
    }

    // ---- Chuyen tab: chuot, phim 1-4, TAB ----
    if (IsKeyPressed(KEY_ONE))   m_tab = TAB_AUDIO;
    if (IsKeyPressed(KEY_TWO))   m_tab = TAB_LANGUAGE;
    if (IsKeyPressed(KEY_THREE)) m_tab = TAB_STORY;
    if (IsKeyPressed(KEY_FOUR))  m_tab = TAB_TEAM;
    if (IsKeyPressed(KEY_TAB))   m_tab = (m_tab + ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ? TAB_COUNT - 1 : 1)) % TAB_COUNT;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        for (int i = 0; i < TAB_COUNT; ++i)
            if (CheckCollisionPointRec(mouse, tabRect(i))) { m_tab = i; m_drag = -1; }

    if (m_tab == TAB_AUDIO) {
        float* values[2] = { &s.m_musicVolume, &s.m_sfxVolume };

        // Phim: len/xuong doi dong, trai/phai chinh 5%
        if (IsKeyPressed(KEY_DOWN)) m_row = (m_row + 1) % 3;
        if (IsKeyPressed(KEY_UP))   m_row = (m_row + 2) % 3;
        const bool left = IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT);
        const bool right = IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT);
        if (left || right) {
            if (m_row < 2) {
                *values[m_row] = clamp01(*values[m_row] + (right ? 0.05f : -0.05f));
                result = (m_row == 1) ? SettingsAction::SFX_PREVIEW : SettingsAction::CHANGED;
            } else { s.m_shaderEnabled = right; result = SettingsAction::CHANGED; }
        }

        // Chuot: bam/keo thanh truot
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int row = 0; row < 2; ++row) {
                Rectangle hit = sliderTrack(row);
                hit.y -= 16.0f; hit.height += 32.0f; hit.x -= 8.0f; hit.width += 16.0f;
                if (CheckCollisionPointRec(mouse, hit)) { m_drag = row; m_row = row; }
            }
            Rectangle toggleHit = toggleRect();
            if (CheckCollisionPointRec(mouse, toggleHit)) {
                s.m_shaderEnabled = !s.m_shaderEnabled;
                m_row = 2;
                result = SettingsAction::CHANGED;
            }
        }
        if (m_drag >= 0) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                const Rectangle t = sliderTrack(m_drag);
                const float v = clamp01((mouse.x - t.x) / t.width);
                if (std::fabs(v - *values[m_drag]) > 0.0001f) {
                    *values[m_drag] = v;
                    result = SettingsAction::CHANGED;
                }
            } else {
                if (m_drag == 1) result = SettingsAction::SFX_PREVIEW;   // tha chuot: nghe thu am luong hieu ung
                m_drag = -1;
            }
        }
    } else if (m_tab == TAB_LANGUAGE) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < 2; ++i)
                if (CheckCollisionPointRec(mouse, languageRect(i))) {
                    const Language lang = (i == 0) ? Language::VI : Language::EN;
                    if (s.m_language != lang) { s.m_language = lang; result = SettingsAction::CHANGED; }
                }
        }
        if (IsKeyPressed(KEY_LEFT))  { if (s.m_language != Language::VI) { s.m_language = Language::VI; result = SettingsAction::CHANGED; } }
        if (IsKeyPressed(KEY_RIGHT)) { if (s.m_language != Language::EN) { s.m_language = Language::EN; result = SettingsAction::CHANGED; } }
    }
    return result;
}

// ---------------- Ve ----------------
void SettingsScreen::draw() const {
    if (!m_open) return;
    const float appear = clamp01(m_time / 0.18f);
    const unsigned char a = static_cast<unsigned char>(255 * appear);
    const Vector2 mouse = UiCanvas::mousePosition();

    DrawRectangle(0, 0, UiCanvas::width(), UiCanvas::height(), Color{ 0, 0, 0, static_cast<unsigned char>(170 * appear) });
    const Rectangle p = panelRect();
    UiShapes::plate(p, COL_PANEL, UiTheme::ash, 1.0f);
    UiText::draw(tr(u8"CÀI ĐẶT", "SETTINGS"), static_cast<int>(p.x + 24), static_cast<int>(p.y + 22), 30, COL_GOLD);
    DrawLineEx(Vector2{ p.x + 20, p.y + 70 }, Vector2{ p.x + p.width - 20, p.y + 70 }, 1.0f, Color{ 255, 214, 96, 90 });

    // Nut dong
    const Rectangle cr = closeRect();
    const bool closeHover = CheckCollisionPointRec(mouse, cr);
    UiShapes::plate(cr, closeHover ? Color{ 120, 40, 40, 230 } : Color{ 30, 26, 40, 220 }, UiTheme::ash, 1.0f);
    drawCentered("X", cr, 20, COL_TEXT);

    // Tab
    const char* tabNames[TAB_COUNT] = {
        tr(u8"Âm thanh", "Audio"), tr(u8"Ngôn ngữ", "Language"),
        tr(u8"Cốt truyện", "Story"), tr(u8"Thành viên", "Credits") };
    for (int i = 0; i < TAB_COUNT; ++i) {
        const Rectangle r = tabRect(i);
        const bool sel = (i == m_tab);
        const bool hov = CheckCollisionPointRec(mouse, r);
        UiShapes::plate(r, sel ? Color{ 86, 62, 24, 235 } : (hov ? Color{ 44, 38, 60, 230 } : Color{ 24, 20, 36, 220 }),
                        sel ? COL_GOLD : UiTheme::ash, 1.0f);
        UiText::draw(std::string(TextFormat("%d  ", i + 1)) + tabNames[i], static_cast<int>(r.x + 16),
                     static_cast<int>(r.y + 15), 19, sel ? COL_GOLD : COL_TEXT);
    }
    UiText::draw(tr(u8"ESC: đóng", "ESC: close"), static_cast<int>(p.x + 24),
                 static_cast<int>(p.y + p.height - 34), 14, COL_DIM);

    const Rectangle c = contentRect();
    switch (m_tab) {
    case TAB_AUDIO:    drawAudio(c); break;
    case TAB_LANGUAGE: drawLanguage(c); break;
    case TAB_STORY:    drawStory(c); break;
    default:           drawTeam(c); break;
    }
    if (appear < 1.0f)   // fade-in nhe
        DrawRectangleRec(p, Color{ 0, 0, 0, static_cast<unsigned char>(255 - a) });
}

void SettingsScreen::drawAudio(const Rectangle& area) const {
    const GameSettings& s = GameSettings::get();
    UiText::draw(tr(u8"Âm thanh & Hiển thị", "Audio & Display"), static_cast<int>(area.x + 24), static_cast<int>(area.y + 8), 24, COL_TEXT);

    const char* labels[2] = { tr(u8"Nhạc nền", "Music"), tr(u8"Hiệu ứng", "Sound effects") };
    const float values[2] = { s.m_musicVolume, s.m_sfxVolume };
    for (int row = 0; row < 2; ++row) {
        const Rectangle t = sliderTrack(row);
        const bool selected = (m_row == row);
        UiText::draw(labels[row], static_cast<int>(t.x), static_cast<int>(t.y - 34), 19, selected ? COL_GOLD : COL_TEXT);
        DrawRectangleRec(t, Color{ 14, 12, 22, 255 });
        DrawRectangleLinesEx(t, 1.0f, UiTheme::ash);
        DrawRectangleRec(Rectangle{ t.x, t.y, t.width * values[row], t.height }, COL_GOLD);
        const float kx = t.x + t.width * values[row];
        DrawCircleV(Vector2{ kx, t.y + t.height * 0.5f }, 13.0f, Color{ 20, 16, 30, 255 });
        DrawCircleV(Vector2{ kx, t.y + t.height * 0.5f }, 11.0f, (m_drag == row || selected) ? COL_GOLD : COL_TEXT);
        UiText::draw(TextFormat("%d%%", static_cast<int>(std::lround(values[row] * 100.0f))),
                     static_cast<int>(t.x + t.width + 24), static_cast<int>(t.y - 12), 22, COL_TEXT);
    }

    // Bat/tat shader
    const Rectangle tg = toggleRect();
    UiText::draw(tr(u8"Hiệu ứng ánh sáng cảnh (F9)", "Scene lighting effect (F9)"),
                 static_cast<int>(sliderTrack(2).x), static_cast<int>(tg.y + 6), 19, m_row == 2 ? COL_GOLD : COL_TEXT);
    UiShapes::plate(tg, s.m_shaderEnabled ? Color{ 40, 110, 60, 235 } : Color{ 60, 36, 40, 235 }, UiTheme::ash, 1.0f);
    drawCentered(s.m_shaderEnabled ? tr(u8"BẬT", "ON") : tr(u8"TẮT", "OFF"), tg, 17, COL_TEXT);

    UiText::draw(tr(u8"Kéo thanh trượt, hoặc dùng ↑ ↓ chọn dòng và ← → chỉnh 5%.",
                    "Drag a slider, or use Up/Down to pick a row and Left/Right to adjust 5%."),
                 static_cast<int>(area.x + 24), static_cast<int>(area.y + area.height - 40), 14, COL_DIM);
}

void SettingsScreen::drawLanguage(const Rectangle& area) const {
    const GameSettings& s = GameSettings::get();
    const Vector2 mouse = UiCanvas::mousePosition();
    UiText::draw(tr(u8"Ngôn ngữ", "Language"), static_cast<int>(area.x + 24), static_cast<int>(area.y + 8), 24, COL_TEXT);
    const char* names[2] = { "Tiếng Việt", "English" };
    const char* sub[2] = { "VI", "EN" };
    for (int i = 0; i < 2; ++i) {
        const Rectangle r = languageRect(i);
        const bool sel = (i == 0) == !s.isEnglish();
        const bool hov = CheckCollisionPointRec(mouse, r);
        UiShapes::plate(r, sel ? Color{ 86, 62, 24, 235 } : (hov ? Color{ 44, 38, 60, 230 } : Color{ 24, 20, 36, 220 }),
                        sel ? COL_GOLD : UiTheme::ash, 1.0f);
        UiText::draw(sub[i], static_cast<int>(r.x + 16), static_cast<int>(r.y + 12), 16, COL_DIM);
        UiText::draw(names[i], static_cast<int>(r.x + 16), static_cast<int>(r.y + 44), 24, sel ? COL_GOLD : COL_TEXT);
    }
    UiText::draw(tr(u8"Cài đặt được lưu tự động và giữ nguyên ở lần chơi sau.",
                    "Settings are saved automatically and kept for your next session."),
                 static_cast<int>(area.x + 24), static_cast<int>(area.y + 200), 16, COL_DIM);
}

void SettingsScreen::drawStory(const Rectangle& area) const {
    int y = static_cast<int>(area.y + 8);
    const int x = static_cast<int>(area.x + 24);
    const int w = static_cast<int>(area.width - 48);
    for (const StoryBlock& b : STORY) {
        const char* text = tr(b.m_vi, b.m_en);
        if (b.m_heading) {
            if (y > area.y + 10) y += 8;
            UiText::draw(text, x, y, 24, COL_GOLD);
            y += 36;
        } else {
            y = drawWrapped(text, x, y, w, 18, 27, COL_TEXT) + 10;
        }
    }
}

void SettingsScreen::drawTeam(const Rectangle& area) const {
    UiText::draw(tr(u8"Nhóm phát triển - THE FIRST STEP", "Development team - THE FIRST STEP"),
                 static_cast<int>(area.x + 24), static_cast<int>(area.y + 8), 22, COL_TEXT);
    const float rowH = 76.0f;
    float y = area.y + 48.0f;
    for (const Member& m : MEMBERS) {
        const Rectangle r{ area.x + 14.0f, y, area.width - 28.0f, rowH - 6.0f };
        UiShapes::plate(r, Color{ 24, 20, 36, 220 }, UiTheme::ash, 1.0f);
        UiText::draw(m.m_name, static_cast<int>(r.x + 14), static_cast<int>(r.y + 6), 19, COL_GOLD);
        const int idW = UiText::measure(m.m_id, 15);
        UiText::draw(m.m_id, static_cast<int>(r.x + r.width - idW - 14), static_cast<int>(r.y + 9), 15, COL_DIM);
        const int nameW = UiText::measure(m.m_name, 19);
        UiText::draw(tr(m.m_roleVi, m.m_roleEn), static_cast<int>(r.x + 14 + nameW + 18), static_cast<int>(r.y + 9), 15, COL_TEXT);
        drawWrapped(tr(m.m_workVi, m.m_workEn), static_cast<int>(r.x + 14), static_cast<int>(r.y + 36),
                    static_cast<int>(r.width - 28), 14, 18, COL_DIM);
        y += rowH;
    }
}
