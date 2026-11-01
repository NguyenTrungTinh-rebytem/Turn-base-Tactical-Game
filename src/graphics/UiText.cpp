#include "UiText.h"
#include "FontManager.h"
#include <cmath>

namespace UiText {
void draw(const char* text, int x, int y, int fontSize, Color tint) {
    if (!text || fontSize <= 0) return;
    DrawTextEx(FontManager::font(), text, Vector2{ static_cast<float>(x), static_cast<float>(y) },
               static_cast<float>(fontSize), 1.0f, tint);
}

void draw(const std::string& text, int x, int y, int fontSize, Color tint) {
    draw(text.c_str(), x, y, fontSize, tint);
}

int measure(const char* text, int fontSize) {
    if (!text || fontSize <= 0) return 0;
    return static_cast<int>(std::ceil(MeasureTextEx(
        FontManager::font(), text, static_cast<float>(fontSize), 1.0f).x));
}

int measure(const std::string& text, int fontSize) {
    return measure(text.c_str(), fontSize);
}
}
