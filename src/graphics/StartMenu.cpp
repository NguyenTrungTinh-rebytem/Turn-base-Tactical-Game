// Member 4 - Huynh Quang Khai | src/graphics/StartMenu.cpp
#include "StartMenu.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiInput.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/UiIcons.h"
#include "graphics/Strings.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720, tinh theo kich thuoc cua so) ----
constexpr float TITLE_Y_RATIO   = 0.17f;
constexpr float BUTTON_W        = 280.0f;
constexpr float BUTTON_H        = 48.0f;
constexpr float BUTTON_GAP      = 18.0f;
constexpr float BUTTONS_Y_RATIO = 0.50f;
constexpr float FADE_IN_SECONDS = 0.18f;
constexpr float HOVER_SPEED     = 10.0f;

// ---- Mau ----
constexpr Color COL_TITLE        = UiTheme::currency;
constexpr Color COL_TITLE_SHADOW{ 12, 10, 18, 220 };
constexpr Color COL_SUBTITLE     = UiTheme::ash;
constexpr Color COL_BTN          = UiTheme::panel;
constexpr Color COL_BTN_SELECTED = UiTheme::primaryTop;
constexpr Color COL_BTN_EDGE     = UiTheme::border;
constexpr Color COL_BTN_EDGE_SEL = UiTheme::ember;
constexpr Color COL_TEXT         = UiTheme::text;
constexpr Color COL_TEXT_SEL     = UiTheme::primaryText;
constexpr Color COL_OK           = UiTheme::verdigris;
constexpr Color COL_ERR          = UiTheme::danger;
constexpr Color COL_EMBER(unsigned char alpha) { return Color{ 232, 131, 43, alpha }; }

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// So gia ngau nhien co dinh theo chi so (khong can luu trang thai)
float hash01(int n) {
    unsigned int x = static_cast<unsigned int>(n) * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    return static_cast<float>(x & 0xFFFFu) / 65535.0f;
}

void drawCenteredText(const char* text, float centerX, float y, int size, Color color,
                      float letterSpacing = 0.0f, UiFontRole role = UiFontRole::AUTO) {
    const int w = UiText::measure(text, size, role, letterSpacing);
    UiText::draw(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), size,
                 color, role, letterSpacing);
}
} // namespace

StartMenu::StartMenu() {
    m_buttons.push_back({ Strings::menuStart, MenuAction::START_GAME, 0.0f });
    m_buttons.push_back({ Strings::menuExit,   MenuAction::EXIT,       0.0f });
}

StartMenu::~StartMenu() { unloadAll(); }

bool StartMenu::loadBackground(const std::string& path) {
    unloadAll();
    if (path.empty() || !FileExists(path.c_str())) return false;   // thieu anh -> nen thu tuc
    m_background = LoadTexture(path.c_str());
    m_hasBackground = (m_background.id != 0);
    if (m_hasBackground) SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);   // anh minh hoa (khong phai pixel art): lam muot khi phong/thu
    return m_hasBackground;
}

void StartMenu::unloadAll() {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);   // sau CloseWindow() thi bo qua, khong crash
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

Rectangle StartMenu::buttonRect(size_t index) const {
    const float W = static_cast<float>(UiCanvas::width());
    const float H = static_cast<float>(UiCanvas::height());
    return { W * m_menuCenterX - BUTTON_W / 2.0f,
             H * BUTTONS_Y_RATIO + static_cast<float>(index) * (BUTTON_H + BUTTON_GAP),
             BUTTON_W, BUTTON_H };
}

Rectangle StartMenu::exitDialogRect() const {
    const float width = 500.0f, height = 190.0f;
    return { (UiCanvas::width() - width) / 2.0f,
             (UiCanvas::height() - height) / 2.0f, width, height };
}

Rectangle StartMenu::exitDialogButtonRect(bool confirm) const {
    const Rectangle dialog = exitDialogRect();
    return { dialog.x + (confirm ? 268.0f : 88.0f), dialog.y + 122.0f, 144.0f, 46.0f };
}

