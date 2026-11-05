// Member 4 - Huynh Quang Khai | src/graphics/StartMenu.cpp
#include "StartMenu.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiBlur.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
constexpr float BUTTON_W        = 306.0f;
constexpr float BUTTON_H        = 38.0f;
constexpr float BUTTON_GAP      = 10.0f;
constexpr float BUTTONS_Y_RATIO = 0.37f;
constexpr float FADE_IN_SECONDS = 0.8f;
constexpr float HOVER_SPEED     = 10.0f;

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// So gia ngau nhien co dinh theo chi so (khong can luu trang thai)
float hash01(int n) {
    unsigned int x = static_cast<unsigned int>(n) * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    return static_cast<float>(x & 0xFFFFu) / 65535.0f;
}

void drawCenteredText(const char* text, float centerX, float y, int size, Color color,
                      float trackingEm = 0.0f, bool shadow = true) {
    const int width = UiText::measure(text, size, trackingEm);
    const int x = static_cast<int>(centerX - width * 0.5f);
    if (shadow)
        UiText::draw(text, x + 1, static_cast<int>(y) + 1, size, Color{ 0, 0, 0, color.a },
                     trackingEm);
    UiText::draw(text, x, static_cast<int>(y), size, color, trackingEm);
}

void sampleMenuSide(Texture2D texture, bool& right, float& centerX) {
    Image image = LoadImageFromTexture(texture);
    if (!image.data || image.width < 2 || image.height < 2) {
        std::cerr << "StartMenu: unable to sample background luminance; using right-side layout\n";
        right = true;
        centerX = 0.82f;
        UnloadImage(image);
        return;
    }
    Color* pixels = LoadImageColors(image);
    if (!pixels) {
        std::cerr << "StartMenu: unable to read background pixels; using right-side layout\n";
        UnloadImage(image);
        right = true;
        centerX = 0.82f;
        return;
    }

    double luminance[2]{ 0.0, 0.0 };
    std::size_t count[2]{ 0, 0 };
    for (int y = image.height / 10; y < image.height * 9 / 10; y += 4) {
        for (int x = 0; x < image.width; x += 4) {
            const int side = x < image.width / 2 ? 0 : 1;
            const Color color = pixels[static_cast<std::size_t>(y) * image.width + x];
            luminance[side] += 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
            ++count[side];
        }
    }
    right = luminance[1] / static_cast<double>(count[1]) <
            luminance[0] / static_cast<double>(count[0]);
    centerX = right ? 0.82f : 0.18f;
    UnloadImageColors(pixels);
    UnloadImage(image);
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
    if (m_hasBackground) {
        SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);
        sampleMenuSide(m_background, m_menuOnRight, m_menuCenterX);
    }
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

void StartMenu::update(float deltaTime) {
    m_time += deltaTime;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const float target = (static_cast<int>(i) == m_selected) ? 1.0f : 0.0f;
        m_buttons[i].m_hover += (target - m_buttons[i].m_hover) * std::min(1.0f, HOVER_SPEED * deltaTime);
    }
}

MenuAction StartMenu::handleInput() {
    if (m_confirmExit) {
        const Rectangle dialog{ 420.0f, 248.0f, 440.0f, 224.0f };
        const Rectangle buttons[] = {
            { dialog.x + 40.0f, dialog.y + 154.0f, 160.0f, 42.0f },
            { dialog.x + 240.0f, dialog.y + 154.0f, 160.0f, 42.0f }
        };
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 mouse = UiCanvas::mousePosition();
            if (CheckCollisionPointRec(mouse, buttons[0])) return MenuAction::EXIT;
            if (CheckCollisionPointRec(mouse, buttons[1])) m_confirmExit = false;
        }
        return MenuAction::NONE;
    }

    if (m_buttons.empty()) return MenuAction::NONE;
    const int count = static_cast<int>(m_buttons.size());

    const Vector2 mouse = UiCanvas::mousePosition();
    const Vector2 moved = UiCanvas::mouseDelta();
    for (int i = 0; i < count; ++i) {
        if (!CheckCollisionPointRec(mouse, buttonRect(static_cast<size_t>(i)))) continue;
        if (moved.x != 0.0f || moved.y != 0.0f) m_selected = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_selected = i;
            const MenuAction action = m_buttons[static_cast<size_t>(i)].m_action;
            if (action == MenuAction::EXIT) {
                m_confirmExit = true;
                return MenuAction::NONE;
            }
            return action;
        }
    }
    return MenuAction::NONE;
}

