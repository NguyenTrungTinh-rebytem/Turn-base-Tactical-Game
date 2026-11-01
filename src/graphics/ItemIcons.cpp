// src/graphics/ItemIcons.cpp
#include "graphics/ItemIcons.h"
#include "graphics/UiText.h"
#include <algorithm>

ItemIcons::~ItemIcons() { unloadAll(); }

void ItemIcons::load(const std::string& itemId, const std::string& path, int category)
{
    Icon icon;
    icon.m_category = category;
    if (!path.empty() && FileExists(path.c_str())) {
        icon.m_texture = LoadTexture(path.c_str());
        icon.m_loaded = (icon.m_texture.id != 0);
        if (icon.m_loaded) SetTextureFilter(icon.m_texture, TEXTURE_FILTER_POINT);   // pixel art: giu net
    }
    if (!icon.m_loaded) TraceLog(LOG_WARNING, "ItemIcons: thieu anh '%s' (item %s)", path.c_str(), itemId.c_str());
    m_icons[itemId] = icon;
}

void ItemIcons::unloadAll()
{
    if (IsWindowReady())
        for (auto& kv : m_icons)
            if (kv.second.m_loaded) UnloadTexture(kv.second.m_texture);
    m_icons.clear();
}

bool ItemIcons::has(const std::string& itemId) const
{
    auto it = m_icons.find(itemId);
    return it != m_icons.end() && it->second.m_loaded;
}

void ItemIcons::draw(const std::string& itemId, Rectangle dest, Color tint) const
{
    auto it = m_icons.find(itemId);
    if (it != m_icons.end() && it->second.m_loaded) {
        const Texture2D& tex = it->second.m_texture;
        const float scale = std::min(dest.width / static_cast<float>(tex.width), dest.height / static_cast<float>(tex.height));
        const float w = static_cast<float>(tex.width) * scale, h = static_cast<float>(tex.height) * scale;
        DrawTexturePro(tex, Rectangle{ 0, 0, static_cast<float>(tex.width), static_cast<float>(tex.height) },
                       Rectangle{ dest.x + (dest.width - w) / 2.0f, dest.y + (dest.height - h) / 2.0f, w, h },
                       Vector2{ 0, 0 }, 0.0f, tint);
        return;
    }
    // o thay the: mau theo nhom + chu cai dau cua phan sau "ITM_"
    Color c = Color{ 200, 80, 90, 255 };
    if (it != m_icons.end()) {
        if (it->second.m_category == 1) c = Color{ 220, 170, 60, 255 };
        else if (it->second.m_category == 2) c = Color{ 90, 150, 230, 255 };
    }
    DrawRectangleRounded(dest, 0.25f, 4, c);
    DrawRectangleLinesEx(dest, 2.0f, Color{ 20, 20, 30, 255 });
    const size_t us = itemId.find('_');
    const char letter[2] = { us != std::string::npos && us + 1 < itemId.size() ? itemId[us + 1] : (itemId.empty() ? '?' : itemId[0]), 0 };
    const int fs = static_cast<int>(dest.height * 0.5f);
    UiText::draw(letter, static_cast<int>(dest.x + (dest.width - UiText::measure(letter, fs)) / 2.0f),
             static_cast<int>(dest.y + (dest.height - fs) / 2.0f), fs, WHITE);
}