void StartMenu::update(float deltaTime) {
    m_time += deltaTime;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const float target = (static_cast<int>(i) == m_selected) ? 1.0f : 0.0f;
        m_buttons[i].m_hover += (target - m_buttons[i].m_hover) * std::min(1.0f, HOVER_SPEED * deltaTime);
    }
}

MenuAction StartMenu::handleInput() {
    if (m_buttons.empty()) return MenuAction::NONE;
    const int count = static_cast<int>(m_buttons.size());

    if (m_exitDialogOpen) {
        const Vector2 mouse = UiInput::mousePosition();
        if (IsKeyPressed(KEY_ESCAPE) || UiInput::rightClicked()) {
            if (UiInput::rightClicked()) UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
            m_exitDialogOpen = false;
            return MenuAction::NONE;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            m_exitDialogOpen = false;
            return MenuAction::EXIT;
        }
        if (UiInput::pressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, exitDialogButtonRect(true))) {
                UiInput::consumeClick();
                m_exitDialogOpen = false;
                return MenuAction::EXIT;
            }
            if (CheckCollisionPointRec(mouse, exitDialogButtonRect(false))) {
                UiInput::consumeClick();
                m_exitDialogOpen = false;
                return MenuAction::NONE;
            }
            UiInput::consumeClick();
        }
        return MenuAction::NONE;
    }

    // ---- Ban phim ----
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) m_selected = (m_selected + 1) % count;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) m_selected = (m_selected + count - 1) % count;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
        const MenuAction action = m_buttons[static_cast<size_t>(m_selected)].m_action;
        if (action == MenuAction::EXIT) {
            m_exitDialogOpen = true;
            return MenuAction::NONE;
        }
        return action;
    }

    // ---- Chuot: di chuyen thi chon nut dang tro, bam trai thi kich hoat ----
    const Vector2 mouse = UiInput::mousePosition();
    const Vector2 moved = UiInput::mouseDelta();
    for (int i = 0; i < count; ++i) {
        if (!CheckCollisionPointRec(mouse, buttonRect(static_cast<size_t>(i)))) continue;
        UiInput::setCursorType(UiInput::CursorType::POINTER);
        if (moved.x != 0.0f || moved.y != 0.0f) m_selected = i;
        if (UiInput::pressed(MOUSE_BUTTON_LEFT)) {
            UiInput::consumeClick();
            UiInput::playClickSound();
            m_selected = i;
            if (m_buttons[static_cast<size_t>(i)].m_action == MenuAction::EXIT) {
                m_exitDialogOpen = true;
                return MenuAction::NONE;
            }
            return m_buttons[static_cast<size_t>(i)].m_action;
        }
    }
    return MenuAction::NONE;
}

// ============ Ve ============
void StartMenu::draw() const {
    drawBackground();
    drawTitle();
    drawButtons();

    // Fade-in tu mau den khi vao menu
    const float fade = 1.0f - clamp01(m_time / FADE_IN_SECONDS);
    if (fade > 0.0f)
        DrawRectangle(0, 0, UiCanvas::width(), UiCanvas::height(),
                      Color{ 0, 0, 0, static_cast<unsigned char>(255 * fade) });
    if (m_exitDialogOpen) drawExitDialog();
}

void StartMenu::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{
            0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H) });
    } else {
        drawProceduralBackground();
    }

    DrawCircleGradient(Vector2{ W * m_menuCenterX, H * 0.39f }, 500.0f,
                       Color{ 0, 0, 0, 150 }, Color{ 0, 0, 0, 0 });

    // Dom dom bay nhe (tinh tu thoi gian, khong luu trang thai)
    for (int i = 0; i < 18; ++i) {
        const float t = m_time * (0.25f + 0.5f * hash01(i * 3 + 1)) + hash01(i * 7) * 20.0f;
        const float x = hash01(i * 11 + 2) * W + std::sin(t) * 24.0f;
        const float y = H * (0.55f + 0.38f * hash01(i * 5 + 3)) + std::cos(t * 0.8f) * 14.0f;
        const float glow = 0.5f + 0.5f * std::sin(m_time * 2.0f + i);
        DrawCircleV(Vector2{ x, y }, 2.0f + glow, COL_EMBER(static_cast<unsigned char>(80 + 150 * glow)));
    }

    // Toi nhe phia tren/duoi de chu de doc
    DrawRectangleGradientV(0, 0, W, H / 3, Color{ 0, 0, 0, 90 }, Color{ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, H * 2 / 3, W, H - H * 2 / 3, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 120 });
}

