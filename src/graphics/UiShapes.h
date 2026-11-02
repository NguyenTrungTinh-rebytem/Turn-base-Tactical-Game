#pragma once

#include "raylib.h"

enum class UiShapePoint { NONE, LEFT, RIGHT };
enum class UiShapeFade { NONE, LEFT_TO_RIGHT, RIGHT_TO_LEFT };

struct UiShapeStyle {
    float cutTopLeft = 10.0f;
    float cutTopRight = 4.0f;
    float cutBottomRight = 10.0f;
    float cutBottomLeft = 4.0f;
    UiShapePoint point = UiShapePoint::NONE;
    float pointSize = 16.0f;
    float skew = 0.0f;
    UiShapeFade fade = UiShapeFade::NONE;
    float fadeStart = 0.65f;
    float fadeEnd = 1.0f;
    Color fill{ 14, 12, 20, 220 };
    Color fillEnd{ 0, 0, 0, 0 };
    Color topLine{ 141, 135, 148, 110 };
    Color bottomLine{ 141, 135, 148, 110 };
    Color glow{ 255, 59, 59, 0 };
    Color shadow{ 0, 0, 0, 0 };
    float lineWidth = 1.0f;
    float glowWidth = 0.0f;
    float shadowBlur = 0.0f;
    Vector2 shadowOffset{ 0.0f, 0.0f };
};

namespace UiShapes {
bool initialize(const char* shaderPath);
void shutdown();
bool usingShader();
void draw(Rectangle bounds, const UiShapeStyle& style);
void plate(Rectangle bounds, Color fill, Color edge = { 141, 135, 148, 110 },
           float lineWidth = 1.0f);
void bladeStrip(Rectangle bounds, Color fill, Color edge, bool rightAnchored,
                bool selected = false, bool hovered = false, bool enabled = true);
void seal(Rectangle bounds, Color ring, bool active = false);
void runeBar(Rectangle bounds, float ratio, Color fill, Color damageTrail = BLANK);
void angledTab(Rectangle bounds, Color fill, Color edge, bool selected = false,
               bool hovered = false);
void diamond(Vector2 center, float size, Color color, bool innerDot = true);
void divider(Vector2 start, Vector2 end, Color color);
void roundedRect(Rectangle bounds, float radius, Color fill,
                 Color border = BLANK, float borderWidth = 0.0f);
}
