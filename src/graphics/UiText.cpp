#include "UiText.h"
#include "FontManager.h"
#include <algorithm>
#include <cmath>

namespace UiText {
void draw(const char* text, int x, int y, int fontSize, Color tint, float trackingEm) {
    if (!text || fontSize <= 0) return;
    const Font& font = FontManager::font();
    if (trackingEm <= 0.0f) {
        DrawTextEx(font, text, Vector2{ static_cast<float>(x), static_cast<float>(y) },
                   static_cast<float>(fontSize), 0.0f, tint);
        return;
    }

    float offset = 0.0f;
    const float scale = font.baseSize > 0
        ? static_cast<float>(fontSize) / static_cast<float>(font.baseSize) : 1.0f;
    const float tracking = trackingEm * static_cast<float>(fontSize);
    for (const char* cursor = text; *cursor;) {
        int bytes = 0;
        const int codepoint = GetCodepoint(cursor, &bytes);
        if (bytes <= 0) break;
        DrawTextCodepoint(font, codepoint,
            Vector2{ static_cast<float>(x) + offset, static_cast<float>(y) },
            static_cast<float>(fontSize), tint);
        const int glyph = GetGlyphIndex(font, codepoint);
        if (glyph >= 0 && glyph < font.glyphCount) {
            const GlyphInfo& info = font.glyphs[glyph];
            const float advance = info.advanceX > 0
                ? static_cast<float>(info.advanceX) : static_cast<float>(info.image.width);
            offset += advance * scale;
        }
        cursor += bytes;
        if (*cursor) offset += tracking;
    }
}

void draw(const std::string& text, int x, int y, int fontSize, Color tint, float trackingEm) {
    draw(text.c_str(), x, y, fontSize, tint, trackingEm);
}

int measure(const char* text, int fontSize, float trackingEm) {
    if (!text || fontSize <= 0) return 0;
    const Font& font = FontManager::font();
    const float width = MeasureTextEx(font, text, static_cast<float>(fontSize), 0.0f).x;
    int codepoints = 0;
    for (const char* cursor = text; *cursor;) {
        int bytes = 0;
        GetCodepoint(cursor, &bytes);
        if (bytes <= 0) break;
        cursor += bytes;
        ++codepoints;
    }
    return static_cast<int>(std::ceil(width +
        std::max(0, codepoints - 1) * trackingEm * static_cast<float>(fontSize)));
}

int measure(const std::string& text, int fontSize, float trackingEm) {
    return measure(text.c_str(), fontSize, trackingEm);
}
}
