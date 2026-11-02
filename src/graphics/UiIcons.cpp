#include "UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace {
std::unordered_map<std::string, Texture2D> textures;
std::unordered_set<std::string> loggedUnknown;

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

bool drawActionIcon(const std::string& key, Rectangle bounds, Color tint) {
    if (key.rfind("action:", 0) != 0) return false;
    const float size = std::min(bounds.width, bounds.height);
    const float stroke = std::max(1.0f, size * 0.1f);
    const float left = bounds.x + size * 0.2f;
    const float right = bounds.x + size * 0.8f;
    const float top = bounds.y + size * 0.2f;
    const float bottom = bounds.y + size * 0.8f;
    const Vector2 center{ bounds.x + bounds.width * 0.5f,
                          bounds.y + bounds.height * 0.5f };
    const auto line = [stroke, tint](Vector2 from, Vector2 to) {
        DrawLineEx(from, to, stroke, tint);
    };

    if (key == "action:attack") {
        line({ left, bottom }, { right, top });
        line({ left + size * 0.05f, bottom - size * 0.18f },
             { left + size * 0.22f, bottom - size * 0.01f });
        line({ right - size * 0.14f, top + size * 0.08f },
             { right, top + size * 0.24f });
    } else if (key == "action:skill") {
        line({ center.x, top }, { center.x, bottom });
        line({ left, center.y }, { right, center.y });
        line({ left + size * 0.08f, top + size * 0.08f },
             { right - size * 0.08f, bottom - size * 0.08f });
        line({ right - size * 0.08f, top + size * 0.08f },
             { left + size * 0.08f, bottom - size * 0.08f });
        DrawCircleV(center, size * 0.12f, tint);
    } else if (key == "action:item") {
        line({ center.x - size * 0.13f, top }, { center.x + size * 0.13f, top });
        line({ center.x - size * 0.13f, top }, { center.x - size * 0.17f, top + size * 0.16f });
        line({ center.x + size * 0.13f, top }, { center.x + size * 0.17f, top + size * 0.16f });
        line({ center.x - size * 0.17f, top + size * 0.16f },
             { center.x - size * 0.23f, bottom - size * 0.08f });
        line({ center.x + size * 0.17f, top + size * 0.16f },
             { center.x + size * 0.23f, bottom - size * 0.08f });
        line({ center.x - size * 0.23f, bottom - size * 0.08f },
             { center.x + size * 0.23f, bottom - size * 0.08f });
        line({ center.x - size * 0.18f, center.y + size * 0.08f },
             { center.x + size * 0.18f, center.y + size * 0.08f });
    } else if (key == "action:confirm") {
        line({ left, center.y }, { center.x - size * 0.06f, bottom - size * 0.08f });
        line({ center.x - size * 0.06f, bottom - size * 0.08f },
             { right, top + size * 0.05f });
    } else if (key == "action:close") {
        line({ left, top }, { right, bottom });
        line({ right, top }, { left, bottom });
    } else if (key == "action:pause") {
        DrawRectangleRec({ center.x - size * 0.22f, top, size * 0.14f, size * 0.6f }, tint);
        DrawRectangleRec({ center.x + size * 0.08f, top, size * 0.14f, size * 0.6f }, tint);
    } else if (key == "action:end_turn") {
        line({ left, center.y }, { right - size * 0.18f, center.y });
        DrawTriangle({ right - size * 0.32f, top + size * 0.03f },
                     { right, center.y }, { right - size * 0.32f, bottom - size * 0.03f }, tint);
    } else if (key == "action:undo") {
        line({ right, top + size * 0.1f }, { left + size * 0.12f, top + size * 0.1f });
        line({ left + size * 0.12f, top + size * 0.1f },
             { left + size * 0.12f, bottom - size * 0.1f });
        DrawTriangle({ left + size * 0.12f, top - size * 0.02f },
                     { left - size * 0.08f, top + size * 0.1f },
                     { left + size * 0.12f, top + size * 0.22f }, tint);
    } else if (key == "action:shop" || key == "action:coin") {
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.28f, tint);
        line({ center.x, top + size * 0.04f }, { center.x, bottom - size * 0.04f });
        line({ center.x + size * 0.13f, top + size * 0.18f },
             { center.x - size * 0.12f, top + size * 0.18f });
        line({ center.x - size * 0.12f, bottom - size * 0.18f },
             { center.x + size * 0.13f, bottom - size * 0.18f });
    } else if (key == "action:target") {
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.28f, tint);
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), size * 0.11f, tint);
        line({ center.x, top }, { center.x, top + size * 0.15f });
        line({ center.x, bottom - size * 0.15f }, { center.x, bottom });
        line({ left, center.y }, { left + size * 0.15f, center.y });
        line({ right - size * 0.15f, center.y }, { right, center.y });
    } else {
        return false;
    }
    return true;
}
}