// ============ Ve ============
void StartMenu::draw() const {
    drawBackground();
    drawTitle();
    drawButtons();
    drawFooter();

    const float fade = 1.0f - clamp01(m_time / FADE_IN_SECONDS);
    if (fade > 0.0f)
        DrawRectangle(0, 0, UiCanvas::width(), UiCanvas::height(),
                      Color{ 0, 0, 0, static_cast<unsigned char>(255 * fade) });
    if (m_confirmExit) {
        const Rectangle dialog{ 420.0f, 248.0f, 440.0f, 224.0f };
        UiBlur::draw(Rectangle{ 0.0f, 0.0f, 1280.0f, 720.0f }, 0.0f, Color{ 6, 8, 18, 175 });
        UiShapes::plate(dialog, UiTheme::panelRaised, UiTheme::crimson, 1.5f);
        drawCenteredText(Strings::exitQuestion, dialog.x + dialog.width / 2.0f,
                         dialog.y + 30.0f, 26, UiTheme::silver,
                         UiTheme::displayTracking);
        drawCenteredText(Strings::exitWarning, dialog.x + dialog.width / 2.0f,
                         dialog.y + 84.0f, 15, UiTheme::ashText);
        const Rectangle buttons[] = {
            { dialog.x + 40.0f, dialog.y + 154.0f, 160.0f, 42.0f },
            { dialog.x + 240.0f, dialog.y + 154.0f, 160.0f, 42.0f }
        };
        for (int i = 0; i < 2; ++i) {
            const bool over = CheckCollisionPointRec(UiCanvas::mousePosition(), buttons[i]);
            const bool selected = i == 1;
            UiShapes::bladeStrip(buttons[i], selected ? Color{ 50, 10, 17, 224 } : UiTheme::panel,
                selected ? UiTheme::crimson : UiTheme::ash, m_menuOnRight, selected, over);
            drawCenteredText(i == 0 ? Strings::exitConfirm : Strings::stay,
                buttons[i].x + buttons[i].width * 0.5f, buttons[i].y + 11.0f, 15,
                UiTheme::silver);
        }
    }
}

void StartMenu::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{
            0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H) });
    } else {
        drawProceduralBackground();
    }

    const Color transparent{ 7, 6, 10, 0 };
    const Color vignette{ 7, 6, 10, 190 };
    if (m_menuOnRight)
        DrawRectangleGradientH(0, 0, W, H, transparent, vignette);
    else
        DrawRectangleGradientH(0, 0, W, H, vignette, transparent);
    DrawRectangleGradientV(0, 0, W, H / 4,
        Color{ 0, 0, 0, 120 }, Color{ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, H * 3 / 4, W, H / 4,
        Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 145 });
}

// Nen thu tuc khi chua co anh: bau troi dem + trang + sao nhap nhay + nui
void StartMenu::drawProceduralBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    const int horizon = static_cast<int>(H * 0.68f);

    DrawRectangleGradientV(0, 0, W, horizon, Color{ 13, 11, 13, 255 },
                           Color{ 69, 19, 28, 255 });
    for (int i = 0; i < 90; ++i) {
        const float tw = 0.5f + 0.5f * std::sin(m_time * (1.0f + hash01(i)) + i);
        DrawCircle(static_cast<int>(hash01(i * 2) * W), static_cast<int>(hash01(i * 2 + 1) * H * 0.5f),
                   1.0f + hash01(i + 500), Color{ 255, 255, 235, static_cast<unsigned char>(90 + 150 * tw) });
    }
    DrawCircle(static_cast<int>(W * 0.90f), static_cast<int>(H * 0.20f), 70.0f, Color{ 82, 20, 28, 255 });
    DrawCircle(static_cast<int>(W * 0.90f), static_cast<int>(H * 0.20f), 62.0f, UiTheme::silver);

    for (int i = 0; i < 9; ++i)
        DrawCircle(static_cast<int>(i * W / 8.0f), horizon + 30,
                   120.0f + hash01(i + 40) * 60.0f, Color{ 31, 15, 21, 255 });
    DrawRectangle(0, horizon + 20, W, H - horizon, Color{ 13, 10, 13, 255 });
    DrawRectangle(0, H - 70, W, 70, UiTheme::ink);
}

