#pragma once
#include "raylib.h"
#include <string>
namespace UiWidgets {
void button(Rectangle area, const std::string& text, bool hovered, bool selected = false, bool enabled = true, bool danger = false);
void commandButton(Rectangle area, const std::string& label, int actionKind, bool hovered, bool enabled = true);
void tooltip(const std::string& title, const std::string& description, Vector2 mouse);
void tooltip(const std::string& title, const std::string& description, Vector2 mouse, Rectangle trigger);
void listRow(Rectangle r, const std::string& title, const std::string& cost, const std::string& iconId, bool hover, bool selected, bool enabled, bool skill);
void skillIcon(Rectangle bounds, const std::string& id);
void itemIcon(Rectangle bounds, const std::string& id);
}
