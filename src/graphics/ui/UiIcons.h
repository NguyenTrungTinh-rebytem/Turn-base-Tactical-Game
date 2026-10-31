#pragma once
#include "raylib.h"
#include "data/Element.h"
namespace UiIcons {
// A geometric sigil for each of the Five Elements. No external artwork.
void element(Element element, Vector2 center, float radius);
}
namespace UiIcons {
enum class Action { SWORD, RUNE, POTION, GEAR };
void action(Action kind, Vector2 center, float radius, Color tint);
}