bool UiIcons::initialize(const std::string& assetsDirectory) {
    shutdown();
    for (const std::string& key : iconKeys()) {
        const std::string path = assetPath(assetsDirectory, key);
        if (!FileExists(path.c_str())) continue;
        Texture2D texture = LoadTexture(path.c_str());
        if (texture.id == 0 || texture.width <= 0 || texture.height <= 0) continue;
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
    if (drawActionIcon(key, bounds, tint)) return;
    const auto found = textures.find(key);
    if (found != textures.end()) {
        if (found->second.id == 0 || found->second.width <= 0 ||
            found->second.height <= 0) {
            textures.erase(found);
        } else {
        DrawTexturePro(found->second,
            Rectangle{ 0.0f, 0.0f, static_cast<float>(found->second.width),
                       static_cast<float>(found->second.height) },
            bounds, Vector2{ 0.0f, 0.0f }, 0.0f, tint);
        return;
        }
    }

    const bool known = std::find(iconKeys().begin(), iconKeys().end(), key) != iconKeys().end() ||
                       key == "action:pause" || key == "action:coin";
    if (!known) {
        if (loggedUnknown.insert(key).second)
            std::cerr << "UiIcons: unknown icon id: " << key << '\n';
#if defined(UI_DEV_TOOLS) && UI_DEV_TOOLS
        DrawLineEx(Vector2{ bounds.x + 2.0f, bounds.y + 2.0f },
                   Vector2{ bounds.x + bounds.width - 2.0f,
                            bounds.y + bounds.height - 2.0f }, 2.0f, MAGENTA);
        DrawLineEx(Vector2{ bounds.x + bounds.width - 2.0f, bounds.y + 2.0f },
                   Vector2{ bounds.x + 2.0f, bounds.y + bounds.height - 2.0f },
                   2.0f, MAGENTA);
        return;
#endif
    }

    const float size = std::min(bounds.width, bounds.height);
    const Vector2 center{ bounds.x + bounds.width * 0.5f,
                          bounds.y + bounds.height * 0.5f };
    if (key.rfind("element:", 0) == 0) {
        UiShapes::seal(bounds, tint, true);
        if (key == "element:kim")
            DrawPoly(center, 4, size * 0.22f, 45.0f, tint);
        else if (key == "element:moc")
            DrawPoly(center, 3, size * 0.25f, -90.0f, tint);
        else if (key == "element:thuy")
            DrawPoly(center, 5, size * 0.24f, -90.0f, tint);
        else if (key == "element:hoa")
            DrawPoly(center, 6, size * 0.23f, 0.0f, tint);
        else
            DrawPoly(center, 4, size * 0.23f, 0.0f, tint);
    } else {
        UiShapes::seal(bounds, UiTheme::ash, false);
        if (known) {
            DrawLineEx(Vector2{ center.x - size * 0.16f, center.y },
                       Vector2{ center.x + size * 0.16f, center.y }, 1.5f, tint);
            DrawLineEx(Vector2{ center.x, center.y - size * 0.16f },
                       Vector2{ center.x, center.y + size * 0.16f }, 1.5f, tint);
        } else {
            DrawCircleV(center, size * 0.08f, tint);
        }
    }
}
