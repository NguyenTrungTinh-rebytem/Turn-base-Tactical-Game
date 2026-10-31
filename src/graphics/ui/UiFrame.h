#pragma once
#include "raylib.h"
namespace UiFrame {
void panel(Rectangle bounds, bool highlighted = false);
void plaque(Rectangle bounds, bool active = false, bool danger = false, float hover = 0.0f);
void gauge(Rectangle bounds, float ratio, Color tint);
}