// Nen thu tuc khi chua co anh: bau troi dem + trang + sao nhap nhay + nui
void StartMenu::drawProceduralBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    const int horizon = static_cast<int>(H * 0.68f);

    DrawRectangleGradientV(0, 0, W, horizon, Color{ 10, 8, 36, 255 }, Color{ 214, 104, 96, 255 });
    for (int i = 0; i < 90; ++i) {
        const float tw = 0.5f + 0.5f * std::sin(m_time * (1.0f + hash01(i)) + i);
        DrawCircle(static_cast<int>(hash01(i * 2) * W), static_cast<int>(hash01(i * 2 + 1) * H * 0.5f),
                   1.0f + hash01(i + 500), Color{ 255, 255, 235, static_cast<unsigned char>(90 + 150 * tw) });
    }
    DrawCircle(static_cast<int>(W * 0.90f), static_cast<int>(H * 0.20f), 70.0f, Color{ 70, 48, 110, 255 });
    DrawCircle(static_cast<int>(W * 0.90f), static_cast<int>(H * 0.20f), 62.0f, Color{ 250, 238, 190, 255 });

    for (int i = 0; i < 9; ++i)
        DrawCircle(static_cast<int>(i * W / 8.0f), horizon + 30, 120.0f + hash01(i + 40) * 60.0f, Color{ 58, 36, 86, 255 });
    DrawRectangle(0, horizon + 20, W, H - horizon, Color{ 22, 16, 44, 255 });
    DrawRectangle(0, H - 70, W, 70, Color{ 14, 11, 30, 255 });
}

void StartMenu::drawTitle() const {
    const float W = static_cast<float>(UiCanvas::width());
    const float H = static_cast<float>(UiCanvas::height());
    const float cx = W * m_menuCenterX;
    const float bob = std::sin(m_time * 1.8f) * 4.0f;
    const float y = H * TITLE_Y_RATIO + bob;

    DrawCircleGradient(Vector2{ cx, y + 48.0f }, 190.0f,
                       Color{ 232, 131, 43, 42 }, Color{ 232, 131, 43, 0 });
    drawCenteredText(Strings::gameTitle, cx + 4.0f, y + 4.0f, 76, COL_TITLE_SHADOW);
    drawCenteredText(Strings::gameTitle, cx, y, 76, COL_TITLE);

    drawCenteredText(Strings::startMenuSubtitle, cx, y + 92.0f, 24, COL_SUBTITLE);

    // Duong ke trang tri
    const float lineW = 300.0f, lineY = y + 128.0f;
    DrawLineEx(Vector2{ cx - lineW / 2.0f, lineY }, Vector2{ cx + lineW / 2.0f, lineY }, 2.0f, Fade(UiTheme::accent, 0.65f));
}

