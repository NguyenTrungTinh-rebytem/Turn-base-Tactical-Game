#include "UiShapes.h"
#include "UiCanvas.h"
#include "RaylibScope.h"
#include "UiTheme.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
Shader shapeShader{};
bool shaderReady = false;
int sizeLocation = -1;
int chamferLocation = -1;
int pointSideLocation = -1;
int pointSizeLocation = -1;
int skewLocation = -1;
int fadeDirectionLocation = -1;
int fadeStartLocation = -1;
int fadeEndLocation = -1;
int lineWidthLocation = -1;
int glowWidthLocation = -1;
int shadowBlurLocation = -1;
int shadowOffsetLocation = -1;
int fillLocation = -1;
int fillEndLocation = -1;
int topLineLocation = -1;
int bottomLineLocation = -1;
int glowLocation = -1;
int shadowLocation = -1;

Vector4 normalized(Color color) {
    constexpr float scale = 1.0f / 255.0f;
    return { color.r * scale, color.g * scale, color.b * scale, color.a * scale };
}

int pointSide(UiShapePoint point) {
    switch (point) {
        case UiShapePoint::LEFT: return 1;
        case UiShapePoint::RIGHT: return 2;
        case UiShapePoint::NONE: return 0;
    }
    return 0;
}

int fadeDirection(UiShapeFade fade) {
    switch (fade) {
        case UiShapeFade::LEFT_TO_RIGHT: return 1;
        case UiShapeFade::RIGHT_TO_LEFT: return 2;
        case UiShapeFade::NONE: return 0;
    }
    return 0;
}

