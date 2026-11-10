// src/graphics/HeroPortraits.cpp
#include "graphics/HeroPortraits.h"
#include <algorithm>
#include <cctype>

namespace {
std::string lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}

bool HeroPortraits::load(const std::string& heroId, const std::string& path)
{
    const std::string key = lower(heroId);
    if (m_textures.count(key)) return true;
    if (!FileExists(path.c_str())) return false;
    Texture2D tex = LoadTexture(path.c_str());
    if (tex.id == 0) return false;
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    m_textures[key] = tex;
    return true;
}

void HeroPortraits::unloadAll()
{
    if (IsWindowReady())
        for (auto& kv : m_textures) UnloadTexture(kv.second);
    m_textures.clear();
}

bool HeroPortraits::draw(const std::string& heroId, Rectangle dest, Color tint) const
{
    const auto it = m_textures.find(lower(heroId));
    if (it == m_textures.end()) return false;
    const Texture2D& tex = it->second;

    // Cat giua theo ti le khung (phu kin, khong meo hinh); uu tien phan tren (khuon mat)
    const float destRatio = dest.width / dest.height;
    const float texRatio = static_cast<float>(tex.width) / static_cast<float>(tex.height);
    Rectangle src{ 0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height) };
    if (texRatio > destRatio) {                       // anh rong hon khung -> cat hai ben
        src.width = tex.height * destRatio;
        src.x = (tex.width - src.width) * 0.5f;
    } else {                                          // anh cao hon khung -> cat day
        src.height = tex.width / destRatio;
        src.y = 0.0f;
    }
    DrawTexturePro(tex, src, dest, Vector2{ 0.0f, 0.0f }, 0.0f, tint);

    // Toi dan o day de chu / chan khong bi lan vao tranh
    const int h = static_cast<int>(dest.height * 0.30f);
    DrawRectangleGradientV(static_cast<int>(dest.x), static_cast<int>(dest.y + dest.height) - h,
                           static_cast<int>(dest.width), h, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 170 });
    DrawRectangleLinesEx(dest, 2.0f, Color{ 200, 170, 90, 200 });
    return true;
}
