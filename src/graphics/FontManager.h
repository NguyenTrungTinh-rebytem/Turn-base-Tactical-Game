#pragma once

#include "raylib.h"
#include <array>
#include <map>
#include <string>
#include <vector>

enum class UiFontRole {
    DISPLAY = 0,
    HUD,
    BODY,
    BODY_SEMIBOLD,
    NUMERALS,
    AUTO
};

class FontManager {
public:
    static FontManager& instance();

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    ~FontManager();

    bool initialize(const std::string& fontDirectory);
    void shutdown();
    const Font& font(UiFontRole role, int logicalSize);
    bool hasGlyph(UiFontRole role, int codepoint, int logicalSize = 48);

private:
    FontManager() = default;

    struct FontFace {
        std::string m_path;
        std::map<int, Font> m_cache;
    };

    static constexpr std::size_t ROLE_COUNT = 5;

    bool loadFace(UiFontRole role, const std::string& path, int pixelSize);
    void clearCache();
    int pixelSizeFor(int logicalSize) const;
    static std::vector<int> requiredCodepoints();

    std::array<FontFace, ROLE_COUNT> m_faces;
    std::vector<int> m_codepoints;
    int m_lastWidth = 0;
    int m_lastHeight = 0;
    bool m_initialized = false;
};