std::vector<Vector2> vertices(Rectangle bounds, const UiShapeStyle& style) {
    const float left = bounds.x;
    const float right = bounds.x + bounds.width;
    const float top = bounds.y;
    const float bottom = bounds.y + bounds.height;
    const float tl = std::clamp(style.cutTopLeft, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    const float tr = std::clamp(style.cutTopRight, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    const float br = std::clamp(style.cutBottomRight, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    const float bl = std::clamp(style.cutBottomLeft, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    std::vector<Vector2> path{
        { left, top + tl }, { left + tl, top }, { right - tr, top },
        { right, top + tr }, { right, bottom - br }, { right - br, bottom },
        { left + bl, bottom }, { left, bottom - bl }
    };
    if (style.point == UiShapePoint::LEFT)
        path.insert(path.begin(), Vector2{ left - style.pointSize, (top + bottom) * 0.5f });
    else if (style.point == UiShapePoint::RIGHT)
        path.insert(path.begin() + 4,
                    Vector2{ right + style.pointSize, (top + bottom) * 0.5f });
    for (Vector2& vertex : path)
        vertex.x += style.skew * (vertex.y - (top + bottom) * 0.5f);
    return path;
}

float fadeAt(float x, const UiShapeStyle& style) {
    if (style.fade == UiShapeFade::LEFT_TO_RIGHT)
        return 1.0f - std::clamp((x - style.fadeStart) /
            std::max(0.001f, style.fadeEnd - style.fadeStart), 0.0f, 1.0f);
    if (style.fade == UiShapeFade::RIGHT_TO_LEFT)
        return std::clamp((x - style.fadeStart) /
            std::max(0.001f, style.fadeEnd - style.fadeStart), 0.0f, 1.0f);
    return 1.0f;
}

Color withAlpha(Color color, float alpha) {
    color.a = static_cast<unsigned char>(static_cast<float>(color.a) *
                                         std::clamp(alpha, 0.0f, 1.0f));
    return color;
}

Color fillAt(Color start, Color end, float amount) {
    if (end.a == 0) return withAlpha(start, amount);
    const float t = std::clamp(amount, 0.0f, 1.0f);
    return Color{
        static_cast<unsigned char>(start.r + (static_cast<float>(end.r) - start.r) * t),
        static_cast<unsigned char>(start.g + (static_cast<float>(end.g) - start.g) * t),
        static_cast<unsigned char>(start.b + (static_cast<float>(end.b) - start.b) * t),
        static_cast<unsigned char>(start.a + (static_cast<float>(end.a) - start.a) * t)
    };
}

void drawFallback(Rectangle bounds, const UiShapeStyle& style) {
    const std::vector<Vector2> path = vertices(bounds, style);
    if (style.shadow.a > 0) {
        std::vector<Vector2> shadowPath = path;
        for (Vector2& vertex : shadowPath) {
            vertex.x += style.shadowOffset.x;
            vertex.y += style.shadowOffset.y;
        }
        rlSetTexture(rlGetTextureIdDefault());
        rlBegin(RL_TRIANGLES);
        rlColor4ub(style.shadow.r, style.shadow.g, style.shadow.b, style.shadow.a);
        const Vector2 shadowCenter{ bounds.x + bounds.width * 0.5f + style.shadowOffset.x,
                                    bounds.y + bounds.height * 0.5f + style.shadowOffset.y };
        for (std::size_t i = 0; i < shadowPath.size(); ++i) {
            const Vector2 a = shadowPath[i];
            const Vector2 b = shadowPath[(i + 1) % shadowPath.size()];
            rlVertex2f(shadowCenter.x, shadowCenter.y);
            rlVertex2f(a.x, a.y);
            rlVertex2f(b.x, b.y);
        }
        rlEnd();
    }

    const Vector2 center{ bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
    rlSetTexture(rlGetTextureIdDefault());
    rlBegin(RL_TRIANGLES);
    for (std::size_t i = 0; i < path.size(); ++i) {
        const Vector2 a = path[i];
        const Vector2 b = path[(i + 1) % path.size()];
        const Color centerColor = withAlpha(fillAt(style.fill, style.fillEnd, 0.5f),
                                             fadeAt(0.5f, style));
        rlColor4ub(centerColor.r, centerColor.g, centerColor.b, centerColor.a);
        rlVertex2f(center.x, center.y);
        const float ax = (a.x - bounds.x) / bounds.width;
        const Color aColor = withAlpha(fillAt(style.fill, style.fillEnd, ax), fadeAt(ax, style));
        rlColor4ub(aColor.r, aColor.g, aColor.b, aColor.a);
        rlVertex2f(a.x, a.y);
        const float bx = (b.x - bounds.x) / bounds.width;
        const Color bColor = withAlpha(fillAt(style.fill, style.fillEnd, bx), fadeAt(bx, style));
        rlColor4ub(bColor.r, bColor.g, bColor.b, bColor.a);
        rlVertex2f(b.x, b.y);
    }
    rlEnd();

    if (style.lineWidth <= 0.0f) return;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const Vector2 a = path[i];
        const Vector2 b = path[(i + 1) % path.size()];
        Color edge = (a.y <= center.y && b.y <= center.y) ? style.topLine : style.bottomLine;
        const float alpha = std::min(fadeAt((a.x - bounds.x) / bounds.width, style),
                                     fadeAt((b.x - bounds.x) / bounds.width, style));
        DrawLineEx(a, b, style.lineWidth / UiCanvas::scale(), withAlpha(edge, alpha));
    }
}

void uniformColor(int location, Color value) {
    if (location < 0) return;
    const Vector4 color = normalized(value);
    SetShaderValue(shapeShader, location, &color, SHADER_UNIFORM_VEC4);
}
}

bool UiShapes::initialize(const char* shaderPath) {
    shutdown();
    shapeShader = LoadShader(nullptr, shaderPath);
    if (shapeShader.id == 0) {
        std::cerr << "UiShapes: parametric shader unavailable; using polygon fallback\n";
        return false;
    }

    sizeLocation = GetShaderLocation(shapeShader, "shapeSize");
    chamferLocation = GetShaderLocation(shapeShader, "chamferSizes");
    pointSideLocation = GetShaderLocation(shapeShader, "pointSide");
    pointSizeLocation = GetShaderLocation(shapeShader, "pointSize");
    skewLocation = GetShaderLocation(shapeShader, "shapeSkew");
    fadeDirectionLocation = GetShaderLocation(shapeShader, "fadeDirection");
    fadeStartLocation = GetShaderLocation(shapeShader, "fadeStart");
    fadeEndLocation = GetShaderLocation(shapeShader, "fadeEnd");
    lineWidthLocation = GetShaderLocation(shapeShader, "lineWidth");
    glowWidthLocation = GetShaderLocation(shapeShader, "glowWidth");
    shadowBlurLocation = GetShaderLocation(shapeShader, "shadowBlur");
    shadowOffsetLocation = GetShaderLocation(shapeShader, "shadowOffset");
    fillLocation = GetShaderLocation(shapeShader, "fillColor");
    fillEndLocation = GetShaderLocation(shapeShader, "fillEndColor");
    topLineLocation = GetShaderLocation(shapeShader, "topLineColor");
    bottomLineLocation = GetShaderLocation(shapeShader, "bottomLineColor");
    glowLocation = GetShaderLocation(shapeShader, "glowColor");
    shadowLocation = GetShaderLocation(shapeShader, "shadowColor");

    shaderReady = sizeLocation >= 0 && chamferLocation >= 0 &&
        pointSideLocation >= 0 && pointSizeLocation >= 0 && skewLocation >= 0 &&
        fadeDirectionLocation >= 0 && fadeStartLocation >= 0 && fadeEndLocation >= 0 &&
        lineWidthLocation >= 0 && glowWidthLocation >= 0 && shadowBlurLocation >= 0 &&
        shadowOffsetLocation >= 0 && fillLocation >= 0 && fillEndLocation >= 0 && topLineLocation >= 0 &&
        bottomLineLocation >= 0 && glowLocation >= 0 && shadowLocation >= 0;
    if (!shaderReady) {
        std::cerr << "UiShapes: required parametric shader uniforms missing; using polygon fallback\n";
        UnloadShader(shapeShader);
        shapeShader = Shader{};
    }
    return shaderReady;
}

void UiShapes::shutdown() {
    if (shapeShader.id != 0 && IsWindowReady()) UnloadShader(shapeShader);
    shapeShader = Shader{};
    shaderReady = false;
}

bool UiShapes::usingShader() { return shaderReady; }

void UiShapes::draw(Rectangle bounds, const UiShapeStyle& style) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;
    if (!shaderReady) {
        drawFallback(bounds, style);
        return;
    }

    const float scale = UiCanvas::scale();
    const float physicalSize[] = { bounds.width * scale, bounds.height * scale };
    const float chamfers[] = { style.cutTopLeft * scale, style.cutTopRight * scale,
                               style.cutBottomRight * scale, style.cutBottomLeft * scale };
    const int point = pointSide(style.point);
    const int fade = fadeDirection(style.fade);
    const float pointExtent = style.pointSize * scale;
    const float skew = style.skew;
    const float fadeStart = style.fadeStart;
    const float fadeEnd = style.fadeEnd;
    const float lineWidth = style.lineWidth;
    const float glowWidth = style.glowWidth;
    const float shadowBlur = style.shadowBlur;
    const float shadowOffset[] = { style.shadowOffset.x * scale, style.shadowOffset.y * scale };
    SetShaderValue(shapeShader, sizeLocation, physicalSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(shapeShader, chamferLocation, chamfers, SHADER_UNIFORM_VEC4);
    SetShaderValue(shapeShader, pointSideLocation, &point, SHADER_UNIFORM_INT);
    SetShaderValue(shapeShader, pointSizeLocation, &pointExtent, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, skewLocation, &skew, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, fadeDirectionLocation, &fade, SHADER_UNIFORM_INT);
    SetShaderValue(shapeShader, fadeStartLocation, &fadeStart, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, fadeEndLocation, &fadeEnd, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, lineWidthLocation, &lineWidth, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, glowWidthLocation, &glowWidth, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, shadowBlurLocation, &shadowBlur, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shapeShader, shadowOffsetLocation, shadowOffset, SHADER_UNIFORM_VEC2);
    uniformColor(fillLocation, style.fill);
    uniformColor(fillEndLocation, style.fillEnd);
    uniformColor(topLineLocation, style.topLine);
    uniformColor(bottomLineLocation, style.bottomLine);
    uniformColor(glowLocation, style.glow);
    uniformColor(shadowLocation, style.shadow);
    RaylibScope::ShaderMode shader(shapeShader);
    DrawRectangleRec(bounds, WHITE);
}

void UiShapes::plate(Rectangle bounds, Color fill, Color edge, float lineWidth) {
    const float cut = std::min(UiTheme::plateChamferMax, bounds.height * 0.24f);
    UiShapeStyle style;
    style.cutTopLeft = cut;
    style.cutTopRight = std::min(5.0f, bounds.height * 0.12f);
    style.cutBottomRight = cut;
    style.cutBottomLeft = std::min(5.0f, bounds.height * 0.12f);
    style.fill = fill;
    style.topLine = edge;
    style.bottomLine = edge;
    style.lineWidth = lineWidth;
    draw(bounds, style);
    const Rectangle inner{ bounds.x + 6.0f, bounds.y + 6.0f,
                           bounds.width - 12.0f, bounds.height - 12.0f };
    if (inner.width > 20.0f && inner.height > 16.0f && edge.a > 0) {
        DrawLineEx(Vector2{ inner.x + 10.0f, inner.y },
                   Vector2{ inner.x + inner.width - 12.0f, inner.y },
                   0.6f, withAlpha(edge, 0.35f));
        DrawLineEx(Vector2{ inner.x, inner.y + 10.0f },
                   Vector2{ inner.x, inner.y + inner.height - 8.0f },
                   0.6f, withAlpha(edge, 0.35f));
        const float tick = 4.0f;
        DrawLineEx(Vector2{ bounds.x + 5.0f, bounds.y + cut + 4.0f },
                   Vector2{ bounds.x + 5.0f, bounds.y + cut + tick + 4.0f },
                   1.0f, withAlpha(edge, 0.65f));
    }
}

void UiShapes::bladeStrip(Rectangle bounds, Color fill, Color edge, bool rightAnchored,
                          bool selected, bool hovered, bool enabled) {
    UiShapeStyle style;
    const float cut = std::min(12.0f, bounds.height * 0.28f);
    style.cutTopLeft = rightAnchored ? 4.0f : cut;
    style.cutBottomLeft = rightAnchored ? 4.0f : cut;
    style.cutTopRight = rightAnchored ? cut : 4.0f;
    style.cutBottomRight = rightAnchored ? cut : 4.0f;
    style.point = rightAnchored ? UiShapePoint::RIGHT : UiShapePoint::LEFT;
    style.pointSize = std::min(18.0f, bounds.height * UiTheme::bladePointRatio);
    style.fade = rightAnchored ? UiShapeFade::RIGHT_TO_LEFT : UiShapeFade::LEFT_TO_RIGHT;
    if (rightAnchored) {
        style.fadeStart = 0.0f;
        style.fadeEnd = UiTheme::bladeFadeFraction;
    } else {
        style.fadeStart = 1.0f - UiTheme::bladeFadeFraction;
        style.fadeEnd = 1.0f;
    }
    style.fill = fill;
    style.fill.a = static_cast<unsigned char>(std::min<unsigned>(style.fill.a, 224));
    style.topLine = selected ? UiTheme::crimson : hovered ? UiTheme::silver : edge;
    style.bottomLine = selected ? UiTheme::crimson : hovered ? UiTheme::silver : edge;
    style.lineWidth = 1.0f;
    if (selected) {
        style.glow = UiTheme::crimsonGlow;
        style.glow.a = 0.38f;
        style.glowWidth = 3.0f;
    }
    if (!enabled) {
        style.fill.a = static_cast<unsigned char>(style.fill.a * 0.4f);
        style.topLine.a = static_cast<unsigned char>(style.topLine.a * 0.4f);
        style.bottomLine.a = static_cast<unsigned char>(style.bottomLine.a * 0.4f);
        style.glow.a = 0;
    }
    draw(bounds, style);
    if (selected) {
        const float x = rightAnchored ? bounds.x + bounds.width - 23.0f : bounds.x + 23.0f;
        diamond(Vector2{ x, bounds.y + bounds.height * 0.5f }, 7.0f, UiTheme::crimsonGlow);
    }
}

void UiShapes::seal(Rectangle bounds, Color ring, bool active) {
    const float cut = std::min(bounds.width, bounds.height) * 0.28f;
    UiShapeStyle outer;
    outer.cutTopLeft = cut;
    outer.cutTopRight = cut;
    outer.cutBottomRight = cut;
    outer.cutBottomLeft = cut;
    outer.fill = UiTheme::ink;
    outer.topLine = ring;
    outer.bottomLine = ring;
    outer.lineWidth = 1.2f;
    if (active) {
        outer.glow = UiTheme::crimsonGlow;
        outer.glow.a = 0.35f;
        outer.glowWidth = 4.0f;
    }
    draw(bounds, outer);
    const Rectangle inner{ bounds.x + 3.0f, bounds.y + 3.0f,
                           bounds.width - 6.0f, bounds.height - 6.0f };
    outer.fill = BLANK;
    outer.topLine = withAlpha(ring, 0.55f);
    outer.bottomLine = outer.topLine;
    outer.lineWidth = 0.7f;
    draw(inner, outer);
    const Vector2 center{ bounds.x + bounds.width * 0.5f,
                          bounds.y + bounds.height * 0.5f };
    for (int tick = 0; tick < 4; ++tick) {
        const float angle = static_cast<float>(tick) * PI * 0.5f;
        const float innerRadius = bounds.width * 0.40f;
        const float outerRadius = bounds.width * (active ? 0.57f : 0.52f);
        DrawLineEx(Vector2{ center.x + std::cos(angle) * innerRadius,
                            center.y + std::sin(angle) * innerRadius },
                   Vector2{ center.x + std::cos(angle) * outerRadius,
                            center.y + std::sin(angle) * outerRadius },
                   1.0f, ring);
    }
}

void UiShapes::runeBar(Rectangle bounds, float ratio, Color fill, Color damageTrail) {
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    UiShapeStyle base;
    base.cutTopLeft = bounds.height * 0.45f;
    base.cutTopRight = bounds.height * 0.45f;
    base.cutBottomRight = bounds.height * 0.45f;
    base.cutBottomLeft = bounds.height * 0.45f;
    base.skew = UiTheme::runeBarSkew;
    base.fill = Color{ 14, 12, 20, 230 };
    base.topLine = UiTheme::ash;
    base.bottomLine = UiTheme::ash;
    base.lineWidth = 0.8f;
    draw(bounds, base);
    Rectangle active = bounds;
    active.x += 1.0f;
    active.y += 1.0f;
    active.width = std::max(0.0f, (active.width - 2.0f) * ratio);
    active.height = std::max(0.0f, active.height - 2.0f);
    if (damageTrail.a > 0 && ratio < 1.0f) {
        Rectangle trail = bounds;
        trail.width *= std::min(1.0f, ratio + 0.12f);
        trail.height = std::max(0.0f, trail.height - 1.0f);
        UiShapeStyle trailStyle = base;
        trailStyle.fill = damageTrail;
        trailStyle.topLine = BLANK;
        trailStyle.bottomLine = BLANK;
        trailStyle.lineWidth = 0.0f;
        draw(trail, trailStyle);
    }
    if (active.width > 0.0f) {
        UiShapeStyle activeStyle = base;
        activeStyle.fill = fill;
        activeStyle.fillEnd = Color{ fill.r, fill.g, fill.b,
                                     static_cast<unsigned char>(fill.a * 0.68f) };
        activeStyle.topLine = BLANK;
        activeStyle.bottomLine = BLANK;
        activeStyle.lineWidth = 0.0f;
        draw(active, activeStyle);
    }
    const float segmentWidth = bounds.width / 10.0f;
    for (int index = 1; index < 10; ++index) {
        const float x = bounds.x + segmentWidth * index;
        DrawLineEx(Vector2{ x, bounds.y + 1.0f },
                   Vector2{ x + bounds.height * 0.12f, bounds.y + bounds.height - 1.0f },
                   0.6f, Color{ 7, 6, 10, 125 });
    }
    if (ratio > 0.0f)
        diamond(Vector2{ bounds.x + bounds.width * ratio, bounds.y + bounds.height * 0.5f },
                std::max(3.0f, bounds.height * 0.75f), UiTheme::silver, false);
}

void UiShapes::angledTab(Rectangle bounds, Color fill, Color edge, bool selected, bool hovered) {
    UiShapeStyle style;
    style.cutTopLeft = 0.0f;
    style.cutTopRight = 0.0f;
    style.cutBottomRight = 0.0f;
    style.cutBottomLeft = 0.0f;
    style.skew = 0.16f;
    style.fill = fill;
    style.topLine = hovered ? UiTheme::silver : edge;
    style.bottomLine = selected ? UiTheme::crimson : edge;
    style.lineWidth = selected ? 1.5f : 0.8f;
    if (selected) {
        style.glow = UiTheme::crimsonGlow;
        style.glow.a = 0.35f;
        style.glowWidth = 2.0f;
    }
    draw(bounds, style);
    if (selected) diamond(Vector2{ bounds.x + 9.0f, bounds.y + bounds.height - 2.0f },
                          5.0f, UiTheme::crimsonGlow);
}

void UiShapes::diamond(Vector2 center, float size, Color color, bool innerDot) {
    const float half = size * 0.5f;
    DrawLineEx(Vector2{ center.x, center.y - half }, Vector2{ center.x + half, center.y },
               1.0f, color);
    DrawLineEx(Vector2{ center.x + half, center.y }, Vector2{ center.x, center.y + half },
               1.0f, color);
    DrawLineEx(Vector2{ center.x, center.y + half }, Vector2{ center.x - half, center.y },
               1.0f, color);
    DrawLineEx(Vector2{ center.x - half, center.y }, Vector2{ center.x, center.y - half },
               1.0f, color);
    if (innerDot) DrawCircleV(center, std::max(1.0f, size * 0.12f), color);
}

void UiShapes::divider(Vector2 start, Vector2 end, Color color) {
    const Vector2 middle{ (start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f };
    DrawLineEx(start, middle, 1.0f, withAlpha(color, 0.55f));
    DrawLineEx(middle, end, 1.0f, withAlpha(color, 0.55f));
    diamond(middle, 6.0f, color);
}

void UiShapes::roundedRect(Rectangle bounds, float radius, Color fill,
                           Color border, float borderWidth) {
    const float cut = std::min(radius, std::min(bounds.width, bounds.height) * 0.28f);
    UiShapeStyle style;
    style.cutTopLeft = cut;
    style.cutTopRight = cut * 0.35f;
    style.cutBottomRight = cut;
    style.cutBottomLeft = cut * 0.35f;
    style.fill = fill;
    style.topLine = border;
    style.bottomLine = border;
    style.lineWidth = borderWidth;
    draw(bounds, style);
}
