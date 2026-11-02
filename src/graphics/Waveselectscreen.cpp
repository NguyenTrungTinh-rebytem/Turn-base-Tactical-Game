// Member 4 - Huynh Quang Khai | src/graphics/WaveSelectScreen.cpp
#include "WaveSelectScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float CARD_W = 560.0f, CARD_H = 452.0f, CARD_GAP = 32.0f, CARD_Y = 142.0f;
constexpr float HEADER_H = 64.0f, SELECT_LIFT = 6.0f;
constexpr float BTN_W = 220.0f, BTN_H = 48.0f, BTN_GAP = 30.0f, BTN_Y = 642.0f;
constexpr size_t MAX_ENEMY_LINES = 7;

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
        case Element::KIM:  return Strings::elements[0];
        case Element::MOC:  return Strings::elements[1];
        case Element::THUY: return Strings::elements[2];
        case Element::HOA:  return Strings::elements[3];
        case Element::THO:  return Strings::elements[4];
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
    m_hoverCard = -1;
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
    const float x0 = (static_cast<float>(UiCanvas::width()) -
                      (CARD_W * 2.0f + CARD_GAP)) * 0.5f;
    if (index == m_selected) return { x0, CARD_Y, CARD_W, CARD_H };
    if (index == m_selected + 1)
        return { x0 + CARD_W + CARD_GAP, CARD_Y, CARD_W, CARD_H };
    return { -CARD_W * 2.0f, CARD_Y, CARD_W, CARD_H };
}

Rectangle WaveSelectScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(UiCanvas::width()) - total) / 2.0f;
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
        m_message = u8"Màn này chưa mở khóa! Hãy hoàn thành màn trước đó.";
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
    const Vector2 mouse = UiCanvas::mousePosition();
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    m_hoverCard = -1;
    for (size_t i = m_selected; i < m_waves.size() && i <= m_selected + 1; ++i)
        if (CheckCollisionPointRec(mouse, cardRect(i))) m_hoverCard = static_cast<int>(i);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (m_hoverCard == static_cast<int>(m_selected + 1))
            m_selected = static_cast<size_t>(m_hoverCard);
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
    const float cx = UiCanvas::width() / 2.0f;
    drawCenteredText(Strings::selectWave, cx, 30.0f, 44, COL_TITLE);
    if (!m_heroSummary.empty())
        drawCenteredText(TextFormat(u8"Anh hùng: %s", m_heroSummary.c_str()), cx, 92.0f, 18, COL_LABEL);

    if (m_waves.empty()) {
        drawCenteredText(Strings::waveDataMissing, cx, 320.0f, 22, COL_ERR);
    } else {
        drawCard(m_selected);
        if (m_selected + 1 < m_waves.size()) drawCard(m_selected + 1);
        else {
            const Rectangle next = cardRect(m_selected + 1);
            UiShapes::roundedRect(next, 16.0f, Color{ 18, 16, 36, 210 },
                                  Color{ 110, 102, 132, 180 }, 2.0f);
            UiIcons::draw("action:close",
                Rectangle{ next.x + next.width * 0.5f - 28.0f, next.y + 92.0f, 56.0f, 56.0f },
                Color{ 120, 116, 132, 200 });
            drawCenteredText(u8"Chưa có màn tiếp theo",
                next.x + next.width * 0.5f, next.y + 176.0f, 24, COL_DIM);
            drawCenteredText(u8"Hoàn thành hành trình hiện tại",
                next.x + next.width * 0.5f, next.y + 218.0f, 18, COL_DIM);
        }
    }

    drawButtons();
    if (m_messageTimer > 0.0f) drawCenteredText(m_message.c_str(), cx, 604.0f, 18, COL_ERR);
}

void WaveSelectScreen::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(W), static_cast<float>(H) });
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 10, 8, 36, 255 }, Color{ 60, 30, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 125 });
}

