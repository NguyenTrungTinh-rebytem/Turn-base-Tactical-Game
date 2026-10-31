#include "graphics/ui/UiInput.h"
#include "graphics/ui/UiFrame.h"
#include "graphics/ui/UiWidgets.h"
#include "graphics/ui/UiTheme.h"
#include "graphics/ui/UiText.h"
#include "graphics/ui/Strings.h"
// Member 4 - Huynh Quang Khai | src/graphics/StartMenu.cpp
#include "StartMenu.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720, tinh theo kich thuoc cua so) ----
constexpr float TITLE_Y_RATIO   = 0.17f;
constexpr float BUTTON_W        = 340.0f;
constexpr float BUTTON_H        = 58.0f;
constexpr float BUTTON_GAP      = 22.0f;
constexpr float BUTTONS_Y_RATIO = 0.50f;
constexpr float FADE_IN_SECONDS = 0.8f;
constexpr float HOVER_SPEED     = 10.0f;

// ---- Mau ----
constexpr Color COL_TITLE = UiTheme::AGED_GOLD;
constexpr Color COL_TITLE_SHADOW { 15, 18, 23, 235 };
constexpr Color COL_SUBTITLE = UiTheme::MUTED;
constexpr Color COL_BTN          { 18, 14, 40, 190 };
constexpr Color COL_BTN_SELECTED { 90, 60, 20, 230 };
constexpr Color COL_BTN_EDGE     { 140, 120, 180, 255 };
constexpr Color COL_BTN_EDGE_SEL = UiTheme::EMBER;
constexpr Color COL_TEXT = UiTheme::TEXT;
constexpr Color COL_TEXT_SEL = UiTheme::AGED_GOLD;
constexpr Color COL_OK           { 120, 230, 140, 255 };
constexpr Color COL_ERR          { 255, 110, 110, 255 };
constexpr Color COL_EMBER(unsigned char alpha) { return Color{ 193, 139, 97, alpha }; }   // tan lua (doi sang 190,255,120 neu muon dom dom xanh)

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// So gia ngau nhien co dinh theo chi so (khong can luu trang thai)
float hash01(int n) {
    unsigned int x = static_cast<unsigned int>(n) * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    return static_cast<float>(x & 0xFFFFu) / 65535.0f;
}

void drawCenteredText(const char* text, float centerX, float y, int size, Color color) {
    const int w = UiText::measure(text, size);
    UiText::draw(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), size, color);
}
} // namespace

