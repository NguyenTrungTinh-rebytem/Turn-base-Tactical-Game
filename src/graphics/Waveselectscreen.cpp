// Member 4 - Huynh Quang Khai | src/graphics/WaveSelectScreen.cpp
#include "WaveSelectScreen.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float CARD_W = 340.0f, CARD_H = 410.0f, CARD_GAP = 40.0f, CARD_Y = 125.0f, SIDE_MARGIN = 40.0f;
constexpr float HEADER_H = 64.0f, SELECT_LIFT = 8.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr size_t MAX_ENEMY_LINES = 6;

// ---- Mau ----
constexpr Color COL_TITLE      { 255, 214, 96, 255 };
constexpr Color COL_PANEL      { 16, 13, 36, 225 };
constexpr Color COL_EDGE       { 140, 120, 180, 255 };
constexpr Color COL_EDGE_SEL   { 255, 214, 96, 255 };
constexpr Color COL_LABEL      { 190, 185, 220, 255 };
constexpr Color COL_TEXT       { 238, 235, 252, 255 };
constexpr Color COL_DIM        { 130, 126, 160, 255 };
constexpr Color COL_ERR        { 255, 110, 110, 255 };
constexpr Color COL_OK         { 120, 230, 140, 255 };
constexpr Color COL_HEAD_NORMAL{ 40, 62, 128, 255 };
constexpr Color COL_HEAD_BOSS  { 130, 32, 44, 255 };

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

void drawCenteredText(const char* text, float cx, float y, int size, Color c) {
    DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
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
} // namespace

WaveSelectScreen::WaveSelectScreen() = default;
WaveSelectScreen::~WaveSelectScreen() { unloadAll(); }

// ============ Tai nguyen ============
bool WaveSelectScreen::loadBackground(const std::string& path) {
    unloadAll();
    if (path.empty() || !FileExists(path.c_str())) return false;
    m_background = LoadTexture(path.c_str());
    m_hasBackground = (m_background.id != 0);
    if (!m_hasBackground)
        TraceLog(LOG_WARNING, "WaveSelectScreen: khong doc duoc anh nen '%s' (phai la PNG/JPG that).", path.c_str());
    else
        SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);
    return m_hasBackground;
}

void WaveSelectScreen::unloadAll() {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

// ============ Du lieu ============
void WaveSelectScreen::setWaves(const std::vector<WaveInfo>& waves) {
    m_waves = waves;
    m_selected = std::min(m_selected, m_waves.empty() ? size_t{ 0 } : m_waves.size() - 1);
}

void WaveSelectScreen::reset(size_t selectedIndex) {
    m_selected = std::min(selectedIndex, m_waves.empty() ? size_t{ 0 } : m_waves.size() - 1);
    m_hoverButton = -1;
    m_message.clear();
    m_messageTimer = 0.0f;
}

int WaveSelectScreen::getSelectedWave() const {
    return m_waves.empty() ? 0 : m_waves[m_selected].m_number;
}

bool WaveSelectScreen::selectedLocked() const {
    return m_waves.empty() || m_waves[m_selected].m_locked;
}

// ============ Bo cuc ============
Rectangle WaveSelectScreen::cardRect(size_t index) const {
    const float W = static_cast<float>(GetScreenWidth());
    const float n = static_cast<float>(std::max<size_t>(1, m_waves.size()));
    float w = CARD_W;
    if (n * w + (n - 1.0f) * CARD_GAP > W - 2.0f * SIDE_MARGIN)      // nhieu wave -> the hep lai
        w = (W - 2.0f * SIDE_MARGIN - (n - 1.0f) * CARD_GAP) / n;
    const float total = n * w + (n - 1.0f) * CARD_GAP;
    return { (W - total) / 2.0f + static_cast<float>(index) * (w + CARD_GAP), CARD_Y, w, CARD_H };
}

Rectangle WaveSelectScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(GetScreenWidth()) - total) / 2.0f;
    return { startX + index * (BTN_W + BTN_GAP), BTN_Y, BTN_W, BTN_H };
}

// ============ Input ============
void WaveSelectScreen::select(int delta) {
    if (m_waves.empty()) return;
    const int next = static_cast<int>(m_selected) + delta;
    m_selected = static_cast<size_t>(std::max(0, std::min(next, static_cast<int>(m_waves.size()) - 1)));
}

WaveSelectAction WaveSelectScreen::tryStart() {
    if (m_waves.empty()) return WaveSelectAction::NONE;
    if (selectedLocked()) {
        m_message = "Wave nay chua mo khoa! Hay hoan thanh wave truoc do.";
        m_messageTimer = 2.0f;
        return WaveSelectAction::NONE;
    }
    return WaveSelectAction::START;
}

