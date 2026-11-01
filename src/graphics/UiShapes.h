#pragma once

#include "raylib.h"

namespace UiShapes {
bool initialize(const char* shaderPath);
void shutdown();
bool usingShader();
void roundedRect(Rectangle bounds, float radius, Color fill,
                 Color border = BLANK, float borderWidth = 0.0f);
}
