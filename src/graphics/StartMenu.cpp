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
constexpr Color COL_TITLE        { 255, 214, 96, 255 };
constexpr Color COL_TITLE_SHADOW { 60, 30, 10, 255 };
constexpr Color COL_SUBTITLE     { 220, 210, 240, 255 };
constexpr Color COL_BTN          { 18, 14, 40, 190 };
constexpr Color COL_BTN_SELECTED { 90, 60, 20, 230 };
constexpr Color COL_BTN_EDGE     { 140, 120, 180, 255 };
constexpr Color COL_BTN_EDGE_SEL { 255, 214, 96, 255 };
constexpr Color COL_TEXT         { 235, 232, 250, 255 };
constexpr Color COL_TEXT_SEL     { 255, 240, 170, 255 };
constexpr Color COL_OK           { 120, 230, 140, 255 };
constexpr Color COL_ERR          { 255, 110, 110, 255 };
constexpr Color COL_EMBER(unsigned char alpha) { return Color{ 255, 140, 70, alpha }; }   // tan lua (doi sang 190,255,120 neu muon dom dom xanh)

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// So gia ngau nhien co dinh theo chi so (khong can luu trang thai)
float hash01(int n) {
    unsigned int x = static_cast<unsigned int>(n) * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    return static_cast<float>(x & 0xFFFFu) / 65535.0f;
}

void drawCenteredText(const char* text, float centerX, float y, int size, Color color) {
    const int w = MeasureText(text, size);
    DrawText(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), size, color);
}
} // namespace

StartMenu::StartMenu() {
    m_buttons.push_back({ "BAT DAU", MenuAction::START_GAME, 0.0f });
    m_buttons.push_back({ "THOAT",   MenuAction::EXIT,       0.0f });
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
    const float W = static_cast<float>(GetScreenWidth());
    const float H = static_cast<float>(GetScreenHeight());
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

    // ---- Ban phim ----
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) m_selected = (m_selected + 1) % count;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) m_selected = (m_selected + count - 1) % count;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
        return m_buttons[static_cast<size_t>(m_selected)].m_action;

    // ---- Chuot: di chuyen thi chon nut dang tro, bam trai thi kich hoat ----
    const Vector2 mouse = GetMousePosition();
    const Vector2 moved = GetMouseDelta();
    for (int i = 0; i < count; ++i) {
        if (!CheckCollisionPointRec(mouse, buttonRect(static_cast<size_t>(i)))) continue;
        if (moved.x != 0.0f || moved.y != 0.0f) m_selected = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_selected = i;
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

    // Fade-in tu mau den khi vao menu
    const float fade = 1.0f - clamp01(m_time / FADE_IN_SECONDS);
    if (fade > 0.0f)
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Color{ 0, 0, 0, static_cast<unsigned char>(255 * fade) });
}

void StartMenu::drawBackground() const {
    const int W = GetScreenWidth(), H = GetScreenHeight();
    if (m_hasBackground) {
        Rectangle src = { 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) };
        Rectangle dst = { 0, 0, static_cast<float>(W), static_cast<float>(H) };
        DrawTexturePro(m_background, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        drawProceduralBackground();
    }

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
    const int W = GetScreenWidth(), H = GetScreenHeight();
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
    const float W = static_cast<float>(GetScreenWidth());
    const float H = static_cast<float>(GetScreenHeight());
    const float cx = W * m_menuCenterX;
    const float bob = std::sin(m_time * 1.8f) * 4.0f;
    const float y = H * TITLE_Y_RATIO + bob;

    // Bong + chu vang
    drawCenteredText("THE FIRST STEP", cx + 4.0f, y + 4.0f, 76, COL_TITLE_SHADOW);
    drawCenteredText("THE FIRST STEP", cx, y, 76, COL_TITLE);

    drawCenteredText("2.5D PIXEL TURN-BASED RPG", cx, y + 92.0f, 24, COL_SUBTITLE);

    // Duong ke trang tri
    const float lineW = 300.0f, lineY = y + 128.0f;
    DrawLineEx(Vector2{ cx - lineW / 2.0f, lineY }, Vector2{ cx + lineW / 2.0f, lineY }, 2.0f, Color{ 255, 214, 96, 160 });
}

void StartMenu::drawButtons() const {
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const Button& b = m_buttons[i];
        const bool selected = (static_cast<int>(i) == m_selected);

        // Nut dang chon phong to nhe va sang len
        Rectangle r = buttonRect(i);
        const float grow = 10.0f * b.m_hover;
        r.x -= grow; r.width += grow * 2.0f;

        DrawRectangleRec(Rectangle{ r.x + 4, r.y + 5, r.width, r.height }, Color{ 0, 0, 0, 110 });   // bong
        DrawRectangleRec(r, selected ? COL_BTN_SELECTED : COL_BTN);
        DrawRectangleLinesEx(r, selected ? 3.0f : 2.0f, selected ? COL_BTN_EDGE_SEL : COL_BTN_EDGE);

        const int fontSize = 28;
        const float textY = r.y + (r.height - fontSize) / 2.0f;
        drawCenteredText(b.m_label.c_str(), r.x + r.width / 2.0f, textY, fontSize, selected ? COL_TEXT_SEL : COL_TEXT);

        if (selected) {   // mui ten hai ben
            const float ax = 16.0f + 4.0f * std::sin(m_time * 6.0f);
            const float cy = r.y + r.height / 2.0f;
            DrawTriangle(Vector2{ r.x + 12.0f, cy - 9.0f }, Vector2{ r.x + 12.0f, cy + 9.0f }, Vector2{ r.x + ax + 10.0f, cy }, COL_BTN_EDGE_SEL);
            DrawTriangle(Vector2{ r.x + r.width - 12.0f, cy + 9.0f }, Vector2{ r.x + r.width - 12.0f, cy - 9.0f },
                         Vector2{ r.x + r.width - ax - 10.0f, cy }, COL_BTN_EDGE_SEL);
        }
    }
}

void StartMenu::drawFooter() const {
    const float W = static_cast<float>(GetScreenWidth());
    const float H = static_cast<float>(GetScreenHeight());

    drawCenteredText("[UP/DOWN] Chon   [ENTER] Xac nhan   [ESC] Thoat", W / 2.0f, H - 34.0f, 18, Color{ 200, 195, 225, 220 });
    if (!m_statusText.empty())
        DrawText(m_statusText.c_str(), 20, static_cast<int>(H) - 30, 16, m_statusOk ? COL_OK : COL_ERR);
    const char* credit = "DT10 - Turn-based RPG Battle Engine";
    DrawText(credit, static_cast<int>(W) - MeasureText(credit, 14) - 20, static_cast<int>(H) - 28, 14, Color{ 160, 155, 190, 200 });
}