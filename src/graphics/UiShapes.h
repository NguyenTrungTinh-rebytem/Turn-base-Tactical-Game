#pragma once

#include "raylib.h"

struct UiShapeEffects {
    Color gradientBottom = BLANK;
    Color glow = BLANK;
    float shadowBlur = 0.0f;
    float shadowOffsetY = 3.0f;
    float glowRadius = 0.0f;
    float glowStrength = 0.0f;
    bool flatBottom = false;
};

namespace UiShapes {
bool initialize(const char* shaderPath);
void shutdown();
bool usingShader();
void roundedRect(Rectangle bounds, float radius, Color fill,
                 Color border = BLANK, float borderWidth = 0.0f,
                 const UiShapeEffects& effects = {});
void roundedTexture(Rectangle bounds, Rectangle source, Texture2D texture, float radius,
                    Color tint, Color border = BLANK, float borderWidth = 0.0f,
                    const UiShapeEffects& effects = {});
}
