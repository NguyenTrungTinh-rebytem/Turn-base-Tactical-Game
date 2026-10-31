#pragma once
#include "raylib.h"

class UiInput {
public:
    static UiInput& instance();
    void beginFrame();
    Vector2 mouse() const { return m_mouse; }
    bool hover(Rectangle area);
    bool click(Rectangle area); // once per frame; prevents click-through
    bool rightClick() const { return m_rightPressed; }
    bool leftPressed() const { return m_leftPressed && !m_consumed; }
    bool held() const { return m_leftHeld; }
    bool released() const { return m_leftReleased; }
    float wheel() const { return m_wheel; }
    void consume() { m_consumed = true; }
    void setTargetCursor(bool enabled) { m_target = enabled; }
    void drawCursor() const;
    void drawFontDebug();
    bool isDebugShown() const { return m_debug; }
private:
    Vector2 m_mouse{};
    bool m_inside = true;
    bool m_leftPressed = false;
    bool m_leftHeld = false;
    bool m_leftReleased = false;
    bool m_rightPressed = false;
    bool m_consumed = false;
    bool m_pointer = false;
    bool m_target = false;
    bool m_debug = false;
    float m_wheel = 0.0f;
    float m_clickTime = -10.0f;
    Vector2 m_clickPos{};
};