WaveSelectAction WaveSelectScreen::handleInput() {
    // ---- Ban phim ----
    if (!m_waves.empty()) {
        if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) select(-1);
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) select(+1);
        const int digitKeys[] = { KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE };
        for (size_t i = 0; i < 5 && i < m_waves.size(); ++i)
            if (IsKeyPressed(digitKeys[i])) m_selected = i;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
            const WaveSelectAction a = tryStart();
            if (a != WaveSelectAction::NONE) return a;
        }
    }

    // ---- Chuot ----
    const Vector2 mouse = GetMousePosition();
    const Vector2 moved = GetMouseDelta();
    const bool mouseMoved = (moved.x != 0.0f || moved.y != 0.0f);

    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    int cardUnderMouse = -1;
    for (size_t i = 0; i < m_waves.size(); ++i)
        if (CheckCollisionPointRec(mouse, cardRect(i))) cardUnderMouse = static_cast<int>(i);
    if (cardUnderMouse >= 0 && mouseMoved) m_selected = static_cast<size_t>(cardUnderMouse);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (cardUnderMouse >= 0) {
            const bool alreadySelected = (static_cast<size_t>(cardUnderMouse) == m_selected);
            m_selected = static_cast<size_t>(cardUnderMouse);
            if (alreadySelected) {                       // bam lan 2 vao the dang chon = bat dau
                const WaveSelectAction a = tryStart();
                if (a != WaveSelectAction::NONE) return a;
            }
        }
        if (m_hoverButton == BTN_BACK) return WaveSelectAction::BACK;
        if (m_hoverButton == BTN_START) {
            const WaveSelectAction a = tryStart();
            if (a != WaveSelectAction::NONE) return a;
        }
    }
    return WaveSelectAction::NONE;
}

void WaveSelectScreen::update(float deltaTime) {
    m_time += deltaTime;
    if (m_messageTimer > 0.0f) m_messageTimer = std::max(0.0f, m_messageTimer - deltaTime);
}

// ============ Ve ============
void WaveSelectScreen::draw() const {
    drawBackground();
    const float cx = GetScreenWidth() / 2.0f;
    drawCenteredText("CHON WAVE", cx, 30.0f, 44, COL_TITLE);
    if (!m_heroSummary.empty())
        drawCenteredText(TextFormat("Anh hung: %s", m_heroSummary.c_str()), cx, 86.0f, 18, COL_LABEL);

    if (m_waves.empty()) {
        drawCenteredText("Khong co du lieu wave (kiem tra thu muc data/)", cx, 320.0f, 22, COL_ERR);
    } else {
        for (size_t i = 0; i < m_waves.size(); ++i)
            if (i != m_selected) drawCard(i);
        drawCard(m_selected);   // ve the dang chon sau cung de nam tren
    }

    drawButtons();
    if (m_messageTimer > 0.0f) drawCenteredText(m_message.c_str(), cx, 612.0f, 20, COL_ERR);
    drawCenteredText("[LEFT/RIGHT] Chon wave   [ENTER] Bat dau   [ESC] Quay lai", cx, 702.0f, 16, Color{ 200, 195, 225, 220 });
}

void WaveSelectScreen::drawBackground() const {
    const int W = GetScreenWidth(), H = GetScreenHeight();
    if (m_hasBackground) {
        DrawTexturePro(m_background, Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, static_cast<float>(W), static_cast<float>(H) }, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 10, 8, 36, 255 }, Color{ 60, 30, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 160 });
}

