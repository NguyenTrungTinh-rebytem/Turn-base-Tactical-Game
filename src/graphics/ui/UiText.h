#pragma once
#include "raylib.h"
#include <string>

namespace UiText {
// Drop-in legacy coordinate adapters; each routes to DrawTextEx/MeasureTextEx.
void draw(const char* text, int x, int y, int fontSize, Color color);
int measure(const char* text, int fontSize);
Vector2 measureEx(const std::string& text, int fontSize);
void drawShadowed(const std::string& text, int x, int y, int size, Color color);
void drawWrapped(const std::string& text, Rectangle bounds, int fontSize, Color color);
void drawEllipsis(const std::string& text, Rectangle bounds, int fontSize, Color color);
void drawFontTest();
}
