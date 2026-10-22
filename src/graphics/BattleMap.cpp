// Member 4 - Huynh Quang Khai | src/graphics/BattleMap.cpp  (2D nhin ngang)
#include "BattleMap.h"
#include <algorithm>

namespace {
constexpr float GROUND_RATIO = 0.72f;   // mat dat nam o 72% chieu cao man hinh
constexpr float SPRITE_SCALE = 3.0f;    // sprite 64px -> 192px
constexpr float FRONT_X      = 0.36f;   // vi tri tuyen dau (ti le chieu rong, tinh tu moi ben)
constexpr float MAX_SPACING  = 0.15f;
constexpr float EDGE_MARGIN  = 0.10f;
}

BattleMap::~BattleMap() {
    if (m_hasBackground) UnloadTexture(m_background);
}

void BattleMap::setStage(StageId stage, const std::string& backgroundPath) {
    m_stage = stage;
    if (m_hasBackground) { UnloadTexture(m_background); m_hasBackground = false; }
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

    // Doi nui phia sau
    for (int i = 0; i < 7; ++i)
        DrawCircle((int)(i * W / 6.0f), gy + 30, 110.0f + (i % 3) * 30.0f, hill);

    // Mat dat: dai co + cac o dat 32px kieu tile pixel
    constexpr int TILE = 32;
    for (int y = gy, row = 0; y < H; y += TILE, ++row)
        for (int x = 0, col = 0; x < W; x += TILE, ++col)
            DrawRectangle(x, y, TILE, TILE, ((row + col) % 2 == 0) ? dirtA : dirtB);
    DrawRectangle(0, gy, W, 10, grass);
    DrawRectangle(0, gy + 10, W, 2, Color{ 0, 0, 0, 50 });
}

void BattleMap::drawShadow(const BattleSlot& slot, float frameWidth) const {
    float rx = frameWidth * slot.m_scale * 0.30f;
    DrawEllipse((int)slot.m_feetPos.x, (int)slot.m_feetPos.y, rx, rx * 0.22f, Color{ 0, 0, 0, 90 });
}

BattleSlot BattleMap::makeSlot(int index, int count, bool isPlayer, float scaleMul) const {
    const float W = (float)GetScreenWidth();

    // Dan deu sang ben ngoai man hinh, tu dong thu hep khoang cach neu dong nguoi
    float spacing = (count > 1) ? std::min(MAX_SPACING, (FRONT_X - EDGE_MARGIN) / (count - 1)) : 0.0f;
    float ratio = FRONT_X - spacing * index;

    BattleSlot s;
    s.m_feetPos.x = isPlayer ? W * ratio : W * (1.0f - ratio);
    s.m_feetPos.y = groundY();
    s.m_scale = SPRITE_SCALE * scaleMul;
    s.m_flipX = !isPlayer;
    return s;
}

BattleSlot BattleMap::getPlayerSlot(int index, int count, float scaleMul) const { return makeSlot(index, count, true, scaleMul); }
BattleSlot BattleMap::getEnemySlot(int index, int count, float scaleMul) const { return makeSlot(index, count, false, scaleMul); }

Vector2 BattleMap::toTopLeft(const BattleSlot& slot, int frameW, int frameH) {
    return { slot.m_feetPos.x - frameW * slot.m_scale * 0.5f, slot.m_feetPos.y - frameH * slot.m_scale };
}

Vector2 BattleMap::getHeadPos(const BattleSlot& slot, int frameH) {
    return { slot.m_feetPos.x, slot.m_feetPos.y - frameH * slot.m_scale - 10.0f };
}