#pragma once
// src/graphics/SceneShader.h
// Shader hau ky cho CANH tran dau, phong cach HD-2D: bloom (toa sang), lam mo tilt-shift (xa mo/gan net),
// tia sang tu tren xuong, chinh mau split-tone (bong xanh ngoc / sang am), vignette + tro bay (ember).
// Chi ap dung cho nen + nhan vat; UI/HP bar/chu ve SAU nen van sac net.
// Khong tao duoc shader (may cu / loi GLSL) -> isActive() = false, game ve binh thuong.
#include "raylib.h"
#include "RaylibScope.h"
#include "UiCanvas.h"
#include <memory>
#include <vector>

enum class SceneMood { NORMAL = 0, BOSS };

class SceneShader {
public:
    SceneShader() = default;
    ~SceneShader();
    SceneShader(const SceneShader&) = delete;
    SceneShader& operator=(const SceneShader&) = delete;

    bool init(int width, int height);     // goi sau InitWindow
    void unload();                        // goi truoc CloseWindow. An toan khi goi nhieu lan

    bool isActive() const { return m_ready && m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Cach dung:  if (isActive()) { begin(); ...ve canh...; end(); draw(mood, time); } else { ...ve canh... }
    void begin();
    void end();
    void draw(SceneMood mood, float timeSeconds);

private:
    bool          m_ready = false;
    bool          m_enabled = true;
    int           m_width = 0, m_height = 0;
    RenderTexture2D m_target{};
    Shader        m_shader{};
    int m_locTint = -1, m_locVignette = -1, m_locContrast = -1, m_locSaturation = -1;
    int m_locRes = -1, m_locTime = -1, m_locBloom = -1, m_locDof = -1, m_locRays = -1, m_locRayColor = -1;
    int m_locLightPos = -1, m_locLightColor = -1, m_locRelief = -1, m_locHaze = -1, m_locHazeColor = -1, m_locAo = -1;

    struct Ember { float x, y, vx, vy, size, phase, life; };
    std::vector<Ember> m_embers;
    std::unique_ptr<UiCanvas::SuspensionGuard> m_canvasSuspension;
    std::unique_ptr<RaylibScope::TextureMode> m_targetScope;
    std::unique_ptr<RaylibScope::CameraMode> m_cameraScope;
    bool m_captureValid = false;
    void updateEmbers(float dt, SceneMood mood);
    void drawEmbers(SceneMood mood, float t);
};
