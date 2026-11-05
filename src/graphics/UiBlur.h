#pragma once

#include "raylib.h"

enum class UiBlurQuality {
    OFF,
    LOW,
    HIGH
};

namespace UiBlur {
bool initialize(const char* shaderPath);
void shutdown();
void setQuality(UiBlurQuality quality);
UiBlurQuality quality();
void draw(Rectangle bounds, float radius, Color tint);
}
