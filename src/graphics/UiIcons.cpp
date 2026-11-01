#include "UiIcons.h"
#include "graphics/UiText.h"
#include <iostream>
#include <unordered_map>
#include <vector>

namespace {
std::unordered_map<std::string, Texture2D> textures;

const std::vector<std::string>& iconKeys() {
    static const std::vector<std::string> keys = {
        "item:ITM_HP_MINOR", "item:ITM_HP_MAJOR", "item:ITM_ENERGY", "item:ITM_PHOENIX",
        "item:ITM_POWER_SERUM", "item:ITM_IRON_SHIELD", "item:ITM_HASTE", "item:ITM_PRECISION",
        "item:ITM_LEFTOVER", "item:ITM_VAMPIRIC", "item:ITM_SASH", "item:ITM_GUARDIAN",
        "item:ITM_BERSERK", "item:ITM_EXECUTIONER", "item:ITM_HEAVY_STONE", "item:ITM_COUNTER",
        "skill:R_OGRE_1", "skill:R_OGRE_2", "skill:R_HUMAN_1", "skill:R_HUMAN_2",
        "skill:R_DWARF_1", "skill:R_DWARF_2", "skill:R_ELF_1", "skill:R_ELF_2",
        "skill:R_ONI_1", "skill:R_ONI_2", "skill:R_BEAST_1", "skill:R_BEAST_2",
        "skill:C_MAGE_1", "skill:C_MAGE_2", "skill:C_SWORD_1", "skill:C_SWORD_2",
        "skill:C_ASSASSIN_1", "skill:C_ASSASSIN_2", "skill:C_TANKER_1", "skill:C_TANKER_2",
        "skill:C_ENCHANT_1", "skill:C_ENCHANT_2", "skill:C_PALADIN_1", "skill:C_PALADIN_2",
        "action:attack", "action:skill", "action:item", "action:confirm", "action:undo",
        "action:end_turn", "action:close", "action:shop", "action:target",
        "element:kim", "element:moc", "element:thuy", "element:hoa", "element:tho"
    };
    return keys;
}

std::string assetPath(const std::string& root, const std::string& key) {
    const std::size_t separator = key.find(':');
    return root + "/" + key.substr(0, separator) + "/" +
           key.substr(separator + 1) + ".png";
}
}

bool UiIcons::initialize(const std::string& assetsDirectory) {
    shutdown();
    for (const std::string& key : iconKeys()) {
        const std::string path = assetPath(assetsDirectory, key);
        if (!FileExists(path.c_str())) {
            std::cerr << "UiIcons: missing generated icon: " << path << '\n';
            continue;
        }
        Texture2D texture = LoadTexture(path.c_str());
        if (texture.id == 0) {
            std::cerr << "UiIcons: failed to load icon: " << path << '\n';
            continue;
        }
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
        textures.emplace(key, texture);
    }
    return !textures.empty();
}

void UiIcons::shutdown() {
    if (IsWindowReady())
        for (const auto& entry : textures) UnloadTexture(entry.second);
    textures.clear();
}

bool UiIcons::contains(const std::string& key) {
    return textures.find(key) != textures.end();
}

void UiIcons::draw(const std::string& key, Rectangle bounds, Color tint) {
    const auto found = textures.find(key);
    if (found != textures.end()) {
        DrawTexturePro(found->second,
            Rectangle{ 0.0f, 0.0f, static_cast<float>(found->second.width),
                       static_cast<float>(found->second.height) },
            bounds, Vector2{ 0.0f, 0.0f }, 0.0f, tint);
        return;
    }

    DrawRectangleRec(bounds, Color{ 20, 20, 35, 255 });
    DrawRectangleLinesEx(bounds, 1.0f, Color{ 230, 230, 240, 255 });
    const std::size_t colon = key.find(':');
    const char fallback[] = { colon == std::string::npos || colon + 1 >= key.size()
        ? '?' : key[colon + 1], '\0' };
    const int size = static_cast<int>(bounds.height * 0.55f);
    UiText::draw(fallback, static_cast<int>(bounds.x + (bounds.width - UiText::measure(fallback, size)) / 2.0f),
                 static_cast<int>(bounds.y + (bounds.height - size) / 2.0f), size, WHITE);
}
