// src/graphics/PrepMenuScreen.cpp
#include "PrepMenuScreen.h"
#include "SkillTooltip.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float TAB_W = 250.0f, TAB_H = 46.0f, TAB_GAP = 12.0f, TAB_Y = 92.0f;
constexpr float PANEL_Y = 138.0f, PANEL_H = 482.0f, PANEL_W = 1036.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr size_t MAX_SKILL_ROWS = 6;
constexpr int    ITEM_SLOTS = 8;

// ---- Mau (dong bo voi WaveSelectScreen) ----
constexpr Color COL_TITLE    { 255, 214, 96, 255 };
constexpr Color COL_PANEL    { 16, 13, 36, 225 };
constexpr Color COL_EDGE     { 140, 120, 180, 255 };
constexpr Color COL_EDGE_SEL { 255, 214, 96, 255 };
constexpr Color COL_LABEL    { 190, 185, 220, 255 };
constexpr Color COL_TEXT     { 238, 235, 252, 255 };
constexpr Color COL_DIM      { 130, 126, 160, 255 };
constexpr Color COL_ERR      { 255, 110, 110, 255 };
constexpr Color COL_OK       { 120, 230, 140, 255 };
constexpr Color COL_BOSS     { 200, 50, 60, 255 };

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return Color{ 215, 215, 230, 255 };
        case Element::MOC:  return Color{ 100, 210, 100, 255 };
        case Element::THUY: return Color{ 90, 160, 255, 255 };
        case Element::HOA:  return Color{ 255, 110, 80, 255 };
        case Element::THO:  return Color{ 205, 160, 90, 255 };
    }
    return WHITE;
}
const char* elementName(Element e) {
    switch (e) {
        case Element::KIM:  return "Kim";
        case Element::MOC:  return "Moc";
        case Element::THUY: return "Thuy";
        case Element::HOA:  return "Hoa";
        case Element::THO:  return "Tho";
    }
    return "?";
}
SkillTipInfo toTipInfo(const PrepSkillInfo& s) {
    SkillTipInfo t;
    t.m_name = s.m_name;
    t.m_element = s.m_element;
    t.m_typeText = s.m_typeText;
    t.m_power = s.m_power;
    t.m_mpCost = s.m_mpCost;
    t.m_effectText = s.m_effectText;
    t.m_isMagic = s.m_isMagic;
    t.m_isHeal = (s.m_typeText.find("Hoi") != std::string::npos);   // "Hoi mau"
    return t;
}
const char* tabLabel(int i) {
    static const char* labels[] = { "DOI HINH", "KY NANG", "TUI DO", "THONG TIN CONG" };
    return labels[i];
}

void drawCenteredText(const char* text, float cx, float y, int size, Color c) {
    DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
}
void drawRightText(const char* text, float right, float y, int size, Color c) {
    DrawText(text, static_cast<int>(right - MeasureText(text, size)), static_cast<int>(y), size, c);
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        const size_t nl = s.find('\n', start);
        out.push_back(s.substr(start, nl == std::string::npos ? std::string::npos : nl - start));
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return out;
}

void drawStatRow(const char* label, int value, float x, float y, float w) {
    DrawText(label, static_cast<int>(x), static_cast<int>(y), 18, COL_LABEL);
    drawRightText(TextFormat("%d", value), x + w, y, 18, COL_TEXT);
}
} // namespace

PrepMenuScreen::PrepMenuScreen() = default;
PrepMenuScreen::~PrepMenuScreen() { unloadAll(); }

// ============ Tai nguyen ============
bool PrepMenuScreen::loadBackground(const std::string& path) {
    unloadAll();
    if (path.empty() || !FileExists(path.c_str())) return false;
    m_background = LoadTexture(path.c_str());
    m_hasBackground = (m_background.id != 0);
    if (!m_hasBackground)
        TraceLog(LOG_WARNING, "PrepMenuScreen: khong doc duoc anh nen '%s'.", path.c_str());
    else
        SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);
    return m_hasBackground;
}

