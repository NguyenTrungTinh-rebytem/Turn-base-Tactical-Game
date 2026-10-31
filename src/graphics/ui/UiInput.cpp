#include "UiInput.h"
#include "UiText.h"
#include "UiTheme.h"
#include <cmath>

UiInput& UiInput::instance() { static UiInput input; return input; }
void UiInput::beginFrame() {
    const float screenW = static_cast<float>(GetScreenWidth());
    const float screenH = static_cast<float>(GetScreenHeight());
    const float scale = fminf(screenW / UiTheme::WIDTH, screenH / UiTheme::HEIGHT);
    const float offsetX = (screenW - UiTheme::WIDTH * scale) * 0.5f;
    const float offsetY = (screenH - UiTheme::HEIGHT * scale) * 0.5f;
    const Vector2 physical = GetMousePosition();
    m_mouse = {(physical.x - offsetX) / scale, (physical.y - offsetY) / scale};
    m_inside = m_mouse.x >= 0 && m_mouse.y >= 0 && m_mouse.x < UiTheme::WIDTH && m_mouse.y < UiTheme::HEIGHT;
    m_leftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_inside;
    m_leftHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && m_inside;
    m_leftReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && m_inside;
    m_rightPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && m_inside;
    m_wheel = m_inside ? GetMouseWheelMove() : 0;
    m_consumed = false;
    m_pointer = false;
    m_target = false;
    if (m_leftPressed) { m_clickTime = static_cast<float>(GetTime()); m_clickPos = m_mouse; }
    if (m_leftPressed && CheckCollisionPointRec(m_mouse,{0,0,19,19})) {m_debug=!m_debug;m_consumed=true;}
}
bool UiInput::hover(Rectangle area) {
    const bool hit = m_inside && CheckCollisionPointRec(m_mouse, area);
    if (hit) m_pointer = true;
    return hit;
}
bool UiInput::click(Rectangle area) {
    if (!m_consumed && m_leftPressed && hover(area)) { m_consumed = true; return true; }
    return false;
}
void UiInput::drawCursor() const {
    if (!m_inside) return;
    const Vector2 p = m_mouse;
    const Color c = m_target ? UiTheme::DANGER : m_pointer ? UiTheme::EMBER : UiTheme::TEXT;
    if (m_target) {
        DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), 12, c);
        DrawLineEx({p.x - 18, p.y}, {p.x - 7, p.y}, 2, c);
        DrawLineEx({p.x + 7, p.y}, {p.x + 18, p.y}, 2, c);
        DrawLineEx({p.x, p.y - 18}, {p.x, p.y - 7}, 2, c);
        DrawLineEx({p.x, p.y + 7}, {p.x, p.y + 18}, 2, c);
    } else {
        DrawTriangle({p.x, p.y}, {p.x + 1, p.y + 19}, {p.x + 6, p.y + 12}, Color{0,0,0,250});
        DrawTriangle({p.x+1, p.y+1}, {p.x+2, p.y+16}, {p.x+5, p.y+10}, c);
        if (m_pointer) DrawCircleLines(static_cast<int>(p.x + 7), static_cast<int>(p.y + 16), 4, c);
    }
    float elapsed = static_cast<float>(GetTime()) - m_clickTime;
    if (elapsed >= 0 && elapsed <= 0.23f) {
        Color ripple = UiTheme::EMBER;
        ripple.a = static_cast<unsigned char>(175 * (1.0f - elapsed / 0.23f));
        DrawCircleLines(static_cast<int>(m_clickPos.x), static_cast<int>(m_clickPos.y), 5.0f + 27.0f * elapsed / 0.23f, ripple);
    }
}
void UiInput::drawFontDebug() {
    if (m_debug) UiText::drawFontTest();
    DrawCircle(15, 15, 6, m_debug ? UiTheme::EMBER : Color{150, 110, 100, 255});
}
