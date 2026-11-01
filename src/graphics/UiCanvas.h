#pragma once

#include "raylib.h"

namespace UiCanvas {
class FrameGuard {
public:
    FrameGuard();
    ~FrameGuard();
    FrameGuard(const FrameGuard&) = delete;
    FrameGuard& operator=(const FrameGuard&) = delete;
    explicit operator bool() const { return active; }
private:
    bool active = false;
};

class SuspensionGuard {
public:
    SuspensionGuard();
    ~SuspensionGuard();
    SuspensionGuard(const SuspensionGuard&) = delete;
    SuspensionGuard& operator=(const SuspensionGuard&) = delete;
    explicit operator bool() const { return active; }
private:
    bool active = false;
};

bool initialize();
void shutdown();
void begin();
void end();
bool suspend();
void resume();
bool isDrawing();
int width();
int height();
Camera2D camera();
Vector2 mousePosition();
Vector2 mouseDelta();
float scale();
bool copyCurrentFrameTo(RenderTexture2D destination);
void drawCover(Texture2D texture, Rectangle destination);
}
