// Member 4 - Huynh Quang Khai | src/graphics/BattleMap.cpp  (2.5D: phoi canh + chieu sau)
#include "BattleMap.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float HORIZON_RATIO = 0.42f;  // duong chan troi o 42% chieu cao
constexpr float FEET_FAR   = 0.60f;     // dong chan hang xa  (ti le chieu cao man hinh)
constexpr float FEET_NEAR  = 0.72f;     // dong chan hang gan (< 0.78 de khong dung khung dieu khien phia duoi)
constexpr float SCALE_FAR  = 1.6f;      // sprite 64px -> ~166px
constexpr float SCALE_NEAR = 2.0f;      // sprite 64px -> ~205px
constexpr float REF_FRAME_PX = 64.0f;
constexpr float FRONT_X    = 0.36f;     // tuyen dau cach mep ngoai 38% chieu rong
constexpr float FRONT_X_PLAYER = 0.41f;  // tuyen dau cua phe nguoi choi (de khong che menu dieu khien)
constexpr float FRONT_X_ENEMY = 0.41f;   // tuyen dau cua phe dich (de khong che menu dieu khien)
constexpr float PLAYER_SIZE_MUL = 1.3f;   // THÊM MỚI: độ to của hero (1.0 = như cũ)
constexpr float EDGE_X     = 0.09f;     // don vi xa nhat cach mep >= 9%
constexpr float STEP_X     = 0.085f;
constexpr float STEP_X_ENEMY = 0.062f; // Visual layout only: keep rightmost enemies clear of anchored HUD    // moi don vi lui ra mep them 8.5% chieu rong
constexpr float GAP_FACTOR = 0.80f;     // 2 don vi CUNG hang: khoang cach tam >= 80% tong nua do rong
float lerpf(float a, float b, float t) { return a + (b - a) * t; }
}

BattleMap::~BattleMap() { unloadAll(); }

void BattleMap::unloadAll() {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);   // sau CloseWindow() thi bo qua, khong crash
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

void BattleMap::setStage(StageId stage, const std::string& backgroundPath) {
    m_stage = stage;
    unloadAll();
    if (!backgroundPath.empty() && FileExists(backgroundPath.c_str())) {
        m_background = LoadTexture(backgroundPath.c_str());
        m_hasBackground = (m_background.id != 0);
        if (m_hasBackground) SetTextureFilter(m_background, TEXTURE_FILTER_POINT);
    }
}

float BattleMap::groundY() const { return 720 * HORIZON_RATIO; }   // y cua duong chan troi

