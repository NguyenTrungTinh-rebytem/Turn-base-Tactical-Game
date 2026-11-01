#pragma once
// src/graphics/SceneShader.h
// Shader hau ky (post-process) cho CANH tran dau: vignette (toi vien) + chinh mau (tong mau, tuong phan, bao hoa).
// Chi ap dung cho nen + nhan vat; UI/HP bar/chu ve SAU nen van sac net.
// Khong tao duoc shader (may cu / loi GLSL) -> isActive() = false, game ve binh thuong.
#include "raylib.h"

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
};
