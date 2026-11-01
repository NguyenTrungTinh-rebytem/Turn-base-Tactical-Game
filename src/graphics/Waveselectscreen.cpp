// Member 4 - Huynh Quang Khai | src/graphics/WaveSelectScreen.cpp
#include "WaveSelectScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiInput.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float CARD_W = 540.0f, CARD_H = 470.0f, CARD_GAP = 36.0f, CARD_Y = 125.0f;
constexpr float HEADER_H = 64.0f, SELECT_LIFT = 8.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr size_t MAX_ENEMY_LINES = 6;

// ---- Mau ----
constexpr Color COL_TITLE      = UiTheme::accent;
constexpr Color COL_PANEL      = UiTheme::panel;
constexpr Color COL_EDGE       = UiTheme::border;
constexpr Color COL_EDGE_SEL   = UiTheme::ember;
constexpr Color COL_LABEL      = UiTheme::ash;
constexpr Color COL_TEXT       = UiTheme::text;
constexpr Color COL_DIM        = UiTheme::ash;
constexpr Color COL_ERR        = UiTheme::danger;
constexpr Color COL_OK         = UiTheme::verdigris;
constexpr Color COL_HEAD_NORMAL{ 29, 24, 48, 255 };
constexpr Color COL_HEAD_BOSS  = UiTheme::dangerTop;

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return UiTheme::kim;
        case Element::MOC:  return UiTheme::moc;
        case Element::THUY: return UiTheme::thuy;
        case Element::HOA:  return UiTheme::hoa;
        case Element::THO:  return UiTheme::tho;
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
                      float letterSpacing = 0.0f, UiFontRole role = UiFontRole::AUTO) {
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size, role, letterSpacing) / 2.0f),
                 static_cast<int>(y), size, c, role, letterSpacing);
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
    const float total = CARD_W * 2.0f + CARD_GAP;
    const float startX = (static_cast<float>(UiCanvas::width()) - total) / 2.0f;
    return { startX + static_cast<float>(index) * (CARD_W + CARD_GAP), CARD_Y, CARD_W, CARD_H };
}

Rectangle WaveSelectScreen::navigationRect(bool next) const {
    const float cx = UiCanvas::width() / 2.0f;
    return { cx + (next ? 176.0f : -216.0f), 28.0f, 40.0f, 40.0f };
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
        m_message = Strings::waveLocked;
        m_messageTimer = 2.0f;
        return WaveSelectAction::NONE;
    }
    return WaveSelectAction::START;
}

WaveSelectAction WaveSelectScreen::handleInput() {
    if (UiInput::rightClicked()) {
        UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
        return WaveSelectAction::BACK;
    }
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
    const Vector2 mouse = UiInput::mousePosition();

    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    int cardSlotUnderMouse = -1;
    for (size_t slot = 0; slot < 2; ++slot) {
        const size_t waveIndex = m_selected + slot;
        if (waveIndex < m_waves.size() && CheckCollisionPointRec(mouse, cardRect(slot)))
            cardSlotUnderMouse = static_cast<int>(slot);
    }
    const bool overPrevious = CheckCollisionPointRec(mouse, navigationRect(false));
    const bool overNext = CheckCollisionPointRec(mouse, navigationRect(true));
    if (overPrevious || overNext || cardSlotUnderMouse >= 0)
        UiInput::setCursorType(UiInput::CursorType::POINTER);

    if (UiInput::pressed(MOUSE_BUTTON_LEFT)) {
        if (overPrevious && m_selected > 0) {
            UiInput::consumeClick();
            select(-1);
        } else if (overNext && m_selected + 1 < m_waves.size()) {
            UiInput::consumeClick();
            select(+1);
        } else if (cardSlotUnderMouse == 1) {
            UiInput::consumeClick();
            select(+1);
        } else if (cardSlotUnderMouse == 0) {
            const bool doubleClick = UiInput::doubleClicked();
            UiInput::consumeClick();
            if (doubleClick) {
                const WaveSelectAction a = tryStart();
                if (a != WaveSelectAction::NONE) return a;
            }
        }
        if (m_hoverButton == BTN_BACK) {
            UiInput::consumeClick();
            return WaveSelectAction::BACK;
        }
        if (m_hoverButton == BTN_START) {
            UiInput::consumeClick();
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
        drawCenteredText(TextFormat(Strings::heroSummaryTemplate, m_heroSummary.c_str()), cx, 86.0f, 18, COL_LABEL);

    if (m_waves.empty()) {
        drawCenteredText(Strings::waveDataMissing, cx, 320.0f, 22, COL_ERR);
    } else {
        drawCard(m_selected);
        if (m_selected + 1 < m_waves.size())
            drawCard(m_selected + 1);
        else
            drawFollowingWavePlaceholder();
        drawNavigation();
    }

    drawButtons();
    if (m_messageTimer > 0.0f) drawCenteredText(m_message.c_str(), cx, 612.0f, 20, COL_ERR);
}

void WaveSelectScreen::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(W), static_cast<float>(H) });
    } else {
        DrawRectangleGradientV(0, 0, W, H, UiTheme::obsidian, UiTheme::panelBottom);
    }
    DrawRectangle(0, 0, W, H, Color{ 12, 10, 18, 135 });
}

