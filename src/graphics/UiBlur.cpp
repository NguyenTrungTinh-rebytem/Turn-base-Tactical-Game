#include "UiBlur.h"
#include "UiCanvas.h"
#include "UiShapes.h"
#include "UiTheme.h"
#include "RaylibScope.h"
#include <iostream>

namespace {
RenderTexture2D snapshot{};
RenderTexture2D blurred{};
Shader blurShader{};
bool ready = false;
int texelSizeLocation = -1;
int blurRadiusLocation = -1;
int qualityLocation = -1;
UiBlurQuality blurQuality = UiBlurQuality::LOW;
}

bool UiBlur::initialize(const char* shaderPath) {
    shutdown();
    snapshot = LoadRenderTexture(UiTheme::CANVAS_WIDTH, UiTheme::CANVAS_HEIGHT);
    blurred = LoadRenderTexture(UiTheme::CANVAS_WIDTH, UiTheme::CANVAS_HEIGHT);
    if (snapshot.id == 0 || blurred.id == 0) {
        std::cerr << "UiBlur: unable to allocate blur render targets; using tint fallback\n";
        shutdown();
        return false;
    }

    SetTextureFilter(snapshot.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(blurred.texture, TEXTURE_FILTER_BILINEAR);
    blurShader = LoadShader(nullptr, shaderPath);
    if (blurShader.id == 0) {
        std::cerr << "UiBlur: blur shader unavailable; using tint fallback\n";
        UnloadRenderTexture(snapshot);
        UnloadRenderTexture(blurred);
        snapshot = RenderTexture2D{};
        blurred = RenderTexture2D{};
        return false;
    }
    texelSizeLocation = GetShaderLocation(blurShader, "texelSize");
    blurRadiusLocation = GetShaderLocation(blurShader, "blurRadius");
    qualityLocation = GetShaderLocation(blurShader, "blurQuality");
    ready = texelSizeLocation >= 0 && blurRadiusLocation >= 0 && qualityLocation >= 0;
    if (!ready) {
        std::cerr << "UiBlur: shader uniforms missing; using tint fallback\n";
        shutdown();
    }
    return ready;
}

void UiBlur::shutdown() {
    if (blurShader.id != 0 && IsWindowReady()) UnloadShader(blurShader);
    if (snapshot.id != 0 && IsWindowReady()) UnloadRenderTexture(snapshot);
    if (blurred.id != 0 && IsWindowReady()) UnloadRenderTexture(blurred);
    snapshot = RenderTexture2D{};
    blurred = RenderTexture2D{};
    blurShader = Shader{};
    ready = false;
}

void UiBlur::setQuality(UiBlurQuality quality) { blurQuality = quality; }
UiBlurQuality UiBlur::quality() { return blurQuality; }

void UiBlur::draw(Rectangle bounds, float radius, Color tint) {
    if (blurQuality == UiBlurQuality::OFF || !ready ||
        !UiCanvas::copyCurrentFrameTo(snapshot)) {
        UiShapes::roundedRect(bounds, radius, tint);
        return;
    }

    const float texelSize[] = {
        1.0f / static_cast<float>(UiTheme::CANVAS_WIDTH),
        1.0f / static_cast<float>(UiTheme::CANVAS_HEIGHT)
    };
    const float blurRadius = (blurQuality == UiBlurQuality::HIGH) ? 4.0f : 2.0f;
    const int qualityValue = (blurQuality == UiBlurQuality::HIGH) ? 1 : 0;
    SetShaderValue(blurShader, texelSizeLocation, texelSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(blurShader, blurRadiusLocation, &blurRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(blurShader, qualityLocation, &qualityValue, SHADER_UNIFORM_INT);

    bool blurredFrameReady = false;
    {
        UiCanvas::SuspensionGuard suspension;
        if (suspension) {
            RaylibScope::TextureMode target(blurred);
            if (target) {
                ClearBackground(BLACK);
                RaylibScope::ShaderMode shader(blurShader);
                DrawTexturePro(snapshot.texture,
                    Rectangle{ 0.0f, 0.0f, static_cast<float>(snapshot.texture.width),
                               -static_cast<float>(snapshot.texture.height) },
                    Rectangle{ 0.0f, 0.0f, static_cast<float>(blurred.texture.width),
                               static_cast<float>(blurred.texture.height) },
                    Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
                blurredFrameReady = true;
            }
        }
    }
    if (!blurredFrameReady) {
        UiShapes::roundedRect(bounds, radius, tint);
        return;
    }

    const float scale = UiCanvas::scale();
    const Rectangle source{
        bounds.x * scale,
        -(bounds.y + bounds.height) * scale,
        bounds.width * scale,
        -bounds.height * scale
    };
    DrawTexturePro(blurred.texture, source, bounds, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    UiShapes::roundedRect(bounds, radius, Color{ 0, 0, 0, 77 });
    UiShapes::roundedRect(bounds, radius, tint);
}
