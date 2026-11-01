#include "UiBlur.h"
#include "UiCanvas.h"
#include "UiShapes.h"
#include "UiTheme.h"
#include "RaylibScope.h"
#include <algorithm>
#include <iostream>

namespace {
RenderTexture2D snapshot{};
RenderTexture2D horizontal{};
RenderTexture2D blurred{};
Shader blurShader{};
bool ready = false;
int texelSizeLocation = -1;
int directionLocation = -1;
int blurRadiusLocation = -1;
UiBlurQuality blurQuality = UiBlurQuality::LOW;

bool blurPass(Texture2D source, RenderTexture2D destination,
              const float direction[2], float radius) {
    UiCanvas::SuspensionGuard suspension;
    if (!suspension) return false;
    RaylibScope::TextureMode target(destination);
    if (!target) return false;
    ClearBackground(BLACK);
    SetShaderValue(blurShader, directionLocation, direction, SHADER_UNIFORM_VEC2);
    SetShaderValue(blurShader, blurRadiusLocation, &radius, SHADER_UNIFORM_FLOAT);
    RaylibScope::ShaderMode shader(blurShader);
    DrawTexturePro(source,
        Rectangle{ 0.0f, 0.0f, static_cast<float>(source.width),
                   -static_cast<float>(source.height) },
        Rectangle{ 0.0f, 0.0f, static_cast<float>(destination.texture.width),
                   static_cast<float>(destination.texture.height) },
        Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    return true;
}
}

bool UiBlur::initialize(const char* shaderPath) {
    shutdown();
    constexpr int downsample = 4;
    const int width = UiTheme::CANVAS_WIDTH / downsample;
    const int height = UiTheme::CANVAS_HEIGHT / downsample;
    snapshot = LoadRenderTexture(width, height);
    horizontal = LoadRenderTexture(width, height);
    blurred = LoadRenderTexture(width, height);
    if (snapshot.id == 0 || horizontal.id == 0 || blurred.id == 0) {
        std::cerr << "UiBlur: unable to allocate quarter-resolution blur targets; using tint fallback\n";
        shutdown();
        return false;
    }

    SetTextureFilter(snapshot.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(horizontal.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(blurred.texture, TEXTURE_FILTER_BILINEAR);
    blurShader = LoadShader(nullptr, shaderPath);
    if (blurShader.id == 0) {
        std::cerr << "UiBlur: blur shader unavailable; using tint fallback\n";
        shutdown();
        return false;
    }
    texelSizeLocation = GetShaderLocation(blurShader, "texelSize");
    directionLocation = GetShaderLocation(blurShader, "direction");
    blurRadiusLocation = GetShaderLocation(blurShader, "blurRadius");
    ready = texelSizeLocation >= 0 && directionLocation >= 0 && blurRadiusLocation >= 0;
    if (!ready) {
        std::cerr << "UiBlur: shader uniforms missing; using tint fallback\n";
        shutdown();
        return false;
    }
    const float texelSize[] = {
        1.0f / static_cast<float>(width),
        1.0f / static_cast<float>(height)
    };
    SetShaderValue(blurShader, texelSizeLocation, texelSize, SHADER_UNIFORM_VEC2);
    return true;
}

void UiBlur::shutdown() {
    if (blurShader.id != 0 && IsWindowReady()) UnloadShader(blurShader);
    if (snapshot.id != 0 && IsWindowReady()) UnloadRenderTexture(snapshot);
    if (horizontal.id != 0 && IsWindowReady()) UnloadRenderTexture(horizontal);
    if (blurred.id != 0 && IsWindowReady()) UnloadRenderTexture(blurred);
    snapshot = RenderTexture2D{};
    horizontal = RenderTexture2D{};
    blurred = RenderTexture2D{};
    blurShader = Shader{};
    ready = false;
}

void UiBlur::setQuality(UiBlurQuality quality) { blurQuality = quality; }
UiBlurQuality UiBlur::quality() { return blurQuality; }

void UiBlur::draw(Rectangle bounds, float radius, Color tint) {
    UiShapeEffects shadow;
    shadow.shadowBlur = UiTheme::floatingShadowSize;
    shadow.shadowOffsetY = 3.0f;
    UiShapes::roundedRect(bounds, radius, BLANK, BLANK, 0.0f, shadow);

    if (blurQuality == UiBlurQuality::OFF || !ready ||
        !UiCanvas::copyCurrentFrameTo(snapshot)) {
        UiShapeEffects effects;
        effects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(bounds, radius, tint, UiTheme::border, 1.0f, effects);
        return;
    }

    const float radiusScale = blurQuality == UiBlurQuality::HIGH ? 1.35f : 1.0f;
    constexpr float horizontalDirection[] = { 1.0f, 0.0f };
    constexpr float verticalDirection[] = { 0.0f, 1.0f };
    if (!blurPass(snapshot.texture, horizontal, horizontalDirection, radiusScale) ||
        !blurPass(horizontal.texture, blurred, verticalDirection, radiusScale)) {
        UiShapeEffects effects;
        effects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(bounds, radius, tint, UiTheme::border, 1.0f, effects);
        return;
    }

    const float sourceScale = UiCanvas::scale() *
        static_cast<float>(blurred.texture.width) / static_cast<float>(UiTheme::CANVAS_WIDTH);
    const Rectangle source{
        bounds.x * sourceScale,
        -(bounds.y + bounds.height) * sourceScale,
        bounds.width * sourceScale,
        -bounds.height * sourceScale
    };
    UiShapes::roundedTexture(bounds, source, blurred.texture, radius,
                             tint, UiTheme::border, 1.0f);
}