void PrepMenuScreen::unloadAll() {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

// ============ Du lieu ============
void PrepMenuScreen::setInfo(const PrepInfo& info) { m_info = info; }

void PrepMenuScreen::reset(int tab) {
    m_tab = std::max(0, std::min(tab, static_cast<int>(TAB_COUNT) - 1));
    m_hoverTab = -1;
    m_hoverButton = -1;
}

// ============ Bo cuc ============
Rectangle PrepMenuScreen::tabRect(int index) const {
    const float W = static_cast<float>(GetScreenWidth());
    const float total = TAB_COUNT * TAB_W + (TAB_COUNT - 1) * TAB_GAP;
    return { (W - total) / 2.0f + static_cast<float>(index) * (TAB_W + TAB_GAP), TAB_Y, TAB_W, TAB_H };
}

Rectangle PrepMenuScreen::contentRect() const {
    const float W = static_cast<float>(GetScreenWidth());
    return { (W - PANEL_W) / 2.0f, PANEL_Y, PANEL_W, PANEL_H };
}

Rectangle PrepMenuScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(GetScreenWidth()) - total) / 2.0f;
    return { startX + index * (BTN_W + BTN_GAP), BTN_Y, BTN_W, BTN_H };
}

// ============ Input ============
void PrepMenuScreen::selectTab(int delta) {
    m_tab = (m_tab + delta + TAB_COUNT) % TAB_COUNT;   // quay vong
}

PrepMenuAction PrepMenuScreen::handleInput() {
    // ---- Ban phim ----
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_Q)) selectTab(-1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_TAB)) selectTab(+1);
    if (IsKeyPressed(KEY_ONE))   m_tab = TAB_PARTY;
    if (IsKeyPressed(KEY_TWO))   m_tab = TAB_SKILLS;
    if (IsKeyPressed(KEY_THREE)) m_tab = TAB_ITEMS;
    if (IsKeyPressed(KEY_FOUR))  m_tab = TAB_GATE;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) return PrepMenuAction::START;

    // ---- Chuot ----
    const Vector2 mouse = GetMousePosition();
    m_hoverTab = -1;
    for (int i = 0; i < TAB_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, tabRect(i))) m_hoverTab = i;
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (m_hoverTab >= 0) m_tab = m_hoverTab;
        if (m_hoverButton == BTN_BACK)  return PrepMenuAction::BACK;
        if (m_hoverButton == BTN_START) return PrepMenuAction::START;
    }
    return PrepMenuAction::NONE;
}

void PrepMenuScreen::update(float deltaTime) { m_time += deltaTime; }

// ============ Ve ============
void PrepMenuScreen::draw() const {
    m_hoverSkill = -1;
    drawBackground();
    const float cx = GetScreenWidth() / 2.0f;
    drawCenteredText("CHUAN BI TRAN DAU", cx, 26.0f, 40, COL_TITLE);
    if (!m_info.m_gate.m_title.empty())
        drawCenteredText(TextFormat("%s  -  Anh hung: %s", m_info.m_gate.m_title.c_str(), m_info.m_hero.m_name.c_str()),
                         cx, 66.0f, 16, COL_LABEL);

    drawTabs();

    const Rectangle area = contentRect();
    DrawRectangleRec(Rectangle{ area.x + 5, area.y + 6, area.width, area.height }, Color{ 0, 0, 0, 120 });
    DrawRectangleRec(area, COL_PANEL);
    DrawRectangleLinesEx(area, 2.0f, COL_EDGE);

    switch (m_tab) {
        case TAB_PARTY:  drawPartyTab(area);  break;
        case TAB_SKILLS: drawSkillsTab(area); break;
        case TAB_ITEMS:  drawItemsTab(area);  break;
        case TAB_GATE:   drawGateTab(area);   break;
        default: break;
    }

    drawButtons();
    drawCenteredText("[Q/E hoac LEFT/RIGHT] Doi tab   [1-4] Chon tab   [ENTER] Vao tran   [ESC] Quay lai",
                     cx, 702.0f, 16, Color{ 200, 195, 225, 220 });

    // Tooltip ky nang: ve CUOI CUNG de nam tren moi thu
    if (m_tab == TAB_SKILLS && m_hoverSkill >= 0 &&
        static_cast<size_t>(m_hoverSkill) < m_info.m_skills.size()) {
        SkillTooltip::draw(toTipInfo(m_info.m_skills[static_cast<size_t>(m_hoverSkill)]),
                           GetMousePosition(), &m_info.m_hero.m_stats);
    }
}

