#include "UiInput.h"
#include "UiCanvas.h"
#include "UiTheme.h"
#include <array>
#include <cmath>
#include <cstring>

namespace UiInput {
namespace {
constexpr int BUTTON_COUNT = 3;
struct ButtonState {
    bool pressed = false;
    bool down = false;
    bool released = false;
    bool doubleClicked = false;
    bool consumed = false;
    double lastClickTime = -1.0;
    Vector2 lastClickPosition{};
};
std::array<ButtonState, BUTTON_COUNT> buttons{};
Vector2 position{};
Vector2 delta{};
float wheelDelta = 0.0f;
std::array<char, 128> currentHoveredId{};
CursorType currentCursorType = CursorType::NORMAL;

int buttonIndex(MouseButton button) {
    const int index = static_cast<int>(button);
    return index >= 0 && index < BUTTON_COUNT ? index : -1;
}
}

void beginFrame() {
    position = UiCanvas::mousePosition();
    delta = UiCanvas::mouseDelta();
    wheelDelta = GetMouseWheelMove();
    currentHoveredId[0] = '\0';
    currentCursorType = CursorType::NORMAL;
    const double now = GetTime();
    for (int index = 0; index < BUTTON_COUNT; ++index) {
        ButtonState& state = buttons[static_cast<std::size_t>(index)];
        const MouseButton button = static_cast<MouseButton>(index);
        state.pressed = IsMouseButtonPressed(button);
        state.down = IsMouseButtonDown(button);
        state.released = IsMouseButtonReleased(button);
        state.consumed = false;
        state.doubleClicked = false;
        if (state.pressed) {
            const float dx = position.x - state.lastClickPosition.x;
            const float dy = position.y - state.lastClickPosition.y;
            state.doubleClicked = state.lastClickTime >= 0.0 &&
                now - state.lastClickTime <= 0.35 &&
                dx * dx + dy * dy <= 24.0f * 24.0f;
            state.lastClickTime = now;
            state.lastClickPosition = position;
        }
    }
}

Vector2 mousePosition() { return position; }
Vector2 logicalMousePosition() {
    return { position.x * static_cast<float>(UiTheme::CANVAS_WIDTH) /
                          static_cast<float>(UiCanvas::width()),
             position.y * static_cast<float>(UiTheme::CANVAS_HEIGHT) /
                          static_cast<float>(UiCanvas::height()) };
}
Vector2 mouseDelta() { return delta; }
bool hovered(Rectangle bounds, const char* id) {
    const bool over = CheckCollisionPointRec(position, bounds);
    if (over && id) {
        std::strncpy(currentHoveredId.data(), id, currentHoveredId.size() - 1);
        currentHoveredId.back() = '\0';
    }
    return over;
}
const char* hoveredId() { return currentHoveredId.data(); }
void setCursorType(CursorType cursor) { currentCursorType = cursor; }
CursorType cursorType() { return currentCursorType; }
bool pressed(MouseButton button) {
    const int index = buttonIndex(button);
    return index >= 0 && buttons[static_cast<std::size_t>(index)].pressed &&
           !buttons[static_cast<std::size_t>(index)].consumed;
}
bool down(MouseButton button) {
    const int index = buttonIndex(button);
    return index >= 0 && buttons[static_cast<std::size_t>(index)].down;
}
bool released(MouseButton button) {
    const int index = buttonIndex(button);
    return index >= 0 && buttons[static_cast<std::size_t>(index)].released;
}
bool doubleClicked(MouseButton button) {
    const int index = buttonIndex(button);
    return index >= 0 && buttons[static_cast<std::size_t>(index)].doubleClicked &&
           !buttons[static_cast<std::size_t>(index)].consumed;
}
bool leftClicked() { return pressed(MOUSE_BUTTON_LEFT); }
bool rightClicked() { return pressed(MOUSE_BUTTON_RIGHT); }
void consumeClick(MouseButton button) {
    const int index = buttonIndex(button);
    if (index >= 0) buttons[static_cast<std::size_t>(index)].consumed = true;
}
bool clickConsumed() {
    for (const ButtonState& state : buttons)
        if (state.consumed) return true;
    return false;
}
bool clickConsumed(MouseButton button) {
    const int index = buttonIndex(button);
    return index >= 0 && buttons[static_cast<std::size_t>(index)].consumed;
}
float wheel() { return wheelDelta; }
void playHoverSound() {}
void playClickSound() {}
bool keyPressed(KeyboardKey key) { return IsKeyPressed(key); }
}
