#include "FontManager.h"
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

Font FontManager::s_font{};
bool FontManager::s_loaded = false;

bool FontManager::initialize(const std::string& fontPath) {
    shutdown();

    std::vector<int> codepoints;
    codepoints.reserve(900);
    for (int codepoint = 32; codepoint <= 126; ++codepoint)
        codepoints.push_back(codepoint);
    for (const auto range : { std::pair<int, int>{ 0x00A0, 0x024F },
                              std::pair<int, int>{ 0x0300, 0x036F },
                              std::pair<int, int>{ 0x1E00, 0x1EFF },
                              std::pair<int, int>{ 0x2010, 0x206F } }) {
        for (int codepoint = range.first; codepoint <= range.second; ++codepoint)
            codepoints.push_back(codepoint);
    }

    s_font = LoadFontEx(fontPath.c_str(), 96, codepoints.data(),
                        static_cast<int>(codepoints.size()));
    if (s_font.texture.id == 0) {
        std::cerr << "FontManager: failed to load font: " << fontPath << '\n';
        return false;
    }
    SetTextureFilter(s_font.texture, TEXTURE_FILTER_BILINEAR);
    s_loaded = true;

    constexpr int requiredGlyphs[] = {
        0x0102, 0x0103, 0x00C2, 0x00E2, 0x00CA, 0x00EA,
        0x00D4, 0x00F4, 0x01A0, 0x01A1, 0x01AF, 0x01B0,
        0x0110, 0x0111, 0x1EA4, 0x1EA5, 0x1EDB, 0x1EDC,
        0x1EAC, 0x1EE2, 0x2026
    };
    for (const int codepoint : requiredGlyphs) {
        assert(hasGlyph(codepoint) && "Bundled font is missing required Vietnamese glyphs");
    }
    return true;
}

void FontManager::shutdown() {
    if (s_loaded && IsWindowReady())
        UnloadFont(s_font);
    s_font = Font{};
    s_loaded = false;
}

const Font& FontManager::font() {
    if (s_loaded) return s_font;
    static const Font fallback = GetFontDefault();
    return fallback;
}

bool FontManager::hasGlyph(int codepoint) {
    if (!s_loaded) return false;
    const int index = GetGlyphIndex(s_font, codepoint);
    return index >= 0 && s_font.glyphs[index].value == codepoint;
}
