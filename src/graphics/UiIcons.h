#pragma once

#include "raylib.h"
#include "data/DataTypes.h"
#include <string>

namespace UiIcons {
bool initialize(const std::string& assetsDirectory);
void shutdown();
bool contains(const std::string& key);
void draw(const std::string& key, Rectangle bounds, Color tint = WHITE);
void drawSkill(Element element, SkillType type, bool isMagic, Rectangle bounds,
               Color tint = WHITE);
}
