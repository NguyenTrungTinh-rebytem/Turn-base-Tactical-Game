#pragma once

#include "raylib.h"
#include <string>

namespace UiText {
void draw(const char* text, int x, int y, int fontSize, Color tint, float trackingEm = 0.0f);
void draw(const std::string& text, int x, int y, int fontSize, Color tint, float trackingEm = 0.0f);
int measure(const char* text, int fontSize, float trackingEm = 0.0f);
int measure(const std::string& text, int fontSize, float trackingEm = 0.0f);
}
