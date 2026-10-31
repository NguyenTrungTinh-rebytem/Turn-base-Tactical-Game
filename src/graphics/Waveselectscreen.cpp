#include "graphics/ui/UiInput.h"
#include "graphics/ui/UiFrame.h"
#include "graphics/ui/UiWidgets.h"
#include "graphics/ui/UiTheme.h"
#include "graphics/ui/UiText.h"
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
constexpr Color COL_TITLE = UiTheme::AGED_GOLD;
constexpr Color COL_PANEL      { 28, 25, 40, 255 };
constexpr Color COL_EDGE = UiTheme::SLATE;
constexpr Color COL_EDGE_SEL = UiTheme::AGED_GOLD;
constexpr Color COL_LABEL = UiTheme::MUTED;
constexpr Color COL_TEXT = UiTheme::TEXT;
constexpr Color COL_DIM = UiTheme::DISABLED;
constexpr Color COL_ERR = UiTheme::DANGER;
constexpr Color COL_OK = UiTheme::HP;
constexpr Color COL_HEAD_NORMAL{ 48, 84, 105, 255 };
constexpr Color COL_HEAD_BOSS  { 112, 54, 61, 255 };

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return UiTheme::elementColor(Element::KIM);
        case Element::MOC:  return UiTheme::elementColor(Element::MOC);
        case Element::THUY: return UiTheme::elementColor(Element::THUY);
        case Element::HOA:  return UiTheme::elementColor(Element::HOA);
        case Element::THO:  return UiTheme::elementColor(Element::THO);
    }
    return WHITE;
}
const char* elementName(Element e) {
    switch (e) {
        case Element::KIM:  return "Kim";
        case Element::MOC:  return "Mộc";
        case Element::THUY: return "Thủy";
        case Element::HOA:  return "Hỏa";
        case Element::THO:  return "Thổ";
    }
    return "?";
}

void drawCenteredText(const char* text, float cx, float y, int size, Color c) {
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size) / 2.0f), static_cast<int>(y), size, c);
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
    const float W = static_cast<float>(UiTheme::WIDTH);
    const float n = static_cast<float>(std::max<size_t>(1, m_waves.size()));
    float w = CARD_W;
    if (n * w + (n - 1.0f) * CARD_GAP > W - 2.0f * SIDE_MARGIN)      // nhieu wave -> the hep lai
        w = (W - 2.0f * SIDE_MARGIN - (n - 1.0f) * CARD_GAP) / n;
    const float total = n * w + (n - 1.0f) * CARD_GAP;
    return { (W - total) / 2.0f + static_cast<float>(index) * (w + CARD_GAP), CARD_Y, w, CARD_H };
}

Rectangle WaveSelectScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(UiTheme::WIDTH) - total) / 2.0f;
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
        m_message = "Ải này chưa mở khóa! Hãy hoàn thành ải trước đó.";
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
    const Vector2 mouse = UiInput::instance().mouse();
    const Vector2 moved = GetMouseDelta();
    const bool mouseMoved = (moved.x != 0.0f || moved.y != 0.0f);

    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    int cardUnderMouse = -1;
    for (size_t i = 0; i < m_waves.size(); ++i)
        if (CheckCollisionPointRec(mouse, cardRect(i))) cardUnderMouse = static_cast<int>(i);
    if (cardUnderMouse >= 0 && mouseMoved) m_selected = static_cast<size_t>(cardUnderMouse);

    if (UiInput::instance().leftPressed()) {
        if (cardUnderMouse >= 0 && UiInput::instance().click(cardRect(static_cast<size_t>(cardUnderMouse)))) {
            const bool alreadySelected = (static_cast<size_t>(cardUnderMouse) == m_selected);
            m_selected = static_cast<size_t>(cardUnderMouse);
            if (alreadySelected) {                       // bam lan 2 vao the dang chon = bat dau
                const WaveSelectAction a = tryStart();
                if (a != WaveSelectAction::NONE) return a;
            }
        }
        if (m_hoverButton == BTN_BACK && UiInput::instance().click(buttonRect(BTN_BACK))) return WaveSelectAction::BACK;
        if (m_hoverButton == BTN_START && UiInput::instance().click(buttonRect(BTN_START))) {
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
    const float cx = UiTheme::WIDTH / 2.0f;
    drawCenteredText("Chọn ải", cx, 30.0f, 44, COL_TITLE);
    if (!m_heroSummary.empty())
        drawCenteredText(TextFormat("Anh hùng: %s", m_heroSummary.c_str()), cx, 86.0f, 18, COL_LABEL);

    if (m_waves.empty()) {
        drawCenteredText("Không có dữ liệu wave (kiem tra thu muc data/)", cx, 320.0f, 22, COL_ERR);
    } else {
        for (size_t i = 0; i < m_waves.size(); ++i)
            if (i != m_selected) drawCard(i);
        drawCard(m_selected);   // ve the dang chon sau cung de nam tren
    }

    drawButtons();
    if (m_messageTimer > 0.0f) drawCenteredText(m_message.c_str(), cx, 612.0f, 20, COL_ERR);
    drawCenteredText("Chọn ải đã mở khóa rồi nhấp Bắt đầu", cx, 702.0f, 16, Color{ 200, 195, 225, 220 });
}

void WaveSelectScreen::drawBackground() const {
    const int W = UiTheme::WIDTH, H = UiTheme::HEIGHT;
    if (m_hasBackground) {
        DrawTexturePro(m_background, Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, static_cast<float>(W), static_cast<float>(H) }, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 18, 22, 30, 255 }, Color{ 54, 57, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 160 });
}

