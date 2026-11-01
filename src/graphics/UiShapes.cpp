#include "UiShapes.h"
#include "UiCanvas.h"
#include "RaylibScope.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
Shader shapeShader{};
bool shaderReady = false;
int sizeLocation = -1;
int quadSizeLocation = -1;
int radiusLocation = -1;
int flatBottomLocation = -1;
int borderWidthLocation = -1;
int fillLocation = -1;
int borderLocation = -1;
int gradientLocation = -1;
int shadowLocation = -1;
int shadowBlurLocation = -1;
int shadowOffsetLocation = -1;
int glowLocation = -1;
int glowRadiusLocation = -1;
int glowStrengthLocation = -1;
int textureSampleLocation = -1;
int textureUvLocation = -1;

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
    quadSizeLocation = GetShaderLocation(shapeShader, "quadSize");
    radiusLocation = GetShaderLocation(shapeShader, "cornerRadius");
    flatBottomLocation = GetShaderLocation(shapeShader, "flatBottom");
    borderWidthLocation = GetShaderLocation(shapeShader, "borderWidth");
    fillLocation = GetShaderLocation(shapeShader, "fillColor");
    borderLocation = GetShaderLocation(shapeShader, "borderColor");
    gradientLocation = GetShaderLocation(shapeShader, "gradientBottom");
    shadowLocation = GetShaderLocation(shapeShader, "shadowColor");
    shadowBlurLocation = GetShaderLocation(shapeShader, "shadowBlur");
    shadowOffsetLocation = GetShaderLocation(shapeShader, "shadowOffsetY");
    glowLocation = GetShaderLocation(shapeShader, "glowColor");
    glowRadiusLocation = GetShaderLocation(shapeShader, "glowRadius");
    glowStrengthLocation = GetShaderLocation(shapeShader, "glowStrength");
    textureSampleLocation = GetShaderLocation(shapeShader, "sampleTexture");
    textureUvLocation = GetShaderLocation(shapeShader, "textureUvRect");
    shaderReady = sizeLocation >= 0 && quadSizeLocation >= 0 &&
                  radiusLocation >= 0 && flatBottomLocation >= 0 &&
                  borderWidthLocation >= 0 && fillLocation >= 0 && borderLocation >= 0 &&
                  gradientLocation >= 0 && shadowLocation >= 0 &&
                  shadowBlurLocation >= 0 && shadowOffsetLocation >= 0 &&
                  glowLocation >= 0 && glowRadiusLocation >= 0 &&
                  glowStrengthLocation >= 0 && textureSampleLocation >= 0 &&
                  textureUvLocation >= 0;
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

