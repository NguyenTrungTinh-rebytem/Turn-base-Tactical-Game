#pragma once

#include "raylib.h"
#include <string>

namespace UiText {
void draw(const char* text, int x, int y, int fontSize, Color tint);
void draw(const std::string& text, int x, int y, int fontSize, Color tint);
int measure(const char* text, int fontSize);
int measure(const std::string& text, int fontSize);
}
