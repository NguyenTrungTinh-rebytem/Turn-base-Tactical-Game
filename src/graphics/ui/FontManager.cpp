#include "FontManager.h"
#include <algorithm>
#include <cstdlib>

FontManager& FontManager::instance() {
    static FontManager manager;
    return manager;
}

void FontManager::initialize(const std::string& assetsDirectory) {
    shutdown();
    std::vector<int> glyphs;
    auto range = [&glyphs](int first, int last) {
        for (int ch = first; ch <= last; ++ch) glyphs.push_back(ch);
    };
    range(32, 126);                 // ASCII
    range(0x00a0, 0x017f);         // Latin-1 + Latin Extended-A
    range(0x0300, 0x036f);         // Combining diacritics
    range(0x1e00, 0x1eff);         // Full Latin Extended Additional (Vietnamese)
    for (int c : {0x2013, 0x2014, 0x2018, 0x2019, 0x201c, 0x201d,
                  0x2022, 0x2026, 0x203a, 0x00d7})
        glyphs.push_back(c);
    const std::string folder = assetsDirectory + "/fonts/";
    const int bodySizes[] = {10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};
    const int displaySizes[] = {32, 34, 36, 40, 48, 50, 56, 72, 76};

    // Prefer the project's themed Noto fonts. If they have not been installed,
    // Windows fonts still render Vietnamese correctly; avoid Raylib's ASCII-only
    // fallback. These are references to OS fonts, not redistributed font files.
    const std::string bodyPaths[] = {
        folder + "NotoSans-Medium.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    const std::string displayPaths[] = {
        folder + "NotoSerifDisplay-SemiBold.ttf",
        "C:/Windows/Fonts/georgiab.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };

    for (Role role : {Role::BODY, Role::DISPLAY}) {
        const bool body = role == Role::BODY;
        const int* sizes = body ? bodySizes : displaySizes;
        const size_t count = body
            ? sizeof(bodySizes) / sizeof(bodySizes[0])
            : sizeof(displaySizes) / sizeof(displaySizes[0]);
        const std::string* paths = body ? bodyPaths : displayPaths;
        const size_t pathCount = body
            ? sizeof(bodyPaths) / sizeof(bodyPaths[0])
            : sizeof(displayPaths) / sizeof(displayPaths[0]);
        bool loaded = false;

        for (size_t p = 0; p < pathCount && !loaded; ++p) {
            const std::string& path = paths[p];
            if (!FileExists(path.c_str())) continue;
            const size_t previousCount = m_entries.size();
            for (size_t i = 0; i < count; ++i) {
                Font font = LoadFontEx(path.c_str(), sizes[i], glyphs.data(), static_cast<int>(glyphs.size()));
                if (font.texture.id == 0 || font.glyphCount <= 0) continue;
                SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
                m_entries.push_back({role, sizes[i], font});
            }
            loaded = m_entries.size() > previousCount;
            if (loaded) {
                TraceLog(p == 0 ? LOG_INFO : LOG_WARNING,
                         "UI FONT: %s loaded from %s",
                         body ? "BODY" : "DISPLAY", path.c_str());
            }
        }
        if (!loaded) {
            TraceLog(LOG_WARNING,
                     "UI FONT: missing font for %s; install Noto fonts into %s",
                     body ? "BODY" : "DISPLAY", folder.c_str());
        }
    }
    m_loaded = !m_entries.empty();
}

Font FontManager::font(Role role, int size) const {
    const Entry* match = nullptr;
    for (const Entry& entry : m_entries) {
        if (entry.m_role != role) continue;
        if (!match || std::abs(entry.m_size - size) < std::abs(match->m_size - size)) match = &entry;
    }
    if (!match && !m_entries.empty()) match = &m_entries.front();
    return match ? match->m_font : GetFontDefault();
}

void FontManager::shutdown() {
    if (IsWindowReady()) for (const Entry& entry : m_entries) UnloadFont(entry.m_font);
    m_entries.clear();
    m_loaded = false;
}