StartMenu::StartMenu() {
    m_buttons.push_back({ Strings::get(Strings::Key::START), MenuAction::START_GAME, 0.0f });
    m_buttons.push_back({ Strings::get(Strings::Key::EXIT),   MenuAction::EXIT,       0.0f });
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

void StartMenu::setStatusText(const std::string& text, bool isOk) {
    m_statusText = text;
    m_statusOk = isOk;
}

Rectangle StartMenu::buttonRect(size_t index) const {
    const float W = static_cast<float>(UiTheme::WIDTH);
    const float H = static_cast<float>(UiTheme::HEIGHT);
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
    if (m_buttons.empty()) return MenuAction::NONE;
    const int count = static_cast<int>(m_buttons.size());
    auto& input = UiInput::instance();
    if (m_confirmExit) {
        if (input.click({450, 398, 172, 55}) || IsKeyPressed(KEY_ESCAPE)) {
            m_confirmExit = false;
            return MenuAction::NONE;
        }
        if (input.click({658, 398, 172, 55})) return MenuAction::EXIT;
        return MenuAction::NONE;
    }

    // Revision 3 ships Vietnamese-only until battle logs and CSV are fully translated.
    // ---- Ban phim ----
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) m_selected = (m_selected + 1) % count;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) m_selected = (m_selected + count - 1) % count;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (m_buttons[static_cast<size_t>(m_selected)].m_action == MenuAction::EXIT) {
            m_confirmExit = true; return MenuAction::NONE;
        }
        return m_buttons[static_cast<size_t>(m_selected)].m_action;
    }

    // ---- Chuot: di chuyen thi chon nut dang tro, bam trai thi kich hoat ----
    const Vector2 mouse = UiInput::instance().mouse();
    const Vector2 moved = GetMouseDelta();
    for (int i = 0; i < count; ++i) {
        if (!CheckCollisionPointRec(mouse, buttonRect(static_cast<size_t>(i)))) continue;
        if (moved.x != 0.0f || moved.y != 0.0f) m_selected = i;
        if (input.click(buttonRect(static_cast<size_t>(i)))) {
            m_selected = i;
            if (m_buttons[static_cast<size_t>(i)].m_action == MenuAction::EXIT) { m_confirmExit = true; return MenuAction::NONE; }
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
    drawFooter();
    if (m_confirmExit) {
        DrawRectangle(0, 0, UiTheme::WIDTH, UiTheme::HEIGHT, Color{0,0,0,178});
        UiFrame::panel({390,275,500,215},true);
        drawCenteredText(Strings::get(Strings::Key::EXIT_CONFIRM),640,310,32,UiTheme::AGED_GOLD);
        UiWidgets::button({450,398,172,55},Strings::get(Strings::Key::STAY), UiInput::instance().hover({450,398,172,55}));
        UiWidgets::button({658,398,172,55},Strings::get(Strings::Key::EXIT), UiInput::instance().hover({658,398,172,55}),false,true,true);
    }

    // Fade-in tu mau den khi vao menu
    const float fade = 1.0f - clamp01(m_time / FADE_IN_SECONDS);
    if (fade > 0.0f)
        DrawRectangle(0, 0, UiTheme::WIDTH, UiTheme::HEIGHT,
                      Color{ 0, 0, 0, static_cast<unsigned char>(255 * fade) });
}

void StartMenu::drawBackground() const {
    const int W = UiTheme::WIDTH, H = UiTheme::HEIGHT;
    if (m_hasBackground) {
        Rectangle src = { 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) };
        Rectangle dst = { 0, 0, static_cast<float>(W), static_cast<float>(H) };
        DrawTexturePro(m_background, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        drawProceduralBackground();
    }

    // Dom dom bay nhe (tinh tu thoi gian, khong luu trang thai)
    for (int i = 0; i < 7; ++i) {
        const float t = m_time * (0.25f + 0.5f * hash01(i * 3 + 1)) + hash01(i * 7) * 20.0f;
        const float x = hash01(i * 11 + 2) * W + std::sin(t) * 24.0f;
        const float y = H * (0.55f + 0.38f * hash01(i * 5 + 3)) + std::cos(t * 0.8f) * 14.0f;
        const float glow = 0.5f + 0.5f * std::sin(m_time * 2.0f + i);
        DrawCircleV(Vector2{ x, y }, 1.2f + glow, COL_EMBER(static_cast<unsigned char>(35 + 55 * glow)));
    }

    // Toi nhe phia tren/duoi de chu de doc
    DrawRectangleGradientV(0, 0, W, H / 3, Color{ 0, 0, 0, 90 }, Color{ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, H * 2 / 3, W, H - H * 2 / 3, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 120 });
}

// Nen thu tuc khi chua co anh: bau troi dem + trang + sao nhap nhay + nui
void StartMenu::drawProceduralBackground() const {
    const int W = UiTheme::WIDTH, H = UiTheme::HEIGHT;
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
    const float W = static_cast<float>(UiTheme::WIDTH);
    const float H = static_cast<float>(UiTheme::HEIGHT);
    const float cx = W * m_menuCenterX;
    const float bob = 0.f; // stable title improves long-session comfort
    const float y = H * TITLE_Y_RATIO + bob;

    UiFrame::panel({cx-330,y-24,660,162},false);
    // Bong + chu vang
    drawCenteredText("THE FIRST STEP", cx + 4.0f, y + 4.0f, 76, COL_TITLE_SHADOW);
    drawCenteredText("THE FIRST STEP", cx, y, 76, COL_TITLE);

    drawCenteredText("2.5D PIXEL TURN-BASED RPG", cx, y + 92.0f, 24, COL_SUBTITLE);

    // Duong ke trang tri
    const float lineW = 300.0f, lineY = y + 128.0f;
    (void)lineW; (void)lineY; // no decorative line crossing content
}

void StartMenu::drawButtons() const {
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const Button& b = m_buttons[i];
        const bool selected = (static_cast<int>(i) == m_selected);

        // Nut dang chon phong to nhe va sang len
        Rectangle r = buttonRect(i);
        const float grow = 3.0f * b.m_hover;
        r.x -= grow; r.width += grow * 2.0f;

        UiFrame::plaque(r, selected, i == 1, b.m_hover);

        const int fontSize = 28;
        const float textY = r.y + (r.height - fontSize) / 2.0f;
        drawCenteredText(Strings::get(b.m_action==MenuAction::EXIT?Strings::Key::EXIT:Strings::Key::START), r.x + r.width / 2.0f, textY, fontSize, selected ? COL_TEXT_SEL : COL_TEXT);

        if (selected) {   // mui ten hai ben
            const float ax = 16.0f;
            const float cy = r.y + r.height / 2.0f;
            DrawTriangle(Vector2{ r.x + 12.0f, cy - 9.0f }, Vector2{ r.x + 12.0f, cy + 9.0f }, Vector2{ r.x + ax + 10.0f, cy }, COL_BTN_EDGE_SEL);
            DrawTriangle(Vector2{ r.x + r.width - 12.0f, cy + 9.0f }, Vector2{ r.x + r.width - 12.0f, cy - 9.0f },
                         Vector2{ r.x + r.width - ax - 10.0f, cy }, COL_BTN_EDGE_SEL);
        }
    }
}

void StartMenu::drawFooter() const {
    const float W = static_cast<float>(UiTheme::WIDTH);
    const float H = static_cast<float>(UiTheme::HEIGHT);

    drawCenteredText("", W / 2.0f, H - 34.0f, 18, Color{ 200, 195, 225, 220 });
    if (!m_statusText.empty())
        UiText::draw(m_statusText.c_str(), 20, static_cast<int>(H) - 30, 16, m_statusOk ? COL_OK : COL_ERR);
    const char* credit = "ĐT10 · RPG chiến thuật";
    UiText::draw(credit, static_cast<int>(W) - UiText::measure(credit, 16) - 20, static_cast<int>(H) - 30, 16, UiTheme::MUTED);
}