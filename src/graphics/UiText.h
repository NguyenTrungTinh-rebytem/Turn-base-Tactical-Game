#pragma once

#include "FontManager.h"
#include "raylib.h"
#include <string>

namespace UiText {
void draw(const char* text, int x, int y, int fontSize, Color tint,
          UiFontRole role = UiFontRole::AUTO, float letterSpacing = 0.0f);
void draw(const std::string& text, int x, int y, int fontSize, Color tint,
          UiFontRole role = UiFontRole::AUTO, float letterSpacing = 0.0f);
void drawShadowed(const std::string& text, int x, int y, int fontSize, Color tint,
                  UiFontRole role = UiFontRole::AUTO, float letterSpacing = 0.0f);
void drawOutlined(const std::string& text, int x, int y, int fontSize, Color tint,
                  Color outline = Color{ 10, 12, 20, 255 }, int thickness = 2,
                  UiFontRole role = UiFontRole::AUTO);
int measure(const char* text, int fontSize, UiFontRole role = UiFontRole::AUTO,
            float letterSpacing = 0.0f);
int measure(const std::string& text, int fontSize, UiFontRole role = UiFontRole::AUTO,
            float letterSpacing = 0.0f);
int drawWrapped(const std::string& text, int x, int y, int maxWidth, int fontSize,
                Color tint, int lineHeight = 0, UiFontRole role = UiFontRole::AUTO);
std::string ellipsis(const std::string& text, int maxWidth, int fontSize,
                     UiFontRole role = UiFontRole::AUTO);
void drawEllipsis(const std::string& text, int x, int y, int maxWidth, int fontSize,
                  Color tint, UiFontRole role = UiFontRole::AUTO);
}
