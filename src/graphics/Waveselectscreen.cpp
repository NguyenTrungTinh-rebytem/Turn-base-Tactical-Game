// Member 4 - Huynh Quang Khai | src/graphics/WaveSelectScreen.cpp
#include "WaveSelectScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float CARD_W = 560.0f, CARD_H = 452.0f, CARD_GAP = 32.0f, CARD_Y = 142.0f;
constexpr float HEADER_H = 64.0f, SELECT_LIFT = 6.0f;
constexpr float BTN_W = 220.0f, BTN_H = 48.0f, BTN_GAP = 30.0f, BTN_Y = 642.0f;
constexpr size_t MAX_ENEMY_LINES = 7;

// ---- Mau ----
constexpr Color COL_TITLE = UiTheme::silver;
constexpr Color COL_PANEL = UiTheme::panel;
constexpr Color COL_EDGE = UiTheme::ash;
constexpr Color COL_EDGE_SEL = UiTheme::crimson;
constexpr Color COL_LABEL = UiTheme::ashText;
constexpr Color COL_TEXT = UiTheme::silver;
constexpr Color COL_DIM = UiTheme::ashText;
constexpr Color COL_ERR = UiTheme::danger;
constexpr Color COL_OK = UiTheme::heal;
constexpr Color COL_HEAD_NORMAL = UiTheme::panelRaised;
constexpr Color COL_HEAD_BOSS{ 50, 10, 17, 224 };

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

void drawCenteredText(const char* text, float cx, float y, int size, Color c,
                      float trackingEm = 0.0f) {
    UiText::draw(text, static_cast<int>(cx -
        UiText::measure(text, size, trackingEm) / 2.0f), static_cast<int>(y), size, c, trackingEm);
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
    drawCenteredText(Strings::selectWave, cx, 30.0f, 44, COL_TITLE,
                     UiTheme::displayTracking);
    if (!m_heroSummary.empty())
        drawCenteredText(TextFormat(u8"Anh hùng: %s", m_heroSummary.c_str()), cx, 92.0f, 18, COL_LABEL);

    if (m_waves.empty()) {
        drawCenteredText(Strings::waveDataMissing, cx, 320.0f, 22, COL_ERR);
    } else {
        drawCard(m_selected);
        if (m_selected + 1 < m_waves.size()) drawCard(m_selected + 1);
        else {
            const Rectangle next = cardRect(m_selected + 1);
            UiShapes::plate(next, UiTheme::panel, UiTheme::ash, 1.0f);
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
        DrawRectangleGradientV(0, 0, W, H, Color{ 18, 12, 16, 255 },
                               Color{ 57, 14, 23, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 125 });
}

void WaveSelectScreen::drawCard(size_t index) const {
    const WaveInfo& w = m_waves[index];
    const bool selected = (index == m_selected);
    Rectangle r = cardRect(index);
    if (selected) r.y -= SELECT_LIFT;
    const bool hovered = static_cast<int>(index) == m_hoverCard;
    UiShapes::plate(Rectangle{ r.x + 5.0f, r.y + 6.0f, r.width, r.height },
                    Color{ 0, 0, 0, 120 }, BLANK);
    UiShapes::plate(r, w.m_locked ? Color{ 14, 12, 20, 220 } : COL_PANEL,
                          selected ? COL_EDGE_SEL : (hovered ? COL_TITLE : COL_EDGE),
                          selected ? 3.0f : 2.0f);

    UiShapes::bladeStrip(Rectangle{ r.x, r.y, r.width, HEADER_H },
                         w.m_isBoss ? COL_HEAD_BOSS : COL_HEAD_NORMAL,
                         selected ? UiTheme::crimson : COL_EDGE, false, selected, hovered);
    drawCenteredText(w.m_title.c_str(), r.x + r.width / 2.0f, r.y + 16.0f, 32, COL_TEXT,
                     UiTheme::displayTracking);
    if (w.m_isBoss) {
        const Rectangle tag = { r.x + r.width - 92.0f, r.y + 16.0f, 70.0f, 28.0f };
        UiShapes::bladeStrip(tag, Color{ 50, 10, 17, 224 }, UiTheme::crimson, false, true);
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
        UiShapes::bladeStrip(row,
            enemyHover ? UiTheme::panelRaised : UiTheme::panel,
            enemyHover ? UiTheme::silver : UiTheme::ash, false, enemyHover, enemyHover);
        UiIcons::draw(std::string("element:") + (static_cast<int>(e.m_element) == 0 ? "kim" :
            static_cast<int>(e.m_element) == 1 ? "moc" : static_cast<int>(e.m_element) == 2 ? "thuy" :
            static_cast<int>(e.m_element) == 3 ? "hoa" : "tho"),
            Rectangle{ r.x + 24.0f, y + 1.0f, 20.0f, 20.0f }, elementColor(e.m_element));
        const std::string name = Strings::localizedName(e.m_name);
        UiText::draw(name.c_str(), static_cast<int>(r.x + 56), static_cast<int>(y), 18, COL_TEXT);
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
        UiShapes::plate(r, Color{ 7, 6, 10, 150 }, BLANK);
        const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f - 10.0f;
        DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy - 14.0f), 13.0f, UiTheme::gold);
        UiShapes::plate(Rectangle{ cx - 24, cy - 8, 48, 36 }, UiTheme::gold, UiTheme::silver, 1.0f);
        UiShapes::diamond(Vector2{ cx, cy + 10.0f }, 8.0f, UiTheme::ink, true);
        const std::string hint = w.m_lockHint.empty()
            ? std::string(TextFormat(u8"Hoàn thành màn %d\nđể mở khóa", std::max(1, w.m_number - 1)))
            : w.m_lockHint;
        float hy = cy + 44.0f;
        for (const std::string& line : splitLines(hint)) {
            drawCenteredText(line.c_str(), cx, hy, 16, COL_TEXT);
            hy += 22.0f;
        }
        UiShapes::plate(r, BLANK, selected ? COL_EDGE_SEL : COL_EDGE,
                        selected ? 3.0f : 2.0f);
    }
}

void WaveSelectScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::back, Strings::start };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const bool disabled = primary && selectedLocked();
        UiShapes::bladeStrip(r, primary ? Color{ 50, 10, 17, 224 } : COL_PANEL,
            disabled ? COL_DIM : (primary ? UiTheme::crimson : COL_EDGE),
            false, primary && !disabled, hover, !disabled);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         disabled ? COL_DIM : COL_TEXT, UiTheme::displayTracking);
    }
}