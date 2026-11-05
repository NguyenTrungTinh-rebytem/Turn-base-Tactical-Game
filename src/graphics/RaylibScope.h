#pragma once

#include "raylib.h"

namespace RaylibScope {
class Drawing {
public:
    Drawing() : active(IsWindowReady()) {
        if (active) BeginDrawing();
    }
    ~Drawing() { if (active) EndDrawing(); }
    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;
    explicit operator bool() const { return active; }
private:
    bool active;
};

class TextureMode {
public:
    explicit TextureMode(RenderTexture2D target) : active(target.id != 0) {
        if (active) BeginTextureMode(target);
    }
    ~TextureMode() { if (active) EndTextureMode(); }
    TextureMode(const TextureMode&) = delete;
    TextureMode& operator=(const TextureMode&) = delete;
    explicit operator bool() const { return active; }
private:
    bool active;
};

class ShaderMode {
public:
    explicit ShaderMode(Shader shader) : active(shader.id != 0) {
        if (active) BeginShaderMode(shader);
    }
    ~ShaderMode() { if (active) EndShaderMode(); }
    ShaderMode(const ShaderMode&) = delete;
    ShaderMode& operator=(const ShaderMode&) = delete;
    explicit operator bool() const { return active; }
private:
    bool active;
};

class ScissorMode {
public:
    ScissorMode(int x, int y, int width, int height) : active(IsWindowReady() && width > 0 && height > 0) {
        if (active) BeginScissorMode(x, y, width, height);
    }
    ~ScissorMode() { if (active) EndScissorMode(); }
    ScissorMode(const ScissorMode&) = delete;
    ScissorMode& operator=(const ScissorMode&) = delete;
private:
    bool active;
};

class Blend {
public:
    explicit Blend(::BlendMode mode) : active(true) { BeginBlendMode(mode); }
    ~Blend() { if (active) EndBlendMode(); }
    Blend(const Blend&) = delete;
    Blend& operator=(const Blend&) = delete;
private:
    bool active;
};

class CameraMode {
public:
    explicit CameraMode(Camera2D camera) : active(true) { BeginMode2D(camera); }
    ~CameraMode() { if (active) EndMode2D(); }
    CameraMode(const CameraMode&) = delete;
    CameraMode& operator=(const CameraMode&) = delete;
private:
    bool active;
};
}