void WaveSelectScreen::drawCard(size_t index) const {
    const WaveInfo& w = m_waves[index];
    const bool selected = (index == m_selected);
    Rectangle r = cardRect(index);
    if (selected) r.y -= SELECT_LIFT;

    UiFrame::panel(r, selected);

    // ---- Dai tieu de ----
    UiFrame::plaque({r.x+11, r.y+9, r.width-22,HEADER_H-14}, selected, w.m_isBoss);
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
    UiText::draw("QUÁI CÓ THỂ GẶP", static_cast<int>(r.x + 20), static_cast<int>(y), 15, COL_TITLE);
    y += 28.0f;
    for (size_t i = 0; i < w.m_enemies.size() && i < MAX_ENEMY_LINES; ++i) {
        const WaveEnemy& e = w.m_enemies[i];
        DrawCircle(static_cast<int>(r.x + 30), static_cast<int>(y + 10), 6.0f, elementColor(e.m_element));
        UiText::draw(e.m_name.c_str(), static_cast<int>(r.x + 46), static_cast<int>(y), 18, COL_TEXT);
        const char* el = elementName(e.m_element);
        UiText::draw(el, static_cast<int>(r.x + r.width - 20 - UiText::measure(el, 14)), static_cast<int>(y + 3), 14, elementColor(e.m_element));
        y += 28.0f;
    }
    if (w.m_enemies.size() > MAX_ENEMY_LINES)
        UiText::draw(TextFormat("+%d quái khác", static_cast<int>(w.m_enemies.size() - MAX_ENEMY_LINES)),
                 static_cast<int>(r.x + 46), static_cast<int>(y), 14, COL_DIM);

    // ---- Trang thai ----
    if (w.m_cleared && !w.m_locked)
        drawCenteredText("ĐÃ HOÀN THÀNH", r.x + r.width / 2.0f, r.y + r.height - 38.0f, 18, COL_OK);

    // ---- Vien (vang khi duoc chon) ----
    // Slanted panel already provides an ember edge; avoid rectangular outline.

    // ---- Khoa: phu toi + o khoa ----
    if (w.m_locked) {
        DrawRectangleRec(r, Color{ 21, 18, 31, 255 });
        const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f - 10.0f;
        DrawRectangleLinesEx(Rectangle{ cx - 14, cy - 34, 28, 30 }, 5.0f, Color{ 200, 170, 80, 255 });   // quai khoa
        DrawRectangleRec(Rectangle{ cx - 24, cy - 8, 48, 36 }, Color{ 200, 170, 80, 255 });              // than khoa
        DrawRectangle(static_cast<int>(cx - 4), static_cast<int>(cy + 4), 8, 12, Color{ 60, 44, 20, 255 });
        const std::string hint = w.m_lockHint.empty()
            ? std::string(TextFormat("Hoàn thành ải %d\nđể mở khóa", std::max(1, w.m_number - 1)))
            : w.m_lockHint;
        float hy = cy + 44.0f;
        for (const std::string& line : splitLines(hint)) {
            drawCenteredText(line.c_str(), cx, hy, 16, COL_TEXT);
            hy += 22.0f;
        }
        // Slanted panel already provides an ember edge; avoid rectangular outline.
    }
}

void WaveSelectScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { "Quay lại", "Bắt đầu" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const bool disabled = primary && selectedLocked();
        UiFrame::plaque(r, primary && !disabled, false, hover&&!disabled?1.f:0.f);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         disabled ? COL_DIM : (hover ? UiTheme::AGED_GOLD : COL_TEXT));
    }
}