void StartMenu::drawTitle() const {
    const float W = static_cast<float>(UiCanvas::width());
    const float cx = W * m_menuCenterX;
    const Vector2 emblemCenter{ cx, 45.0f };
    const Color emblemColor{ UiTheme::ash.r, UiTheme::ash.g, UiTheme::ash.b, 76 };
    DrawCircleLines(static_cast<int>(emblemCenter.x), static_cast<int>(emblemCenter.y),
                    30.0f, emblemColor);
    DrawCircleLines(static_cast<int>(emblemCenter.x), static_cast<int>(emblemCenter.y),
                    24.0f, emblemColor);
    for (int tick = 0; tick < 8; ++tick) {
        const float angle = static_cast<float>(tick) * PI / 4.0f;
        const Vector2 from{ emblemCenter.x + std::cos(angle) * 32.0f,
                            emblemCenter.y + std::sin(angle) * 32.0f };
        const Vector2 to{ emblemCenter.x + std::cos(angle) * 38.0f,
                          emblemCenter.y + std::sin(angle) * 38.0f };
        DrawLineEx(from, to, 1.0f, emblemColor);
    }
    drawCenteredText(Strings::gameTitle, cx, 82.0f, 31, UiTheme::silver,
                     UiTheme::displayTracking);
    drawCenteredText(Strings::menuTagline, cx, 148.0f, 10, UiTheme::ashText,
                     UiTheme::hudTracking);
    UiShapes::divider(Vector2{ cx - 114.0f, 176.0f },
                      Vector2{ cx + 114.0f, 176.0f }, UiTheme::ash);
}

void StartMenu::drawButtons() const {
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const Button& b = m_buttons[i];
        const bool selected = (static_cast<int>(i) == m_selected);

        Rectangle r = buttonRect(i);
        r.x += (m_menuOnRight ? -1.0f : 1.0f) * 8.0f * b.m_hover;
        const bool over = CheckCollisionPointRec(UiCanvas::mousePosition(), buttonRect(i));
        UiShapes::bladeStrip(r, selected ? Color{ 50, 10, 17, 224 } : UiTheme::panel,
            selected ? UiTheme::crimson : UiTheme::ash, m_menuOnRight,
            selected, over, true);

        const bool start = b.m_action == MenuAction::START_GAME;
        const float iconSize = 24.0f;
        const float iconX = r.x + 16.0f;
        UiIcons::draw(start ? "action:start" : "action:exit",
            Rectangle{ iconX, r.y + (r.height - iconSize) * 0.5f, iconSize, iconSize },
            UiTheme::silver);
        const int fontSize = 20;
        UiText::draw(b.m_label, static_cast<int>(iconX + iconSize + 12.0f),
            static_cast<int>(r.y + (r.height - fontSize) * 0.5f),
            fontSize, UiTheme::silver, UiTheme::displayTracking);
    }
}

void StartMenu::drawFooter() const {
    const float x = 56.0f;
    const float y = 614.0f;
    DrawLineEx(Vector2{ x, y - 2.0f }, Vector2{ x, y + 48.0f }, 1.0f, UiTheme::ash);
    UiShapes::diamond(Vector2{ x, y - 2.0f }, 7.0f, UiTheme::crimson);
    UiText::draw(Strings::menuLoreLine1, static_cast<int>(x + 17.0f),
                 static_cast<int>(y + 1.0f), 13, Color{ 0, 0, 0, 210 });
    UiText::draw(Strings::menuLoreLine1, static_cast<int>(x + 16.0f),
                 static_cast<int>(y), 13, UiTheme::ashText);
    UiText::draw(Strings::menuLoreLine2, static_cast<int>(x + 17.0f),
                 static_cast<int>(y + 23.0f), 13, Color{ 0, 0, 0, 210 });
    UiText::draw(Strings::menuLoreLine2, static_cast<int>(x + 16.0f),
                 static_cast<int>(y + 22.0f), 13, UiTheme::silver);
}