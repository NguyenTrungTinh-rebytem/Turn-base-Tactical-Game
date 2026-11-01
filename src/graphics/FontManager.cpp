#include "FontManager.h"
#include "graphics/UiCanvas.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
std::size_t roleIndex(UiFontRole role) {
    const int index = static_cast<int>(role);
    return index >= 0 && index < 5 ? static_cast<std::size_t>(index) : 0U;
}

const char* const FONT_FILES[] = {
    "CormorantGaramond.ttf",
    "BarlowSemiCondensed-SemiBold.ttf",
    "BeVietnamPro-Regular.ttf",
    "BeVietnamPro-SemiBold.ttf",
    "BeVietnamPro-ExtraBold.ttf"
};

bool criticalUiGlyph(int codepoint) {
    if ((codepoint >= 0x0020 && codepoint <= 0x007E) ||
        (codepoint >= 0x00C0 && codepoint <= 0x00D6) ||
        (codepoint >= 0x00D8 && codepoint <= 0x00F6) ||
        (codepoint >= 0x00F8 && codepoint <= 0x00FF) ||
        (codepoint >= 0x1EA0 && codepoint <= 0x1EF9))
        return true;
    switch (codepoint) {
        case 0x0102: case 0x0103: case 0x0110: case 0x0111:
        case 0x01A0: case 0x01A1: case 0x01AF: case 0x01B0:
        case 0x2013: case 0x2014: case 0x2026: case 0x201C: case 0x201D:
        case 0x2018: case 0x2019: case 0x2022: case 0x00D7: case 0x00B7:
            return true;
        default:
            return false;
    }
}
}

FontManager& FontManager::instance() {
    static FontManager manager;
    return manager;
}

FontManager::~FontManager() {
    shutdown();
}

std::vector<int> FontManager::requiredCodepoints() {
    std::vector<int> codepoints;
    codepoints.reserve(740);
    codepoints.push_back('?');

    const auto appendRange = [&codepoints](int first, int last) {
        for (int codepoint = first; codepoint <= last; ++codepoint)
            if (codepoint != '?') codepoints.push_back(codepoint);
    };

    appendRange(0x0020, 0x007E);
    appendRange(0x00A0, 0x024F);
    appendRange(0x0300, 0x036F);
    appendRange(0x1EA0, 0x1EF9);
    for (const int codepoint : { 0x2013, 0x2014, 0x2026, 0x201C, 0x201D,
                                 0x2018, 0x2019, 0x2022, 0x00D7, 0x00B7,
                                 0x25B2, 0x25BC, 0x25B6, 0x25C0 }) {
        if (std::find(codepoints.begin(), codepoints.end(), codepoint) == codepoints.end())
            codepoints.push_back(codepoint);
    }
    return codepoints;
}

bool FontManager::initialize(const std::string& fontDirectory) {
    shutdown();
    m_codepoints = requiredCodepoints();
    m_lastWidth = GetScreenWidth();
    m_lastHeight = GetScreenHeight();

    for (int index = 0; index < static_cast<int>(ROLE_COUNT); ++index) {
        const UiFontRole role = static_cast<UiFontRole>(index);
        const std::string path = fontDirectory + "/" + FONT_FILES[index];
        if (!loadFace(role, path, pixelSizeFor(48))) {
            std::cerr << "FontManager: unable to load required font face: " << path << '\n';
            shutdown();
            return false;
        }
    }
    m_initialized = true;
    return true;
}

void FontManager::shutdown() {
    clearCache();
    m_codepoints.clear();
    m_lastWidth = 0;
    m_lastHeight = 0;
    m_initialized = false;
}

void FontManager::clearCache() {
    for (FontFace& face : m_faces) {
        for (auto& entry : face.m_cache)
            if (entry.second.texture.id != 0 && IsWindowReady())
                UnloadFont(entry.second);
        face.m_cache.clear();
    }
}

int FontManager::pixelSizeFor(int logicalSize) const {
    const float scale = static_cast<float>(GetScreenWidth()) /
                        static_cast<float>(std::max(1, UiCanvas::width()));
    return std::max(1, static_cast<int>(std::lround(std::max(1, logicalSize) * scale)));
}

bool FontManager::loadFace(UiFontRole role, const std::string& path, int pixelSize) {
    FontFace& face = m_faces[roleIndex(role)];
    const auto existing = face.m_cache.find(pixelSize);
    if (existing != face.m_cache.end()) return existing->second.texture.id != 0;

    Font loaded = LoadFontEx(path.c_str(), pixelSize, m_codepoints.data(),
                             static_cast<int>(m_codepoints.size()));
    if (loaded.texture.id == 0) return false;

    SetTextureFilter(loaded.texture, TEXTURE_FILTER_BILINEAR);
    int missingCriticalCount = 0;
    const int fallbackIndex = GetGlyphIndex(loaded, 0x10FFFF);
    for (const int codepoint : m_codepoints) {
        const int glyphIndex = GetGlyphIndex(loaded, codepoint);
        const bool present = glyphIndex >= 0 && glyphIndex < loaded.glyphCount &&
                             loaded.glyphs[glyphIndex].value == codepoint;
        if (!present) {
            std::cerr << "FontManager: missing U+" << std::hex << codepoint << std::dec
                      << " in " << path << " (fallback glyph index " << fallbackIndex << ")\n";
            if (criticalUiGlyph(codepoint)) ++missingCriticalCount;
        }
    }
    assert(missingCriticalCount == 0 && "Bundled fonts must contain all critical UI glyphs");
    if (missingCriticalCount != 0) {
        UnloadFont(loaded);
        return false;
    }

    face.m_path = path;
    face.m_cache.emplace(pixelSize, loaded);
    return true;
}

const Font& FontManager::font(UiFontRole role, int logicalSize) {
    static const Font fallback = GetFontDefault();
    if (!m_initialized && m_codepoints.empty()) return fallback;

    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    if (m_lastWidth > 0 && (width != m_lastWidth || height != m_lastHeight)) {
        clearCache();
        m_lastWidth = width;
        m_lastHeight = height;
    }

    const std::size_t index = roleIndex(role);
    FontFace& face = m_faces[index];
    const int pixelSize = pixelSizeFor(logicalSize);
    auto found = face.m_cache.find(pixelSize);
    if (found == face.m_cache.end()) {
        if (face.m_path.empty() || !loadFace(role, face.m_path, pixelSize)) {
            std::cerr << "FontManager: unable to load font at " << pixelSize
                      << " px: " << face.m_path << '\n';
            if (face.m_cache.empty()) return fallback;
            return face.m_cache.begin()->second;
        }
        found = face.m_cache.find(pixelSize);
    }
    return found->second;
}

bool FontManager::hasGlyph(UiFontRole role, int codepoint, int logicalSize) {
    const Font& loaded = font(role, logicalSize);
    const int index = GetGlyphIndex(loaded, codepoint);
    return index >= 0 && index < loaded.glyphCount &&
           loaded.glyphs[index].value == codepoint;
}
