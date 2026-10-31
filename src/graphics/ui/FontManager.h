#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Raylib fonts are textures. FontManager owns each texture until shutdown(),
// which MUST run before CloseWindow().
class FontManager {
public:
    enum class Role { BODY, DISPLAY };
    static FontManager& instance();
    void initialize(const std::string& assetsDirectory);
    void shutdown();
    Font font(Role role, int fontSize) const;
    bool isLoaded() const { return m_loaded; }
private:
    struct Entry {
        Role m_role = Role::BODY;
        int m_size = 0;
        Font m_font{};
    };
    std::vector<Entry> m_entries;
    bool m_loaded = false;
};