void PrepMenuScreen::drawBackground() const {
    const int W = GetScreenWidth(), H = GetScreenHeight();
    if (m_hasBackground) {
        DrawTexturePro(m_background, Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, static_cast<float>(W), static_cast<float>(H) }, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 10, 8, 36, 255 }, Color{ 60, 30, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 160 });
}

void PrepMenuScreen::drawTabs() const {
    for (int i = 0; i < TAB_COUNT; ++i) {
        const Rectangle r = tabRect(i);
        const bool active = (i == m_tab);
        const bool hover = (i == m_hoverTab);
        DrawRectangleRec(r, active ? Color{ 60, 46, 18, 240 } : (hover ? Color{ 40, 34, 70, 235 } : Color{ 24, 20, 50, 225 }));
        DrawRectangleLinesEx(r, active ? 3.0f : 2.0f, active ? COL_EDGE_SEL : COL_EDGE);
        drawCenteredText(TextFormat("%d. %s", i + 1, tabLabel(i)), r.x + r.width / 2.0f, r.y + (r.height - 20.0f) / 2.0f, 20,
                         active ? COL_TITLE : (hover ? COL_TEXT : COL_LABEL));
    }
}

// ---------------- Tab 1: DOI HINH ----------------
void PrepMenuScreen::drawPartyTab(const Rectangle& area) const {
    const int slots = 1 + std::max(0, m_info.m_teammateSlots);
    const float gap = 24.0f, pad = 24.0f;
    const float cardW = (area.width - 2.0f * pad - (slots - 1) * gap) / static_cast<float>(slots);
    const float cardH = area.height - 2.0f * pad;

    for (int i = 0; i < slots; ++i) {
        const Rectangle r = { area.x + pad + i * (cardW + gap), area.y + pad, cardW, cardH };
        if (i == 0) {
            const PrepHeroInfo& h = m_info.m_hero;
            DrawRectangleRec(r, Color{ 28, 24, 58, 235 });
            DrawRectangleRec(Rectangle{ r.x, r.y, r.width, 54.0f }, Color{ 40, 62, 128, 255 });
            drawCenteredText(h.m_name.c_str(), r.x + r.width / 2.0f, r.y + 14.0f, 28, COL_TEXT);

            const float cx = r.x + r.width / 2.0f;
            drawCenteredText(TextFormat("%s %s", h.m_race.c_str(), h.m_class.c_str()), cx, r.y + 68.0f, 18, COL_LABEL);
            DrawCircle(static_cast<int>(cx - 30), static_cast<int>(r.y + 106), 7.0f, elementColor(h.m_element));
            DrawText(TextFormat("He %s", elementName(h.m_element)), static_cast<int>(cx - 16), static_cast<int>(r.y + 97), 18, elementColor(h.m_element));

            float y = r.y + 140.0f;
            const float x = r.x + 28.0f, w = r.width - 56.0f;
            drawStatRow("HP",   h.m_stats.m_hp,  x, y, w); y += 30.0f;
            drawStatRow("MP",   h.m_stats.m_mp,  x, y, w); y += 30.0f;
            drawStatRow("ATK",  h.m_stats.m_a,   x, y, w); y += 30.0f;
            drawStatRow("MATK", h.m_stats.m_ma,  x, y, w); y += 30.0f;
            drawStatRow("DEF",  h.m_stats.m_d,   x, y, w); y += 30.0f;
            drawStatRow("MDEF", h.m_stats.m_md,  x, y, w); y += 30.0f;
            drawStatRow("SPD",  h.m_stats.m_spd, x, y, w);

            DrawRectangleLinesEx(r, 3.0f, COL_EDGE_SEL);
            drawCenteredText("ANH HUNG", cx, r.y + r.height - 34.0f, 16, COL_OK);
        } else {
            DrawRectangleRec(r, Color{ 20, 18, 40, 200 });
            DrawRectangleLinesEx(r, 2.0f, COL_DIM);
            const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f;
            DrawRectangle(static_cast<int>(cx - 3), static_cast<int>(cy - 28), 6, 56, COL_DIM);     // dau +
            DrawRectangle(static_cast<int>(cx - 28), static_cast<int>(cy - 3), 56, 6, COL_DIM);
            drawCenteredText(TextFormat("DONG DOI %d", i), cx, cy + 46.0f, 20, COL_LABEL);
            drawCenteredText("Chua gia nhap", cx, cy + 74.0f, 16, COL_DIM);
        }
    }
}

