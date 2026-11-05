#pragma once

#include "raylib.h"
#include <string>

class FontManager {
public:
    static bool initialize(const std::string& fontPath);
    static void shutdown();
    static const Font& font();
    static bool hasGlyph(int codepoint);

private:
    static Font s_font;
    static bool s_loaded;
};