void WaveSelectScreen::drawCard(size_t index) const {
    const WaveInfo& w = m_waves[index];
    const bool selected = (index == m_selected);
    const size_t slot = index == m_selected ? 0 : 1;
    Rectangle r = cardRect(slot);
    if (selected) r.y -= SELECT_LIFT;

    UiShapeEffects cardEffects;
    cardEffects.gradientBottom = UiTheme::panelBottom;
    cardEffects.shadowBlur = 8.0f;
    cardEffects.glow = UiTheme::ember;
    cardEffects.glowRadius = selected ? 5.0f : 0.0f;
    cardEffects.glowStrength = selected ? 0.35f : 0.0f;
    UiShapes::roundedRect(r, UiTheme::radiusCard, COL_PANEL,
                          selected ? COL_EDGE_SEL : COL_EDGE, 1.0f, cardEffects);

    // ---- Dai tieu de ----
    UiShapeEffects headerEffects;
    headerEffects.flatBottom = true;
    headerEffects.gradientBottom = w.m_isBoss ? UiTheme::dangerBottom : UiTheme::panelBottom;
    UiShapes::roundedRect(Rectangle{ r.x, r.y, r.width, HEADER_H },
                          UiTheme::radiusCard, w.m_isBoss ? COL_HEAD_BOSS : COL_HEAD_NORMAL,
                          BLANK, 0.0f, headerEffects);
    const float titleCenter = w.m_isBoss ? r.x + r.width / 2.0f - 18.0f : r.x + r.width / 2.0f;
    drawCenteredText(w.m_title.c_str(), titleCenter, r.y + 16.0f, 32, COL_TEXT);
    if (w.m_isBoss) {
        const Rectangle tag = { r.x + r.width - 156.0f, r.y + 18.0f, 140.0f, 28.0f };
        UiShapes::roundedRect(tag, UiTheme::radiusChip, UiTheme::danger,
                              UiTheme::dangerTop, 1.0f);
        drawCenteredText(TextFormat("%s · %s", Strings::boss,
            TextFormat(Strings::bossHealthTemplate, w.m_bossHp)),
            tag.x + tag.width / 2.0f, tag.y + 5.0f, 14, WHITE, 0.0f, UiFontRole::HUD);
    }

    // ---- Mo ta ----
    float y = r.y + HEADER_H + 14.0f;
    for (const std::string& line : splitLines(w.m_description)) {
        drawCenteredText(line.c_str(), r.x + r.width / 2.0f, y, 16, COL_LABEL);
        y += 22.0f;
    }

    // ---- Danh sach quai ----
    y = r.y + 150.0f;
    UiText::draw(Strings::availableEnemies, static_cast<int>(r.x + 20), static_cast<int>(y), 16, COL_TITLE,
                 UiFontRole::HUD);
    y += 28.0f;
    for (size_t i = 0; i < w.m_enemies.size() && i < MAX_ENEMY_LINES; ++i) {
        const WaveEnemy& e = w.m_enemies[i];
        UiElementSigil::draw(e.m_element, Rectangle{ r.x + 21.0f, y + 1.0f, 18.0f, 18.0f });
        const std::string name = Strings::localizedName(e.m_name);
        UiText::draw(name.c_str(), static_cast<int>(r.x + 46), static_cast<int>(y), 18, COL_TEXT);
        const char* el = elementName(e.m_element);
        UiText::draw(el, static_cast<int>(r.x + r.width - 20 - UiText::measure(el, 16)),
                     static_cast<int>(y + 3), 16, elementColor(e.m_element), UiFontRole::HUD);
        y += 28.0f;
    }
    if (w.m_enemies.size() > MAX_ENEMY_LINES)
        UiText::draw(TextFormat(Strings::additionalEnemiesTemplate, static_cast<int>(w.m_enemies.size() - MAX_ENEMY_LINES)),
                 static_cast<int>(r.x + 46), static_cast<int>(y), 16, COL_DIM);

    // ---- Trang thai ----
    if (w.m_cleared && !w.m_locked)
        drawCenteredText(Strings::waveCleared, r.x + r.width / 2.0f, r.y + r.height - 38.0f, 18, COL_OK);

    // ---- Vien (vang khi duoc chon) ----
    // ---- Khoa: phu toi + o khoa ----
    if (w.m_locked) {
        UiShapes::roundedRect(r, UiTheme::radiusCard, Color{ 12, 10, 18, 90 });
        const float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f - 10.0f;
        UiIcons::draw("action:lock", Rectangle{ cx - 20.0f, cy - 18.0f, 40.0f, 40.0f });
        const std::string hint = w.m_lockHint.empty()
            ? std::string(TextFormat(Strings::unlockWaveTemplate, std::max(1, w.m_number - 1)))
            : w.m_lockHint;
        float hy = cy + 44.0f;
        for (const std::string& line : splitLines(hint)) {
            drawCenteredText(line.c_str(), cx, hy, 16, COL_TEXT);
            hy += 22.0f;
        }
    }
}