// ---------------- Tab 2: KY NANG ----------------
void PrepMenuScreen::drawSkillsTab(const Rectangle& area) const {
    DrawText("KY NANG CUA ANH HUNG", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
    drawRightText("MP", area.x + area.width - 250.0f, area.y + 24.0f, 14, COL_DIM);
    drawRightText("SAT THUONG", area.x + area.width - 120.0f, area.y + 24.0f, 14, COL_DIM);

    if (m_info.m_skills.empty()) {
        drawCenteredText("Chua co ky nang", area.x + area.width / 2.0f, area.y + area.height / 2.0f, 22, COL_DIM);
        return;
    }

    const float rowH = 64.0f, rowGap = 10.0f;
    float y = area.y + 56.0f;
    for (size_t i = 0; i < m_info.m_skills.size() && i < MAX_SKILL_ROWS; ++i) {
        const PrepSkillInfo& s = m_info.m_skills[i];
        const Rectangle r = { area.x + 24.0f, y, area.width - 48.0f, rowH };
        const bool rowHover = CheckCollisionPointRec(GetMousePosition(), r);
        if (rowHover) m_hoverSkill = static_cast<int>(i);
        DrawRectangleRec(r, rowHover ? Color{ 40, 34, 80, 245 } : Color{ 28, 24, 58, 235 });
        DrawRectangleLinesEx(r, 2.0f, rowHover ? COL_EDGE_SEL : COL_EDGE);
        DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y), 6, static_cast<int>(r.height), elementColor(s.m_element));

        DrawText(s.m_name.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 10), 22, COL_TEXT);
        std::string sub = s.m_typeText;
        if (!s.m_effectText.empty()) sub += "  |  " + s.m_effectText;
        DrawText(sub.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 38), 16, COL_LABEL);

        DrawText(elementName(s.m_element), static_cast<int>(r.x + r.width - 410), static_cast<int>(r.y + 20), 18, elementColor(s.m_element));
        drawRightText(TextFormat("%d", s.m_mpCost), r.x + r.width - 226.0f, r.y + 20.0f, 20, Color{ 110, 170, 255, 255 });
        drawRightText(s.m_power > 0 ? TextFormat("%d%%", s.m_power) : "-", r.x + r.width - 24.0f, r.y + 20.0f, 20, COL_TEXT);
        y += rowH + rowGap;
    }
}