namespace {
void drawShape(Rectangle bounds, float radius, Color fill, Color border,
               float borderWidth, const UiShapeEffects& effects,
               Texture2D texture, Rectangle source) {
    const float maxRadius = 0.5f * std::min(bounds.width, bounds.height);
    radius = std::clamp(radius, 0.0f, maxRadius);
    borderWidth = std::clamp(borderWidth, 0.0f, maxRadius);

    if (shaderReady && (texture.id == 0 || (source.width != 0.0f && source.height != 0.0f))) {
        const float scale = UiCanvas::scale();
        const float physicalSize[] = { bounds.width * scale, bounds.height * scale };
        const float physicalRadius = radius * scale;
        const float physicalBorder = borderWidth * scale;
        const float extent = texture.id != 0 ? 0.0f :
            std::max(effects.shadowBlur + std::abs(effects.shadowOffsetY),
                     effects.glowRadius) + 2.0f;
        const float physicalPadding = extent * scale;
        const float quadSize[] = {
            physicalSize[0] + 2.0f * physicalPadding,
            physicalSize[1] + 2.0f * physicalPadding
        };
        const int flatBottom = effects.flatBottom ? 1 : 0;
        const Vector4 fillValue = normalized(fill);
        const Vector4 borderValue = normalized(border);
        const Vector4 gradientValue = normalized(
            effects.gradientBottom.a > 0 ? effects.gradientBottom : fill);
        const Vector4 shadowValue = normalized(Color{ 0, 0, 0,
            static_cast<unsigned char>(effects.shadowBlur > 0.0f ? 92 : 0) });
        const Vector4 glowValue = normalized(effects.glow);
        const float physicalShadowBlur = effects.shadowBlur * scale;
        const float physicalShadowOffset = effects.shadowOffsetY * scale;
        const float physicalGlowRadius = effects.glowRadius * scale;
        const float glowStrength = std::clamp(effects.glowStrength, 0.0f, 1.0f);
        const int useTexture = texture.id != 0 ? 1 : 0;
        const float uv[] = {
            texture.id != 0 ? source.x / static_cast<float>(texture.width) : 0.0f,
            texture.id != 0 ? source.y / static_cast<float>(texture.height) : 0.0f,
            texture.id != 0 ? source.width / static_cast<float>(texture.width) : 1.0f,
            texture.id != 0 ? source.height / static_cast<float>(texture.height) : 1.0f
        };
        SetShaderValue(shapeShader, sizeLocation, physicalSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shapeShader, quadSizeLocation, quadSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(shapeShader, radiusLocation, &physicalRadius, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, flatBottomLocation, &flatBottom, SHADER_UNIFORM_INT);
        SetShaderValue(shapeShader, borderWidthLocation, &physicalBorder, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, fillLocation, &fillValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, borderLocation, &borderValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, gradientLocation, &gradientValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, shadowLocation, &shadowValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, shadowBlurLocation, &physicalShadowBlur, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, shadowOffsetLocation, &physicalShadowOffset, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, glowLocation, &glowValue, SHADER_UNIFORM_VEC4);
        SetShaderValue(shapeShader, glowRadiusLocation, &physicalGlowRadius, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, glowStrengthLocation, &glowStrength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shapeShader, textureSampleLocation, &useTexture, SHADER_UNIFORM_INT);
        SetShaderValue(shapeShader, textureUvLocation, uv, SHADER_UNIFORM_VEC4);
        RaylibScope::ShaderMode shader(shapeShader);
        const Rectangle expanded{ bounds.x - extent, bounds.y - extent,
                                  bounds.width + 2.0f * extent,
                                  bounds.height + 2.0f * extent };
        if (texture.id != 0) {
            DrawTexturePro(texture, source, bounds, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
        } else {
            DrawRectangleRec(expanded, WHITE);
        }
        return;
    }

    const float roundness = (maxRadius > 0.0f) ? radius / maxRadius : 0.0f;
    if (texture.id != 0) {
        DrawTexturePro(texture, source, bounds, Vector2{ 0.0f, 0.0f }, 0.0f, fill);
        if (borderWidth > 0.0f && border.a > 0)
            DrawRectangleRoundedLinesEx(bounds, roundness, 16, borderWidth, border);
        return;
    }
    if (effects.shadowBlur > 0.0f) {
        DrawRectangleRounded(Rectangle{ bounds.x, bounds.y + effects.shadowOffsetY,
                                        bounds.width, bounds.height },
                             roundness, 16, Color{ 0, 0, 0, 92 });
    }
    DrawRectangleRounded(bounds, roundness, 16, fill);
    if (borderWidth > 0.0f && border.a > 0) {
        DrawRectangleRoundedLinesEx(bounds, roundness, 16, borderWidth, border);
    }
}
}

void UiShapes::roundedRect(Rectangle bounds, float radius, Color fill,
                           Color border, float borderWidth,
                           const UiShapeEffects& effects) {
    drawShape(bounds, radius, fill, border, borderWidth, effects, Texture2D{}, Rectangle{});
}

void UiShapes::roundedTexture(Rectangle bounds, Rectangle source, Texture2D texture,
                              float radius, Color tint, Color border, float borderWidth,
                              const UiShapeEffects& effects) {
    if (texture.id == 0) {
        roundedRect(bounds, radius, tint, border, borderWidth, effects);
        return;
    }
    drawShape(bounds, radius, tint, border, borderWidth, effects, texture, source);
}
