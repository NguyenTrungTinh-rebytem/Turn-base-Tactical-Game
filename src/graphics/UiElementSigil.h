#pragma once

#include "raylib.h"
#include "data/Element.h"

namespace UiElementSigil {
Color color(Element element);
const char* iconKey(Element element);
void draw(Element element, Rectangle bounds, Color tint = WHITE);
}