void WaveSelectScreen::drawFollowingWavePlaceholder() const {
    const Rectangle r = cardRect(1);
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(r, UiTheme::radiusCard, UiTheme::panel,
                          UiTheme::border, 1.0f, effects);
    UiIcons::draw("action:confirm",
        Rectangle{ r.x + r.width / 2.0f - 24.0f, r.y + 146.0f, 48.0f, 48.0f },
        UiTheme::disabled);
    drawCenteredText(Strings::noFollowingWave, r.x + r.width / 2.0f,
                     r.y + 212.0f, 22, COL_DIM, 0.0f, UiFontRole::BODY);
}

void WaveSelectScreen::drawNavigation() const {
    const Rectangle previous = navigationRect(false);
    const Rectangle next = navigationRect(true);
    for (int i = 0; i < 2; ++i) {
        const bool right = i == 1;
        const Rectangle r = right ? next : previous;
        const bool enabled = right ? m_selected + 1 < m_waves.size() : m_selected > 0;
        UiShapes::roundedRect(r, UiTheme::radiusSmall,
            enabled ? UiTheme::panel : UiTheme::obsidian,
            enabled ? UiTheme::secondaryBorder : UiTheme::border, 1.0f);
        const float cx = r.x + r.width / 2.0f;
        const float cy = r.y + r.height / 2.0f;
        const Color color = enabled ? UiTheme::accent : UiTheme::disabled;
        if (right)
            DrawTriangle(Vector2{ cx - 5.0f, cy - 9.0f }, Vector2{ cx - 5.0f, cy + 9.0f },
                         Vector2{ cx + 7.0f, cy }, color);
        else
            DrawTriangle(Vector2{ cx + 5.0f, cy + 9.0f }, Vector2{ cx + 5.0f, cy - 9.0f },
                         Vector2{ cx - 7.0f, cy }, color);
    }
}

void WaveSelectScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::back, Strings::start };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const bool disabled = primary && selectedLocked();
        Rectangle body = r;
        if (hover && !disabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) body.y += 2.0f;
        UiShapeEffects effects;
        effects.gradientBottom = primary ? UiTheme::primaryBottom : UiTheme::panelBottom;
        effects.shadowBlur = 6.0f;
        effects.glow = UiTheme::ember;
        effects.glowRadius = hover && !disabled ? 5.0f : 0.0f;
        effects.glowStrength = hover && !disabled ? 0.4f : 0.0f;
        UiShapes::roundedRect(body, UiTheme::radiusSmall,
            disabled ? UiTheme::panel : (primary ? UiTheme::primaryTop : UiTheme::panel),
            disabled ? UiTheme::disabled : ((hover || primary) ? COL_EDGE_SEL : COL_EDGE),
            1.0f, effects);
        drawCenteredText(labels[i], body.x + body.width / 2.0f,
                         body.y + (body.height - 24.0f) / 2.0f, 24,
                         disabled ? COL_DIM : (primary ? UiTheme::primaryText : COL_TEXT),
                         24.0f * 0.04f, UiFontRole::HUD);
    }
}