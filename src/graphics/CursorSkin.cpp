#include "graphics/CursorSkin.h"
#include "graphics/UiCanvas.h"
#include "raylib.h"
#include <algorithm>

namespace {
constexpr int   FRAME = 32;       // moi khung 32x32
constexpr float DRAW_SCALE = 1.0f;

struct Skin {
    const char* m_file;
    float m_frameSeconds;
    int   m_hotX, m_hotY;         // diem nhon (hotspot) trong khung
    Texture2D m_tex{};
    int   m_frames = 0;
};

Skin g_skins[5] = {
    { "cursor_normal.png",      0.100f, 1, 1 },
    { "cursor_link.png",        0.083f, 1, 1 },
    { "cursor_busy.png",        0.033f, 1, 1 },
    { "cursor_unavailable.png", 0.100f, 1, 1 },
    { "cursor_text.png",        0.090f, 1, 8 },
};
bool g_ready = false;
bool g_hover = false;
bool g_overrideActive = false;
CursorSkin::Kind g_override = CursorSkin::Kind::NORMAL;
}

bool CursorSkin::initialize(const std::string& directory) {
    shutdown();
    for (Skin& s : g_skins) {
        const std::string path = directory + "/" + s.m_file;
        if (!FileExists(path.c_str())) continue;
        s.m_tex = LoadTexture(path.c_str());
        if (s.m_tex.id == 0) continue;
        SetTextureFilter(s.m_tex, TEXTURE_FILTER_BILINEAR);
        s.m_frames = std::max(1, s.m_tex.width / FRAME);
    }
    g_ready = g_skins[0].m_tex.id != 0;          // toi thieu phai co con tro thuong
    if (g_ready) HideCursor();                   // an con tro he dieu hanh
    else TraceLog(LOG_WARNING, "CursorSkin: thieu %s/cursor_normal.png -> dung con tro cu.", directory.c_str());
    return g_ready;
}

void CursorSkin::shutdown() {
    for (Skin& s : g_skins) {
        if (s.m_tex.id != 0 && IsWindowReady()) UnloadTexture(s.m_tex);
        s.m_tex = Texture2D{};
        s.m_frames = 0;
    }
    g_ready = false;
}

void CursorSkin::setHover(bool overClickable) { g_hover = g_hover || overClickable; }

void CursorSkin::setOverride(Kind kind, bool active) { g_overrideActive = active; g_override = kind; }

bool CursorSkin::draw() {
    if (!g_ready) return false;
    if (!IsCursorOnScreen()) { g_hover = false; return true; }

    Kind kind = Kind::NORMAL;
    if (g_overrideActive) kind = g_override;
    else if (g_hover || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) kind = Kind::LINK;   // dang tro nut / dang bam
    g_hover = false;

    const Skin* skin = &g_skins[static_cast<int>(kind)];
    if (skin->m_tex.id == 0) skin = &g_skins[0];

    const int frame = static_cast<int>(GetTime() / skin->m_frameSeconds) % std::max(1, skin->m_frames);
    const Vector2 mouse = UiCanvas::mousePosition();
    const float size = FRAME * DRAW_SCALE;
    DrawTexturePro(skin->m_tex,
                   Rectangle{ static_cast<float>(frame * FRAME), 0.0f, static_cast<float>(FRAME), static_cast<float>(FRAME) },
                   Rectangle{ mouse.x - skin->m_hotX * DRAW_SCALE, mouse.y - skin->m_hotY * DRAW_SCALE, size, size },
                   Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    return true;
}