void WaveSelectScreen::drawCard(size_t index) const {
    const WaveInfo& w = m_waves[index];
    const bool selected = (index == m_selected);
    Rectangle r = cardRect(index);
    if (selected) r.y -= SELECT_LIFT;
    const bool hovered = static_cast<int>(index) == m_hoverCard;
    UiShapes::roundedRect(Rectangle{ r.x + 5.0f, r.y + 6.0f, r.width, r.height },
                          16.0f, Color{ 0, 0, 0, 120 });
    UiShapes::roundedRect(r, 16.0f, w.m_locked ? Color{ 34, 34, 46, 220 } : COL_PANEL,
                          selected ? COL_EDGE_SEL : (hovered ? COL_TITLE : COL_EDGE),
                          selected ? 3.0f : 2.0f);

    // ---- Dai tieu de ----
    DrawRectangleRounded(Rectangle{ r.x, r.y, r.width, HEADER_H }, 0.15f, 8,
                         w.m_isBoss ? COL_HEAD_BOSS : COL_HEAD_NORMAL);
    drawCenteredText(w.m_title.c_str(), r.x + r.width / 2.0f, r.y + 16.0f, 32, COL_TEXT);
    if (w.m_isBoss) {
        const Rectangle tag = { r.x + r.width - 92.0f, r.y + 16.0f, 70.0f, 28.0f };
        DrawRectangleRec(tag, Color{ 200, 50, 60, 255 });
        DrawRectangleLinesEx(tag, 1.0f, Color{ 255, 150, 150, 255 });
        drawCenteredText(u8"BOSS", tag.x + tag.width / 2.0f, tag.y + 5.0f, 16, WHITE);
    }

    // ---- Mo ta ----
    float y = r.y + HEADER_H + 14.0f;
    for (const std::string& line : splitLines(w.m_description)) {
        drawCenteredText(line.c_str(), r.x + r.width / 2.0f, y, 16, COL_LABEL);
        y += 22.0f;
    }

    // ---- Danh sach quai ----
    y = r.y + 150.0f;
    UiText::draw(Strings::availableEnemies, static_cast<int>(r.x + 20), static_cast<int>(y), 15, COL_TITLE);
    y += 28.0f;
    for (size_t i = 0; i < w.m_enemies.size() && i < MAX_ENEMY_LINES; ++i) {
        const WaveEnemy& e = w.m_enemies[i];
        const Rectangle row{ r.x + 18.0f, y - 3.0f, r.width - 36.0f, 26.0f };
        const bool enemyHover = CheckCollisionPointRec(UiCanvas::mousePosition(), row);
        UiShapes::roundedRect(row, 6.0f,
            enemyHover ? Color{ 48, 42, 75, 245 } : Color{ 27, 24, 50, 230 });
        UiIcons::draw(std::string("element:") + (static_cast<int>(e.m_element) == 0 ? "kim" :
            static_cast<int>(e.m_element) == 1 ? "moc" : static_cast<int>(e.m_element) == 2 ? "thuy" :
            static_cast<int>(e.m_element) == 3 ? "hoa" : "tho"),
            Rectangle{ r.x + 24.0f, y + 1.0f, 20.0f, 20.0f }, elementColor(e.m_element));
        const std::string name = Strings::localizedName(e.m_name);
        UiText::draw(name.c_str(), static_cast<int>(r.x + 54), static_cast<int>(y), 18, COL_TEXT);
        const char* el = elementName(e.m_element);
        if (e.m_isBoss) {
            UiText::draw(TextFormat(u8"BOSS · HP %d", e.m_hp),
                static_cast<int>(r.x + r.width - 160.0f), static_cast<int>(y + 3.0f), 14,
                Color{ 255, 145, 145, 255 });
        } else {
            UiText::draw(el, static_cast<int>(r.x + r.width - 20 - UiText::measure(el, 14)),
                         static_cast<int>(y + 3), 14, elementColor(e.m_element));
        }
        y += 31.0f;
    }
    if (w.m_enemies.size() > MAX_ENEMY_LINES)
        UiText::draw(TextFormat(u8"+%d quái khác", static_cast<int>(w.m_enemies.size() - MAX_ENEMY_LINES)),
                 static_cast<int>(r.x + 46), static_cast<int>(y), 14, COL_DIM);

    // ---- Trang thai ----
    if (w.m_cleared && !w.m_locked)
        drawCenteredText(Strings::waveCleared, r.x + r.width / 2.0f, r.y + r.height - 38.0f, 18, COL_OK);

    // ---- Khoa: phu toi + o khoa ----
    if (w.m_locked) {
        DrawRectangleRounded(r, 0.1f, 10, Color{ 0, 0, 0, 130 });
        const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f - 10.0f;
        DrawRectangleLinesEx(Rectangle{ cx - 14, cy - 34, 28, 30 }, 5.0f, Color{ 200, 170, 80, 255 });   // quai khoa
        DrawRectangleRec(Rectangle{ cx - 24, cy - 8, 48, 36 }, Color{ 200, 170, 80, 255 });              // than khoa
        DrawRectangle(static_cast<int>(cx - 4), static_cast<int>(cy + 4), 8, 12, Color{ 60, 44, 20, 255 });
        const std::string hint = w.m_lockHint.empty()
            ? std::string(TextFormat(u8"Hoàn thành màn %d\nđể mở khóa", std::max(1, w.m_number - 1)))
            : w.m_lockHint;
        float hy = cy + 44.0f;
        for (const std::string& line : splitLines(hint)) {
            drawCenteredText(line.c_str(), cx, hy, 16, COL_TEXT);
            hy += 22.0f;
        }
        DrawRectangleRoundedLinesEx(r, 0.1f, 10, selected ? 3.0f : 2.0f,
                                    selected ? COL_EDGE_SEL : COL_EDGE);
    }
}

void WaveSelectScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::back, Strings::start };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const bool disabled = primary && selectedLocked();
        UiShapes::roundedRect(Rectangle{ r.x + 4, r.y + 5, r.width, r.height }, 10.0f,
                              Color{ 0, 0, 0, 110 });
        UiShapes::roundedRect(r, 10.0f,
            disabled ? Color{ 24, 22, 34, 220 } :
            (hover ? Color{ 90, 60, 20, 235 } : (primary ? Color{ 50, 40, 14, 230 } : COL_PANEL)),
            disabled ? Color{ 80, 76, 100, 255 } :
            ((hover || primary) ? COL_EDGE_SEL : COL_EDGE), hover && !disabled ? 3.0f : 2.0f);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         disabled ? COL_DIM : (hover ? Color{ 255, 240, 170, 255 } : COL_TEXT));
    }
}