void WaveSelectScreen::drawCard(size_t index) const {
    const WaveInfo& w = m_waves[index];
    const bool selected = (index == m_selected);
    Rectangle r = cardRect(index);
    if (selected) r.y -= SELECT_LIFT;

    DrawRectangleRec(Rectangle{ r.x + 5, r.y + 6, r.width, r.height }, Color{ 0, 0, 0, 120 });   // bong
    DrawRectangleRec(r, COL_PANEL);

    // ---- Dai tieu de ----
    DrawRectangleRec(Rectangle{ r.x, r.y, r.width, HEADER_H }, w.m_isBoss ? COL_HEAD_BOSS : COL_HEAD_NORMAL);
    drawCenteredText(w.m_title.c_str(), r.x + r.width / 2.0f, r.y + 16.0f, 32, COL_TEXT);
    if (w.m_isBoss) {
        const Rectangle tag = { r.x + r.width / 2.0f - 34.0f, r.y + HEADER_H - 11.0f, 68.0f, 22.0f };
        DrawRectangleRec(tag, Color{ 200, 50, 60, 255 });
        DrawRectangleLinesEx(tag, 1.0f, Color{ 255, 150, 150, 255 });
        drawCenteredText("BOSS", tag.x + tag.width / 2.0f, tag.y + 3.0f, 16, WHITE);
    }

    // ---- Mo ta ----
    float y = r.y + HEADER_H + 14.0f;
    for (const std::string& line : splitLines(w.m_description)) {
        drawCenteredText(line.c_str(), r.x + r.width / 2.0f, y, 16, COL_LABEL);
        y += 22.0f;
    }

    // ---- Danh sach quai ----
    y = r.y + 150.0f;
    DrawText("QUAI CO THE GAP", static_cast<int>(r.x + 20), static_cast<int>(y), 15, COL_TITLE);
    y += 28.0f;
    for (size_t i = 0; i < w.m_enemies.size() && i < MAX_ENEMY_LINES; ++i) {
        const WaveEnemy& e = w.m_enemies[i];
        DrawCircle(static_cast<int>(r.x + 30), static_cast<int>(y + 10), 6.0f, elementColor(e.m_element));
        DrawText(e.m_name.c_str(), static_cast<int>(r.x + 46), static_cast<int>(y), 18, COL_TEXT);
        const char* el = elementName(e.m_element);
        DrawText(el, static_cast<int>(r.x + r.width - 20 - MeasureText(el, 14)), static_cast<int>(y + 3), 14, elementColor(e.m_element));
        y += 28.0f;
    }
    if (w.m_enemies.size() > MAX_ENEMY_LINES)
        DrawText(TextFormat("+%d quai khac", static_cast<int>(w.m_enemies.size() - MAX_ENEMY_LINES)),
                 static_cast<int>(r.x + 46), static_cast<int>(y), 14, COL_DIM);

    // ---- Trang thai ----
    if (w.m_cleared && !w.m_locked)
        drawCenteredText("DA HOAN THANH", r.x + r.width / 2.0f, r.y + r.height - 38.0f, 18, COL_OK);

    // ---- Vien (vang khi duoc chon) ----
    DrawRectangleLinesEx(r, selected ? 4.0f : 2.0f, selected ? COL_EDGE_SEL : COL_EDGE);

    // ---- Khoa: phu toi + o khoa ----
    if (w.m_locked) {
        DrawRectangleRec(r, Color{ 0, 0, 0, 175 });
        const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f - 10.0f;
        DrawRectangleLinesEx(Rectangle{ cx - 14, cy - 34, 28, 30 }, 5.0f, Color{ 200, 170, 80, 255 });   // quai khoa
        DrawRectangleRec(Rectangle{ cx - 24, cy - 8, 48, 36 }, Color{ 200, 170, 80, 255 });              // than khoa
        DrawRectangle(static_cast<int>(cx - 4), static_cast<int>(cy + 4), 8, 12, Color{ 60, 44, 20, 255 });
        const std::string hint = w.m_lockHint.empty()
            ? std::string(TextFormat("Hoan thanh Wave %d\nde mo khoa", std::max(1, w.m_number - 1)))
            : w.m_lockHint;
        float hy = cy + 44.0f;
        for (const std::string& line : splitLines(hint)) {
            drawCenteredText(line.c_str(), cx, hy, 16, COL_TEXT);
            hy += 22.0f;
        }
        DrawRectangleLinesEx(r, selected ? 4.0f : 2.0f, selected ? COL_EDGE_SEL : COL_EDGE);
    }
}

void WaveSelectScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { "QUAY LAI", "BAT DAU" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const bool disabled = primary && selectedLocked();
        DrawRectangleRec(Rectangle{ r.x + 4, r.y + 5, r.width, r.height }, Color{ 0, 0, 0, 110 });
        DrawRectangleRec(r, disabled ? Color{ 24, 22, 34, 220 } : (hover ? Color{ 90, 60, 20, 235 } : (primary ? Color{ 50, 40, 14, 230 } : COL_PANEL)));
        DrawRectangleLinesEx(r, hover && !disabled ? 3.0f : 2.0f, disabled ? Color{ 80, 76, 100, 255 } : ((hover || primary) ? COL_EDGE_SEL : COL_EDGE));
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         disabled ? COL_DIM : (hover ? Color{ 255, 240, 170, 255 } : COL_TEXT));
    }
}