// ---------------- Tab 3: TUI DO ----------------
void PrepMenuScreen::drawItemsTab(const Rectangle& area) const {
    DrawText("TUI DO MANG VAO TRAN", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);

    const int cols = 4;
    const float pad = 28.0f, gap = 18.0f, top = 64.0f;
    const float slotW = (area.width - 2.0f * pad - (cols - 1) * gap) / cols;
    const float slotH = 150.0f;

    for (int i = 0; i < ITEM_SLOTS; ++i) {
        const int col = i % cols, row = i / cols;
        const Rectangle r = { area.x + pad + col * (slotW + gap), area.y + top + row * (slotH + gap), slotW, slotH };
        const bool filled = static_cast<size_t>(i) < m_info.m_items.size();
        DrawRectangleRec(r, filled ? Color{ 28, 24, 58, 235 } : Color{ 20, 18, 40, 170 });
        DrawRectangleLinesEx(r, 2.0f, filled ? COL_EDGE : Color{ 70, 66, 96, 255 });
        if (filled) {
            const PrepItemInfo& it = m_info.m_items[static_cast<size_t>(i)];
            const float cx = r.x + r.width / 2.0f;
            DrawCircle(static_cast<int>(cx), static_cast<int>(r.y + 52), 24.0f, Color{ 220, 80, 100, 255 });   // bieu tuong tam
            DrawRectangle(static_cast<int>(cx - 7), static_cast<int>(r.y + 22), 14, 12, Color{ 200, 190, 220, 255 });
            drawCenteredText(it.m_name.c_str(), cx, r.y + 92.0f, 20, COL_TEXT);
            drawCenteredText(TextFormat("x%d", it.m_count), cx, r.y + 118.0f, 18, COL_OK);
        } else {
            drawCenteredText("Trong", r.x + r.width / 2.0f, r.y + r.height / 2.0f - 9.0f, 18, Color{ 70, 66, 96, 255 });
        }
    }
}

