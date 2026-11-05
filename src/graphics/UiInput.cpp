#include "UiInput.h"
#include "UiCanvas.h"

namespace UiInput {
Vector2 mousePosition() { return UiCanvas::mousePosition(); }
bool pressed(MouseButton button) { return IsMouseButtonPressed(button); }
bool down(MouseButton button) { return IsMouseButtonDown(button); }
bool released(MouseButton button) { return IsMouseButtonReleased(button); }
float wheel() { return GetMouseWheelMove(); }
bool keyPressed(KeyboardKey key) { return IsKeyPressed(key); }
}