void StartMenu::drawButtons() const {
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const Button& b = m_buttons[i];
        const bool selected = (static_cast<int>(i) == m_selected);

        // Nut dang chon phong to nhe va sang len
        Rectangle r = buttonRect(i);
        const float grow = 10.0f * b.m_hover;
        r.x -= grow; r.width += grow * 2.0f;
        const bool held = CheckCollisionPointRec(UiCanvas::mousePosition(), r) &&
                          IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (held) r.y += 2.0f;

        UiShapeEffects effects;
        effects.gradientBottom = selected ? UiTheme::primaryBottom : UiTheme::panelBottom;
        effects.shadowBlur = 8.0f;
        effects.glow = UiTheme::ember;
        effects.glowRadius = b.m_hover > 0.1f ? 5.0f : 0.0f;
        effects.glowStrength = b.m_hover * 0.45f;
        UiShapes::roundedRect(r, UiTheme::radiusSmall,
            selected ? UiTheme::primaryTop : UiTheme::panel,
            selected ? COL_BTN_EDGE_SEL : COL_BTN_EDGE, 1.0f, effects);

        const char* iconKey = i == 0 ? "action:confirm" : "action:close";
        UiIcons::draw(iconKey, Rectangle{ r.x + 22.0f, r.y + 12.0f, 24.0f, 24.0f });
        const int fontSize = 24;
        const float textY = r.y + (r.height - fontSize) / 2.0f;
        drawCenteredText(b.m_label.c_str(), r.x + r.width / 2.0f + 12.0f, textY, fontSize,
                         selected ? COL_TEXT_SEL : COL_TEXT, fontSize * 0.04f, UiFontRole::HUD);

        if (selected) {   // mui ten hai ben
            const float ax = 16.0f + 4.0f * std::sin(m_time * 6.0f);
            const float cy = r.y + r.height / 2.0f;
            DrawTriangle(Vector2{ r.x + 12.0f, cy - 9.0f }, Vector2{ r.x + 12.0f, cy + 9.0f }, Vector2{ r.x + ax + 10.0f, cy }, COL_BTN_EDGE_SEL);
            DrawTriangle(Vector2{ r.x + r.width - 12.0f, cy + 9.0f }, Vector2{ r.x + r.width - 12.0f, cy - 9.0f },
                         Vector2{ r.x + r.width - ax - 10.0f, cy }, COL_BTN_EDGE_SEL);
        }
    }
}

void StartMenu::drawExitDialog() const {
        const Rectangle dialog = exitDialogRect();
        DrawRectangle(0, 0, UiCanvas::width(), UiCanvas::height(), Color{ 4, 3, 8, 190 });
        UiShapeEffects effects;
        effects.gradientBottom = UiTheme::panelBottom;
        effects.shadowBlur = UiTheme::floatingShadowSize;
        UiShapes::roundedRect(dialog, UiTheme::radiusLarge, UiTheme::panel,
                              UiTheme::border, 1.0f, effects);
        UiText::draw(Strings::exitConfirmTitle, static_cast<int>(dialog.x + 24.0f),
                     static_cast<int>(dialog.y + 22.0f), 26, UiTheme::accent, UiFontRole::DISPLAY);
        UiText::draw(Strings::exitConfirmBody, static_cast<int>(dialog.x + 24.0f),
                     static_cast<int>(dialog.y + 68.0f), 18, UiTheme::text, UiFontRole::BODY);

        const Rectangle cancel = exitDialogButtonRect(false);
        const Rectangle confirm = exitDialogButtonRect(true);
        const bool cancelHover = CheckCollisionPointRec(UiInput::mousePosition(), cancel);
        const bool confirmHover = CheckCollisionPointRec(UiInput::mousePosition(), confirm);
        if (cancelHover || confirmHover) UiInput::setCursorType(UiInput::CursorType::POINTER);
        UiShapes::roundedRect(cancel, UiTheme::radiusSmall,
                              cancelHover ? UiTheme::panel : UiTheme::panelBottom,
                              cancelHover ? UiTheme::accent : UiTheme::secondaryBorder, 1.0f);
        UiShapes::roundedRect(confirm, UiTheme::radiusSmall,
                              confirmHover ? UiTheme::danger : UiTheme::dangerTop,
                              UiTheme::danger, 1.0f);
        drawCenteredText(Strings::cancel, cancel.x + cancel.width / 2.0f,
                         cancel.y + 12.0f, 20, UiTheme::text, 0.8f, UiFontRole::HUD);
        drawCenteredText(Strings::confirmExit, confirm.x + confirm.width / 2.0f,
                         confirm.y + 12.0f, 20, WHITE, 0.8f, UiFontRole::HUD);
}