// ---------------- Tab 4: THONG TIN CONG ----------------
void PrepMenuScreen::drawGateTab(const Rectangle& area) const {
    const PrepGateInfo& g = m_info.m_gate;
    const float pad = 28.0f;
    const float leftX = area.x + pad, leftW = 430.0f;
    const float rightX = area.x + pad + leftW + 36.0f, rightW = area.x + area.width - pad - rightX;

    // ---- Cot trai: cong + thoi han ----
    DrawText(g.m_title.empty() ? "CONG" : g.m_title.c_str(), static_cast<int>(leftX), static_cast<int>(area.y + 22), 34, COL_TITLE);
    float y = area.y + 72.0f;
    for (const std::string& line : splitLines(g.m_description)) {
        DrawText(line.c_str(), static_cast<int>(leftX), static_cast<int>(y), 18, COL_LABEL);
        y += 26.0f;
    }

    // Thoi han (dong ho cong)
    const Rectangle clockBox = { leftX, area.y + 170.0f, leftW, 120.0f };
    const bool hasDeadline = g.m_deadlineTurns >= 0;
    DrawRectangleRec(clockBox, Color{ 28, 24, 58, 235 });
    DrawRectangleLinesEx(clockBox, 2.0f, hasDeadline ? COL_EDGE_SEL : COL_EDGE);
    DrawText("THOI HAN", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 16), 16, COL_DIM);
    if (hasDeadline) {
        DrawText(TextFormat("%d", g.m_deadlineTurns), static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 44), 56, COL_TITLE);
        DrawText("luot de ha gac cong", static_cast<int>(clockBox.x + 100 + (g.m_deadlineTurns >= 100 ? 36 : 0)),
                 static_cast<int>(clockBox.y + 66), 18, COL_TEXT);
    } else {
        DrawText("Khong gioi han", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 52), 30, COL_OK);
    }

    // Goi y khac he
    const Rectangle hintBox = { leftX, area.y + 310.0f, leftW, 150.0f };
    DrawRectangleRec(hintBox, Color{ 28, 24, 58, 235 });
    DrawRectangleLinesEx(hintBox, 2.0f, COL_EDGE);
    DrawText("VONG KHAC HE", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 14), 16, COL_DIM);
    const Element ring[] = { Element::KIM, Element::MOC, Element::THO, Element::THUY, Element::HOA };
    float hx = hintBox.x + 20.0f;
    for (int i = 0; i < 5; ++i) {
        DrawCircle(static_cast<int>(hx + 8), static_cast<int>(hintBox.y + 58), 8.0f, elementColor(ring[i]));
        DrawText(elementName(ring[i]), static_cast<int>(hx + 20), static_cast<int>(hintBox.y + 49), 16, elementColor(ring[i]));
        hx += 78.0f;
    }
    DrawText("Khac he: x1.5 sat thuong", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 84), 16, COL_OK);
    DrawText("Bi khac he: x0.5 sat thuong", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 108), 16, COL_ERR);
    DrawText("(Kim > Moc > Tho > Thuy > Hoa > Kim)", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 128), 14, COL_DIM);

    // ---- Cot phai: boss + quai ----
    float ry = area.y + 22.0f;
    if (g.m_hasBoss) {
        const Rectangle boss = { rightX, ry, rightW, 130.0f };
        DrawRectangleRec(boss, Color{ 60, 20, 30, 235 });
        DrawRectangleLinesEx(boss, 3.0f, COL_BOSS);
        DrawRectangleRec(Rectangle{ boss.x + 18, boss.y + 14, 68, 22 }, COL_BOSS);
        drawCenteredText("BOSS", boss.x + 18 + 34, boss.y + 17, 16, WHITE);
        DrawText(g.m_bossName.c_str(), static_cast<int>(boss.x + 100), static_cast<int>(boss.y + 12), 28, COL_TEXT);
        DrawCircle(static_cast<int>(boss.x + 30), static_cast<int>(boss.y + 76), 8.0f, elementColor(g.m_bossElement));
        DrawText(TextFormat("He %s", elementName(g.m_bossElement)), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 66), 18, elementColor(g.m_bossElement));
        DrawText(TextFormat("HP %d", g.m_bossHp), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 94), 18, COL_TEXT);
        ry += 130.0f + 20.0f;
    }

    DrawText(g.m_hasBoss ? "QUAN HO TONG" : "QUAI CO THE GAP", static_cast<int>(rightX), static_cast<int>(ry), 16, COL_TITLE);
    ry += 28.0f;
    size_t shown = 0;
    for (const PrepEnemyInfo& e : g.m_enemies) {
        if (ry + 26.0f > area.y + area.height - 16.0f) break;
        DrawCircle(static_cast<int>(rightX + 10), static_cast<int>(ry + 11), 6.0f, elementColor(e.m_element));
        DrawText(e.m_name.c_str(), static_cast<int>(rightX + 28), static_cast<int>(ry), 20, COL_TEXT);
        drawRightText(elementName(e.m_element), rightX + rightW, ry + 3.0f, 16, elementColor(e.m_element));
        ry += 30.0f;
        ++shown;
    }
    if (g.m_enemies.empty())
        DrawText("Khong co du lieu", static_cast<int>(rightX), static_cast<int>(ry), 18, COL_DIM);
    else if (shown < g.m_enemies.size())
        DrawText(TextFormat("+%d quai khac", static_cast<int>(g.m_enemies.size() - shown)), static_cast<int>(rightX + 28), static_cast<int>(ry), 14, COL_DIM);
}

void PrepMenuScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { "QUAY LAI", "VAO TRAN" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        DrawRectangleRec(Rectangle{ r.x + 4, r.y + 5, r.width, r.height }, Color{ 0, 0, 0, 110 });
        DrawRectangleRec(r, hover ? Color{ 90, 60, 20, 235 } : (primary ? Color{ 50, 40, 14, 230 } : COL_PANEL));
        DrawRectangleLinesEx(r, hover ? 3.0f : 2.0f, (hover || primary) ? COL_EDGE_SEL : COL_EDGE);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         hover ? Color{ 255, 240, 170, 255 } : COL_TEXT);
    }
}