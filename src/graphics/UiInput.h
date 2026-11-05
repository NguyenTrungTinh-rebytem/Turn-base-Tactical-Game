#pragma once

#include "raylib.h"

namespace UiInput {
Vector2 mousePosition();
bool pressed(MouseButton button = MOUSE_BUTTON_LEFT);
bool down(MouseButton button = MOUSE_BUTTON_LEFT);
bool released(MouseButton button = MOUSE_BUTTON_LEFT);
float wheel();
bool keyPressed(KeyboardKey key);
}
