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

uniform vec2  resolution;
uniform float time;
uniform vec3  tint;
uniform vec3  rayColor;
uniform float vignette;
uniform float contrast;
uniform float saturation;
uniform float bloom;        // do toa sang
uniform float dof;          // do mo tilt-shift (pixel)
uniform float rays;         // cuong do tia sang

vec3 samp(vec2 uv) { return texture(texture0, clamp(uv, vec2(0.001), vec2(0.999))).rgb; }

void main()
{
    vec2 uv = fragTexCoord;
    vec2 px = 1.0 / resolution;

    // --- Tilt-shift: mo dan o tren/duoi, giua (khu chien dau) sac net ---
    float focus = smoothstep(0.20, 0.62, abs(uv.y - 0.60));
    float r = dof * focus;
    vec3 col = samp(uv);
    if (r > 0.4) {
        vec3 acc = col; float wsum = 1.0;
        for (int i = 0; i < 12; ++i) {
            float a = float(i) * 2.399963;               // goc vang
            float rad = sqrt((float(i) + 0.5) / 12.0) * r;
            acc += samp(uv + vec2(cos(a), sin(a)) * rad * px);
            wsum += 1.0;
        }
        col = acc / wsum;
    }

    // --- Bloom: lay vung sang, lam mo rong, cong lai ---
    vec3 glow = vec3(0.0);
    for (int i = 0; i < 16; ++i) {
        float a = float(i) * 2.399963;
        float rad = (0.25 + float(i) / 16.0) * 14.0;
        vec3 s = samp(uv + vec2(cos(a), sin(a)) * rad * px);
        float l = dot(s, vec3(0.299, 0.587, 0.114));
        glow += s * smoothstep(0.50, 0.95, l);
    }
    col += glow / 16.0 * bloom * 2.2;

    // --- Tia sang xien tu goc tren ---
    float d = uv.x * 0.9 + uv.y * 0.5;
    float shafts = sin(d * 17.0 + time * 0.25) * 0.5 + 0.5;
    shafts *= sin(d * 7.3 - time * 0.15) * 0.5 + 0.5;
    shafts = pow(shafts, 1.6);
    float fade = smoothstep(0.85, 0.0, uv.y) * smoothstep(0.0, 0.25, uv.x + 0.15);
    col += rayColor * shafts * fade * rays;

    // --- Chinh mau ---
    float luma = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(vec3(luma), col, saturation);
    col = (col - 0.5) * contrast + 0.5;
    // split-tone: bong ngả xanh ngoc, vung sang ngả am
    col += vec3(-0.03, 0.02, 0.04) * (1.0 - smoothstep(0.0, 0.5, luma));
    col += vec3(0.05, 0.025, -0.02) * smoothstep(0.5, 1.0, luma);
    col *= tint;

    // --- Vignette ---
    vec2 q = uv * 2.0 - 1.0;
    float v = smoothstep(0.40, 1.30, length(q * vec2(0.92, 1.0)));
    col *= 1.0 - vignette * v;

    finalColor = vec4(clamp(col, 0.0, 1.0), 1.0);
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
    m_locRes = GetShaderLocation(m_shader, "resolution");
    m_locTime = GetShaderLocation(m_shader, "time");
    m_locBloom = GetShaderLocation(m_shader, "bloom");
    m_locDof = GetShaderLocation(m_shader, "dof");
    m_locRays = GetShaderLocation(m_shader, "rays");
    m_locRayColor = GetShaderLocation(m_shader, "rayColor");
    // Bien dich loi -> raylib tra ve shader mac dinh (khong co cac uniform tren)
    if (m_locVignette < 0 || m_locTint < 0 || m_locContrast < 0 || m_locSaturation < 0 || m_locBloom < 0 || m_locDof < 0 || m_locRays < 0 || m_locRes < 0) {
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

    float tint[3], rayColor[3], vignette, contrast, saturation, bloom, dof, rays;
    if (mood == SceneMood::BOSS) {
        // Boss: lanh/do toi, tia sang do, bloom manh
        tint[0] = 1.00f; tint[1] = 0.86f; tint[2] = 0.90f;
        rayColor[0] = 0.55f; rayColor[1] = 0.12f; rayColor[2] = 0.08f;
        vignette = 0.60f + 0.06f * std::sin(timeSeconds * 1.6f);
        contrast = 1.14f; saturation = 1.00f;
        bloom = 0.85f; dof = 5.0f; rays = 0.20f;
    } else {
        // Wave thuong: xanh ngoc am, tia sang vang nhe
        tint[0] = 1.02f; tint[1] = 1.02f; tint[2] = 0.96f;
        rayColor[0] = 0.40f; rayColor[1] = 0.50f; rayColor[2] = 0.30f;
        vignette = 0.38f;
        contrast = 1.10f; saturation = 1.15f;
        bloom = 0.70f; dof = 4.0f; rays = 0.16f;
    }
    float res[2] = { static_cast<float>(m_width), static_cast<float>(m_height) };
    SetShaderValue(m_shader, m_locRes, res, SHADER_UNIFORM_VEC2);
    SetShaderValue(m_shader, m_locTime, &timeSeconds, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locTint, tint, SHADER_UNIFORM_VEC3);
    SetShaderValue(m_shader, m_locRayColor, rayColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(m_shader, m_locVignette, &vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locContrast, &contrast, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locSaturation, &saturation, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locBloom, &bloom, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locDof, &dof, SHADER_UNIFORM_FLOAT);
    SetShaderValue(m_shader, m_locRays, &rays, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(m_shader);
    // RenderTexture bi lat doc -> chieu cao am
    DrawTextureRec(m_target.texture,
                   Rectangle{ 0.0f, 0.0f, static_cast<float>(m_width), -static_cast<float>(m_height) },
                   Vector2{ 0.0f, 0.0f }, WHITE);
    EndShaderMode();

    updateEmbers(GetFrameTime(), mood);
    drawEmbers(mood, timeSeconds);
}

void SceneShader::updateEmbers(float dt, SceneMood mood)
{
    const size_t target = (mood == SceneMood::BOSS) ? 70 : 45;
    if (dt > 0.1f) dt = 0.1f;
    while (m_embers.size() < target) {
        Ember e;
        e.x = static_cast<float>(GetRandomValue(0, m_width));
        e.y = static_cast<float>(GetRandomValue(m_height / 3, m_height + 40));
        e.vx = static_cast<float>(GetRandomValue(-8, 14));
        e.vy = -static_cast<float>(GetRandomValue(12, 45));
        e.size = static_cast<float>(GetRandomValue(8, 22)) / 10.0f;
        e.phase = static_cast<float>(GetRandomValue(0, 628)) / 100.0f;
        e.life = static_cast<float>(GetRandomValue(30, 90)) / 10.0f;
        m_embers.push_back(e);
    }
    for (size_t i = 0; i < m_embers.size();) {
        Ember& e = m_embers[i];
        e.life -= dt;
        e.x += (e.vx + std::sin(e.phase + e.life * 1.8f) * 10.0f) * dt;
        e.y += e.vy * dt;
        if (e.life <= 0.0f || e.y < -10.0f) { m_embers[i] = m_embers.back(); m_embers.pop_back(); }
        else ++i;
    }
}

void SceneShader::drawEmbers(SceneMood mood, float t)
{
    const Color core = (mood == SceneMood::BOSS) ? Color{ 255, 90, 50, 255 } : Color{ 255, 190, 90, 255 };
    BeginBlendMode(BLEND_ADDITIVE);
    for (const Ember& e : m_embers) {
        float tw = 0.55f + 0.45f * std::sin(t * 4.0f + e.phase);
        float fadeIn = std::fmin(1.0f, e.life);                // tat dan khi het song
        unsigned char a = static_cast<unsigned char>(200.0f * tw * fadeIn);
        DrawCircleV(Vector2{ e.x, e.y }, e.size * 2.6f, Color{ core.r, core.g, core.b, static_cast<unsigned char>(a / 6) });
        DrawCircleV(Vector2{ e.x, e.y }, e.size, Color{ core.r, core.g, core.b, a });
    }
    EndBlendMode();
}
