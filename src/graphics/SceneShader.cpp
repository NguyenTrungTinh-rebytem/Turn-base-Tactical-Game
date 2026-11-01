// src/graphics/SceneShader.cpp
#include "graphics/SceneShader.h"
#include <cmath>

namespace {
// GLSL 330 (raylib desktop OpenGL 3.3)
const char* const FRAGMENT_SHADER = R"(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3  tint;          // nhan mau (tong mau)
uniform float vignette;      // 0 = khong toi vien, 1 = rat toi
uniform float contrast;      // 1 = giu nguyen
uniform float saturation;    // 1 = giu nguyen, 0 = den trang

void main()
{
    vec4 c = texture(texture0, fragTexCoord) * colDiffuse * fragColor;

    float luma = dot(c.rgb, vec3(0.299, 0.587, 0.114));
    vec3 rgb = mix(vec3(luma), c.rgb, saturation);
    rgb = (rgb - 0.5) * contrast + 0.5;
    rgb *= tint;

    vec2 uv = fragTexCoord * 2.0 - 1.0;
    float d = length(uv * vec2(0.92, 1.0));
    float v = smoothstep(0.38, 1.30, d);
    rgb *= 1.0 - vignette * v;

    finalColor = vec4(clamp(rgb, 0.0, 1.0), c.a);
}
)";
}

SceneShader::~SceneShader() { unload(); }

bool SceneShader::init(int width, int height)
{
    unload();
    m_width = width;
    m_height = height;

    m_target = LoadRenderTexture(width, height);
    if (m_target.id == 0) {
        TraceLog(LOG_WARNING, "SceneShader: khong tao duoc RenderTexture -> tat shader.");
        return false;
    }
    SetTextureFilter(m_target.texture, TEXTURE_FILTER_BILINEAR);

    m_shader = LoadShaderFromMemory(nullptr, FRAGMENT_SHADER);
    m_locVignette = GetShaderLocation(m_shader, "vignette");
    m_locTint = GetShaderLocation(m_shader, "tint");
    m_locContrast = GetShaderLocation(m_shader, "contrast");
    m_locSaturation = GetShaderLocation(m_shader, "saturation");
    // Bien dich loi -> raylib tra ve shader mac dinh (khong co cac uniform tren)
    if (m_locVignette < 0 || m_locTint < 0 || m_locContrast < 0 || m_locSaturation < 0) {
        TraceLog(LOG_WARNING, "SceneShader: shader khong bien dich duoc -> game ve khong shader.");
        UnloadRenderTexture(m_target);
        m_target = RenderTexture2D{};
        return false;
    }
    m_ready = true;
    return true;
}

void SceneShader::unload()
{
    if (!m_ready) return;
    if (IsWindowReady()) {
        UnloadShader(m_shader);
        UnloadRenderTexture(m_target);
    }
    m_shader = Shader{};
    m_target = RenderTexture2D{};
    m_ready = false;
}

void SceneShader::begin()
{
    if (!isActive()) return;
    BeginTextureMode(m_target);
    ClearBackground(BLACK);
}

void SceneShader::end()
{
    if (!isActive()) return;
    EndTextureMode();
}

void SceneShader::draw(SceneMood mood, float timeSeconds)
{
    if (!isActive()) return;

    float tint[3], vignette, contrast, saturation;
    if (mood == SceneMood::BOSS) {
        // Boss: lanh/do toi, tuong phan cao, toi vien dap nhe
        tint[0] = 1.00f; tint[1] = 0.84f; tint[2] = 0.88f;
        vignette = 0.62f + 0.06f * std::sin(timeSeconds * 1.6f);
        contrast = 1.14f;
        saturation = 0.95f;
    } else {
        // Wave thuong: am, ruc ro nhe
        tint[0] = 1.05f; tint[1] = 1.00f; tint[2] = 0.93f;
        vignette = 0.42f;
        contrast = 1.08f;
        saturation = 1.10f;
    }
    SetShaderValue(m_shader, m_locTint, tint, SHADER_UNIFORM_VEC3);
    SetShaderValue(m_shader, m_locVignette, &vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locContrast, &contrast, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locSaturation, &saturation, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(m_shader);
    // RenderTexture bi lat doc -> chieu cao am
    DrawTextureRec(m_target.texture,
                   Rectangle{ 0.0f, 0.0f, static_cast<float>(m_width), -static_cast<float>(m_height) },
                   Vector2{ 0.0f, 0.0f }, WHITE);
    EndShaderMode();
}
