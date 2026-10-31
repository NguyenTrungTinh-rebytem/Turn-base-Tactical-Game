#pragma once
#include "raylib.h"
#include <string>
namespace UiWidgets {
void button(Rectangle area, const std::string& text, bool hovered, bool selected = false, bool enabled = true, bool danger = false);
void tooltip(const std::string& title, const std::string& description, Vector2 mouse);
void itemIcon(Rectangle bounds, const std::string& id);
}
