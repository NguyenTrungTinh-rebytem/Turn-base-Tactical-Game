#pragma once

#include "raylib.h"
#include <cstdint>

namespace UiInput {
enum class CursorType : int { NORMAL, POINTER, TARGET, DISABLED };

void beginFrame();
Vector2 mousePosition();
Vector2 logicalMousePosition();
Vector2 mouseDelta();
bool hovered(Rectangle bounds, const char* id = nullptr);
const char* hoveredId();
void setCursorType(CursorType cursorType);
CursorType cursorType();
bool pressed(MouseButton button = MOUSE_BUTTON_LEFT);
bool down(MouseButton button = MOUSE_BUTTON_LEFT);
bool released(MouseButton button = MOUSE_BUTTON_LEFT);
bool doubleClicked(MouseButton button = MOUSE_BUTTON_LEFT);
bool leftClicked();
bool rightClicked();
float wheel();
void consumeClick(MouseButton button = MOUSE_BUTTON_LEFT);
bool clickConsumed();
bool clickConsumed(MouseButton button);
void playHoverSound();
void playClickSound();
bool keyPressed(KeyboardKey key);
}
