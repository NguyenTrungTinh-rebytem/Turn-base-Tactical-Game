#pragma once
#include "raylib.h"
#include <string>
#include <vector>
// RAII owner for Raylib texture-backed fonts. Call shutdown before CloseWindow.
class FontManager {
public:
    enum class Role { BODY, HUD, DISPLAY };
    static FontManager& instance();
    void initialize(const std::string& assetsDirectory);
    void setRenderScale(float windowScale);
    void shutdown();
    Font font(Role role, int logicalSize) const;
    bool isLoaded() const {return m_loaded;}
    float renderScale() const {return m_scale;}
private:
    struct Entry {Role m_role=Role::BODY; int m_logicalSize=0; Font m_font{};};
    void load();
    std::vector<Entry> m_entries;
    std::string m_assetsDirectory;
    bool m_loaded=false;
    float m_scale=1.f;
};
