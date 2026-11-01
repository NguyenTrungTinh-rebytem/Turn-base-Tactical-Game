#include "UiShapes.h"
#include "UiCanvas.h"
#include "RaylibScope.h"
#include <algorithm>
#include <iostream>

namespace {
Shader shapeShader{};
bool shaderReady = false;
int sizeLocation = -1;
int radiusLocation = -1;
int borderWidthLocation = -1;
int fillLocation = -1;
int borderLocation = -1;

Vector4 normalized(Color color) {
    constexpr float scale = 1.0f / 255.0f;
    return { color.r * scale, color.g * scale, color.b * scale, color.a * scale };
}
}

bool UiShapes::initialize(const char* shaderPath) {
    shutdown();
    shapeShader = LoadShader(nullptr, shaderPath);
    if (shapeShader.id == 0) {
        std::cerr << "UiShapes: SDF shader unavailable; using rounded-rectangle fallback\n";
        return false;
    }

    sizeLocation = GetShaderLocation(shapeShader, "shapeSize");
    radiusLocation = GetShaderLocation(shapeShader, "cornerRadius");
    borderWidthLocation = GetShaderLocation(shapeShader, "borderWidth");
    fillLocation = GetShaderLocation(shapeShader, "fillColor");
    borderLocation = GetShaderLocation(shapeShader, "borderColor");
    shaderReady = sizeLocation >= 0 && radiusLocation >= 0 &&
                  borderWidthLocation >= 0 && fillLocation >= 0 && borderLocation >= 0;
    if (!shaderReady) {
        std::cerr << "UiShapes: shader uniforms missing; using rounded-rectangle fallback\n";
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

void UiShapes::roundedRect(Rectangle bounds, float radius, Color fill,
                           Color border, float borderWidth) {
    const float maxRadius = 0.5f * std::min(bounds.width, bounds.height);
    radius = std::clamp(radius, 0.0f, maxRadius);
    borderWidth = std::clamp(borderWidth, 0.0f, maxRadius);

    if (shaderReady) {
        const float scale = UiCanvas::scale();
        const float physicalSize[] = { bounds.width * scale, bounds.height * scale };
        const float physicalRadius = radius * scale;
        const float physicalBorder = borderWidth * scale;
        const Vector4 fillValue = normalized(fill);
        const Vector4 borderValue = normalized(border);
        SetShaderValue(shapeShader, sizeLocation, physicalSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shapeShader, radiusLocation, &physicalRadius, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, borderWidthLocation, &physicalBorder, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, fillLocation, &fillValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, borderLocation, &borderValue, SHADER_UNIFORM_VEC4);
        RaylibScope::ShaderMode shader(shapeShader);
        DrawRectangleRec(bounds, WHITE);
        return;
    }

    const float roundness = (maxRadius > 0.0f) ? radius / maxRadius : 0.0f;
    DrawRectangleRounded(bounds, roundness, 12, fill);
    if (borderWidth > 0.0f && border.a > 0) {
        DrawRectangleRoundedLinesEx(bounds, roundness, 12, borderWidth, border);
    }
}
