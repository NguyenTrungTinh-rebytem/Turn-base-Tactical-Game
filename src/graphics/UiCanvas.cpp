#include "UiCanvas.h"
#include "UiTheme.h"
#include "UiCursor.h"
#include "RaylibScope.h"
#include <algorithm>
#include <iostream>
#include <optional>

namespace {
RenderTexture2D canvas{};
bool initialized = false;
constexpr float SCALE = static_cast<float>(UiTheme::CANVAS_WIDTH) /
                       static_cast<float>(UiTheme::DESIGN_WIDTH);
const Camera2D canvasCamera2D{ Vector2{ 0.0f, 0.0f }, Vector2{ 0.0f, 0.0f }, 0.0f, SCALE };
std::optional<RaylibScope::TextureMode> canvasTarget;
std::optional<RaylibScope::CameraMode> canvasCamera;
}

UiCanvas::FrameGuard::FrameGuard() {
    UiCanvas::begin();
    active = UiCanvas::isDrawing();
}

UiCanvas::FrameGuard::~FrameGuard() {
    if (active) UiCanvas::end();
}

UiCanvas::SuspensionGuard::SuspensionGuard() : active(UiCanvas::suspend()) {}

UiCanvas::SuspensionGuard::~SuspensionGuard() {
    if (active) UiCanvas::resume();
}

bool UiCanvas::initialize() {
    shutdown();
    canvas = LoadRenderTexture(UiTheme::CANVAS_WIDTH, UiTheme::CANVAS_HEIGHT);
    if (canvas.id == 0) return false;
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    initialized = true;
    return true;
}

void UiCanvas::shutdown() {
    canvasCamera.reset();
    canvasTarget.reset();
    if (initialized && IsWindowReady())
        UnloadRenderTexture(canvas);
    canvas = RenderTexture2D{};
    initialized = false;
}

void UiCanvas::begin() {
    if (!initialized || canvasTarget) return;
    canvasTarget.emplace(canvas);
    if (!*canvasTarget) {
        canvasTarget.reset();
        std::cerr << "UiCanvas: unable to begin canvas render target\n";
        return;
    }
    ClearBackground(BLACK);
    canvasCamera.emplace(canvasCamera2D);
}

void UiCanvas::end() {
    if (!initialized || !canvasTarget) return;
    canvasCamera.reset();
    canvasTarget.reset();
    DrawTexturePro(canvas.texture,
        Rectangle{ 0.0f, 0.0f, static_cast<float>(canvas.texture.width),
                   -static_cast<float>(canvas.texture.height) },
        Rectangle{ 0.0f, 0.0f, static_cast<float>(GetScreenWidth()),
                   static_cast<float>(GetScreenHeight()) },
        Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    UiCursor::draw();
}

bool UiCanvas::suspend() {
    if (!canvasTarget) return false;
    canvasCamera.reset();
    canvasTarget.reset();
    return true;
}

void UiCanvas::resume() {
    if (!initialized || canvasTarget) return;
    canvasTarget.emplace(canvas);
    if (!*canvasTarget) {
        canvasTarget.reset();
        std::cerr << "UiCanvas: unable to resume canvas render target\n";
        return;
    }
    canvasCamera.emplace(canvasCamera2D);
}

bool UiCanvas::isDrawing() {
    return initialized && canvasTarget.has_value() && canvasCamera.has_value();
}

int UiCanvas::width() { return UiTheme::DESIGN_WIDTH; }
int UiCanvas::height() { return UiTheme::DESIGN_HEIGHT; }
Camera2D UiCanvas::camera() { return canvasCamera2D; }
float UiCanvas::scale() { return SCALE; }

bool UiCanvas::copyCurrentFrameTo(RenderTexture2D destination) {
    if (!isDrawing() || destination.id == 0) return false;
    SuspensionGuard suspension;
    if (!suspension) return false;
    RaylibScope::TextureMode target(destination);
    if (!target) return false;
    ClearBackground(BLACK);
    DrawTexturePro(canvas.texture,
        Rectangle{ 0.0f, 0.0f, static_cast<float>(canvas.texture.width),
                   -static_cast<float>(canvas.texture.height) },
        Rectangle{ 0.0f, 0.0f, static_cast<float>(destination.texture.width),
                   static_cast<float>(destination.texture.height) },
        Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    return true;
}

Vector2 UiCanvas::mousePosition() {
    const Vector2 position = GetMousePosition();
    return { position.x * static_cast<float>(width()) / static_cast<float>(GetScreenWidth()),
             position.y * static_cast<float>(height()) / static_cast<float>(GetScreenHeight()) };
}

Vector2 UiCanvas::mouseDelta() {
    const Vector2 delta = GetMouseDelta();
    return { delta.x * static_cast<float>(width()) / static_cast<float>(GetScreenWidth()),
             delta.y * static_cast<float>(height()) / static_cast<float>(GetScreenHeight()) };
}

void UiCanvas::drawCover(Texture2D texture, Rectangle destination) {
    if (texture.id == 0 || texture.width <= 0 || texture.height <= 0) return;
    const float sourceAspect = static_cast<float>(texture.width) / static_cast<float>(texture.height);
    const float destinationAspect = destination.width / destination.height;
    Rectangle source{ 0.0f, 0.0f, static_cast<float>(texture.width),
                      static_cast<float>(texture.height) };
    if (sourceAspect > destinationAspect) {
        source.width = static_cast<float>(texture.height) * destinationAspect;
        source.x = (static_cast<float>(texture.width) - source.width) * 0.5f;
    } else {
        source.height = static_cast<float>(texture.width) / destinationAspect;
        source.y = (static_cast<float>(texture.height) - source.height) * 0.5f;
    }
    DrawTexturePro(texture, source, destination, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
}
