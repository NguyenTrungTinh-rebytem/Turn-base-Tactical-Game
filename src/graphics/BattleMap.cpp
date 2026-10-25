// Member 4 - Huynh Quang Khai | src/graphics/BattleMap.cpp  (2D nhin ngang)
#include "BattleMap.h"
#include <algorithm>

namespace {
constexpr float GROUND_RATIO = 0.72f;   // mat dat o 72% chieu cao man hinh
constexpr float SPRITE_SCALE = 3.0f;    // sprite 64px -> 192px
constexpr float REF_FRAME_PX = 64.0f;   // kich thuoc frame tham chieu de tinh do rong
constexpr float FRONT_X      = 0.40f;   // tuyen dau cach mep ngoai 40% chieu rong (2 phe cach nhau 20%)
constexpr float EDGE_X       = 0.09f;   // don vi xa nhat cach mep >= 9%
constexpr float GAP_FACTOR   = 0.80f;   // khoang cach 2 tam = 80% (tong 2 nua do rong): cho phep cham mep trong suot
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

float BattleMap::groundY() const { return GetScreenHeight() * GROUND_RATIO; }

void BattleMap::draw() const {
    if (m_hasBackground) {
        Rectangle src = { 0, 0, (float)m_background.width, (float)m_background.height };
        Rectangle dst = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(m_background, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        drawProceduralBackground();
    }
}

void BattleMap::drawProceduralBackground() const {
    const int W = GetScreenWidth(), H = GetScreenHeight();
    const int gy = (int)groundY();
    const bool s1 = (m_stage == StageId::STAGE_1);

    Color skyTop    = s1 ? Color{ 110, 180, 240, 255 } : Color{ 40, 10, 35, 255 };
    Color skyBottom = s1 ? Color{ 205, 232, 250, 255 } : Color{ 150, 45, 45, 255 };
    Color hill      = s1 ? Color{ 90, 150, 100, 255 }  : Color{ 70, 30, 50, 255 };
    Color grass     = s1 ? Color{ 84, 150, 72, 255 }   : Color{ 100, 60, 90, 255 };
    Color dirtA     = s1 ? Color{ 120, 84, 56, 255 }   : Color{ 62, 52, 66, 255 };
    Color dirtB     = s1 ? Color{ 108, 74, 48, 255 }   : Color{ 50, 42, 54, 255 };
    Color orb       = s1 ? Color{ 255, 230, 120, 255 } : Color{ 230, 70, 60, 255 };

    DrawRectangleGradientV(0, 0, W, gy, skyTop, skyBottom);
    DrawCircle((int)(W * 0.80f), (int)(gy * 0.30f), 50.0f, orb);

    for (int i = 0; i < 7; ++i)
        DrawCircle((int)(i * W / 6.0f), gy + 30, 110.0f + (i % 3) * 30.0f, hill);

    constexpr int TILE = 32;
    for (int y = gy, row = 0; y < H; y += TILE, ++row)
        for (int x = 0, col = 0; x < W; x += TILE, ++col)
            DrawRectangle(x, y, TILE, TILE, ((row + col) % 2 == 0) ? dirtA : dirtB);
    DrawRectangle(0, gy, W, 10, grass);
    DrawRectangle(0, gy + 10, W, 2, Color{ 0, 0, 0, 50 });
}

void BattleMap::drawShadow(const BattleSlot& slot, float frameWidth, float alpha) const {
    float rx = frameWidth * slot.m_scale * 0.30f;
    float a = std::max(0.0f, std::min(1.0f, alpha));
    DrawEllipse((int)slot.m_feetPos.x, (int)slot.m_feetPos.y, rx, rx * 0.22f,
                Color{ 0, 0, 0, static_cast<unsigned char>(90 * a) });
}

// Xep cac don vi theo DO RONG THAT (khong de nhau, Boss to hon van khong bi dung), thu nho deu neu qua dong.
std::vector<BattleSlot> BattleMap::layoutSide(const std::vector<float>& scaleMuls, bool isPlayer) const {
    std::vector<BattleSlot> slots(scaleMuls.size());
    if (scaleMuls.empty()) return slots;

    const float W = (float)GetScreenWidth();
    const size_t n = scaleMuls.size();

    auto widthOf = [&](size_t i) { return REF_FRAME_PX * SPRITE_SCALE * std::max(0.1f, scaleMuls[i]); };

    // do lech tu tuyen dau (px, huong ra mep man hinh)
    std::vector<float> offset(n, 0.0f);
    for (size_t i = 1; i < n; ++i)
        offset[i] = offset[i - 1] + 0.5f * (widthOf(i - 1) + widthOf(i)) * GAP_FACTOR;

    const float available = W * (FRONT_X - EDGE_X);
    const float k = (n > 1 && offset[n - 1] > available) ? available / offset[n - 1] : 1.0f;

    for (size_t i = 0; i < n; ++i) {
        float distFromEdge = W * FRONT_X - offset[i] * k;
        slots[i].m_feetPos.x = isPlayer ? distFromEdge : W - distFromEdge;
        slots[i].m_feetPos.y = groundY();
        slots[i].m_scale = SPRITE_SCALE * std::max(0.1f, scaleMuls[i]) * k;
        slots[i].m_flipX = !isPlayer;
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