void BattleMap::draw() const {
    if (m_hasBackground) {
        Rectangle src = { 0, 0, (float)m_background.width, (float)m_background.height };
        Rectangle dst = { 0, 0, (float)1280, (float)720 };
        DrawTexturePro(m_background, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        drawProceduralBackground();
    }
}

void BattleMap::drawProceduralBackground() const {
    const int W = 1280, H = 720;
    const float hz = groundY();
    const bool s1 = (m_stage == StageId::STAGE_1);

    Color skyTop    = s1 ? Color{ 110, 180, 240, 255 } : Color{ 40, 10, 35, 255 };
    Color skyBottom = s1 ? Color{ 205, 232, 250, 255 } : Color{ 150, 45, 45, 255 };
    Color hill      = s1 ? Color{ 90, 150, 100, 255 }  : Color{ 70, 30, 50, 255 };
    Color groundA   = s1 ? Color{ 84, 140, 72, 255 }   : Color{ 62, 52, 66, 255 };
    Color groundB   = s1 ? Color{ 72, 124, 62, 255 }   : Color{ 50, 42, 54, 255 };
    Color orb       = s1 ? Color{ 255, 230, 120, 255 } : Color{ 230, 70, 60, 255 };

    // Troi + mat troi/mat trang + doi xa
    DrawRectangleGradientV(0, 0, W, (int)hz, skyTop, skyBottom);
    DrawCircle((int)(W * 0.80f), (int)(hz * 0.38f), 52.0f, orb);
    for (int i = 0; i < 7; ++i)
        DrawCircle((int)(i * W / 6.0f), (int)hz + 25, 110.0f + (i % 3) * 30.0f, hill);

    // Mat dat: cac hang chia theo phoi canh (hang gan cao hon)
    constexpr int ROWS = 10;
    for (int k = 0; k < ROWS; ++k) {
        float y0 = hz + (H - hz) * std::pow(k / (float)ROWS, 1.6f);
        float y1 = hz + (H - hz) * std::pow((k + 1) / (float)ROWS, 1.6f);
        DrawRectangle(0, (int)y0, W, (int)std::ceil(y1 - y0) + 1, (k % 2 == 0) ? groundA : groundB);
    }
    // Duong phoi canh hoi tu o chan troi
    for (int i = -8; i <= 8; ++i)
        DrawLineEx(Vector2{ W / 2.0f + i * 22.0f, hz }, Vector2{ W / 2.0f + i * W / 7.0f, (float)H },
                   1.5f, Color{ 0, 0, 0, 35 });
    DrawLine(0, (int)hz, W, (int)hz, Color{ 0, 0, 0, 60 });
}

void BattleMap::drawShadow(const BattleSlot& slot, float frameWidth, float alpha) const {
    float rx = frameWidth * slot.m_scale * 0.30f;
    float a = std::max(0.0f, std::min(1.0f, alpha));
    DrawEllipse((int)slot.m_feetPos.x, (int)slot.m_feetPos.y, rx, rx * 0.22f,
                Color{ 0, 0, 0, static_cast<unsigned char>(90 * a) });
}

// Xep le 2 hang: chan = hang gan (to, thap), le = hang xa (nho, cao). 1 don vi => dung giua 2 hang.
// Cac don vi CUNG hang khong de nhau theo do rong that (Boss to hon van khong bi dung); qua dong thi thu nho deu.
std::vector<BattleSlot> BattleMap::layoutSide(const std::vector<float>& scaleMuls, bool isPlayer) const {
    std::vector<BattleSlot> slots(scaleMuls.size());
    if (scaleMuls.empty()) return slots;

    const float W = (float)1280, H = (float)720;
    const size_t n = scaleMuls.size();

    auto mulOf = [&](size_t i) { return std::max(0.1f, scaleMuls[i] * (isPlayer ? PLAYER_SIZE_MUL : 1.0f)); };
    auto depthOf = [&](size_t i) { return (n == 1) ? 0.5f : (i % 2 == 0 ? 1.0f : 0.0f); };
    auto widthOf = [&](size_t i) { return REF_FRAME_PX * lerpf(SCALE_FAR, SCALE_NEAR, depthOf(i)) * mulOf(i); };

    // do lech (px) tu tuyen dau ra phia mep man hinh
    std::vector<float> offset(n, 0.0f);
    for (size_t i = 1; i < n; ++i) {
        offset[i] = offset[i - 1] + (isPlayer ? STEP_X : STEP_X_ENEMY) * W;
        if (i >= 2)   // cung hang voi i-2
            offset[i] = std::max(offset[i], offset[i - 2] + 0.5f * (widthOf(i) + widthOf(i - 2)) * GAP_FACTOR);
    }
    const float frontX = isPlayer ? FRONT_X_PLAYER : FRONT_X_ENEMY;
    const float available = W * (frontX - EDGE_X);
    const float k = (n > 1 && offset[n - 1] > available) ? available / offset[n - 1] : 1.0f;

    for (size_t i = 0; i < n; ++i) {
        const float t = depthOf(i);
        const float distFromEdge = W * frontX - offset[i] * k;
        slots[i].m_feetPos.x = isPlayer ? distFromEdge : W - distFromEdge;
        slots[i].m_feetPos.y = lerpf(H * FEET_FAR, H * FEET_NEAR, t);
        slots[i].m_scale = lerpf(SCALE_FAR, SCALE_NEAR, t) * mulOf(i) * k;
        slots[i].m_flipX = !isPlayer;
        slots[i].m_depth = t;
    }
    return slots;
}

std::vector<BattleSlot> BattleMap::layoutPlayers(const std::vector<float>& scaleMuls) const { return layoutSide(scaleMuls, true); }
std::vector<BattleSlot> BattleMap::layoutEnemies(const std::vector<float>& scaleMuls) const { return layoutSide(scaleMuls, false); }

BattleSlot BattleMap::pickSlot(int index, int count, bool isPlayer, float scaleMul) const {
    const int n = std::max(1, count);
    const int i = std::max(0, std::min(index, n - 1));
    std::vector<float> muls(static_cast<size_t>(n), 1.0f);
    muls[static_cast<size_t>(i)] = scaleMul;
    return layoutSide(muls, isPlayer)[static_cast<size_t>(i)];
}

BattleSlot BattleMap::getPlayerSlot(int index, int count, float scaleMul) const { return pickSlot(index, count, true, scaleMul); }
BattleSlot BattleMap::getEnemySlot(int index, int count, float scaleMul) const { return pickSlot(index, count, false, scaleMul); }

Vector2 BattleMap::toTopLeft(const BattleSlot& slot, int frameW, int frameH) {
    return { slot.m_feetPos.x - frameW * slot.m_scale * 0.5f, slot.m_feetPos.y - frameH * slot.m_scale };
}

Vector2 BattleMap::getHeadPos(const BattleSlot& slot, int frameH) {
    return { slot.m_feetPos.x, slot.m_feetPos.y - frameH * slot.m_scale - 10.0f };
}