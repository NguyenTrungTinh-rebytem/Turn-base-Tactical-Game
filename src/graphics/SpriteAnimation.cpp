// Member 4 - Huynh Quang Khai | src/graphics/SpriteAnimation.cpp
#include "SpriteAnimation.h"

SpriteAnimation::~SpriteAnimation() {
    if (m_loaded) UnloadTexture(m_texture);
}

bool SpriteAnimation::load(const std::string& path, int frameWidth, int frameHeight) {
    if (m_loaded) { UnloadTexture(m_texture); m_loaded = false; }

    m_frameWidth  = frameWidth;
    m_frameHeight = frameHeight;

    // Mac dinh: hang 0 = IDLE, 1 = ATTACK, 2 = TAKE_DAMAGE (chinh bang setClip neu sheet khac)
    m_clips[static_cast<size_t>(AnimState::IDLE)]        = { 0, 4, 0.18f, true  };
    m_clips[static_cast<size_t>(AnimState::ATTACK)]      = { 1, 4, 0.10f, false };
    m_clips[static_cast<size_t>(AnimState::TAKE_DAMAGE)] = { 2, 2, 0.12f, false };

    if (!FileExists(path.c_str())) return false;
    m_texture = LoadTexture(path.c_str());
    m_loaded  = (m_texture.id != 0);
    if (m_loaded) SetTextureFilter(m_texture, TEXTURE_FILTER_POINT); // Pixel Art: khong lam mo
    return m_loaded;
}

void SpriteAnimation::setClip(AnimState state, const AnimClip& clip) {
    m_clips[static_cast<size_t>(state)] = clip;
}

void SpriteAnimation::play(AnimState state, bool restart) {
    if (state == m_state && !restart && !m_oneShotFinished) return;
    m_state = state;
    m_currentFrame = 0;
    m_timer = 0.0f;
    m_oneShotFinished = false;
}

void SpriteAnimation::update(float deltaTime) {
    const AnimClip& clip = currentClip();
    if (clip.m_frameCount <= 1 && clip.m_loop) return;

    m_timer += deltaTime;
    while (m_timer >= clip.m_frameTime) {
        m_timer -= clip.m_frameTime;
        if (m_currentFrame + 1 < clip.m_frameCount) {
            ++m_currentFrame;
        } else if (clip.m_loop) {
            m_currentFrame = 0;
        } else {
            // Clip 1-lan ket thuc -> quay ve IDLE
            m_oneShotFinished = true;
            m_state = AnimState::IDLE;
            m_currentFrame = 0;
            m_timer = 0.0f;
            return;
        }
    }
}

void SpriteAnimation::drawFrame(Vector2 pos, bool flipX, float scale, Color tint) const {
    const AnimClip& clip = currentClip();
    const float fw = static_cast<float>(m_frameWidth);
    const float fh = static_cast<float>(m_frameHeight);
    Rectangle dest = { pos.x, pos.y, fw * scale, fh * scale };

    if (!m_loaded) {
        // Placeholder de team test khi chua co asset
        Color c = (m_state == AnimState::ATTACK) ? ORANGE
                : (m_state == AnimState::TAKE_DAMAGE) ? RED : SKYBLUE;
        DrawRectangleRec(dest, c);
        DrawRectangleLinesEx(dest, 2, DARKGRAY);
        DrawText(TextFormat("f%d", m_currentFrame), (int)dest.x + 4, (int)dest.y + 4, 10, BLACK);
        return;
    }

    // Chieu rong am => Raylib lat ngang texture
    Rectangle src = {
        fw * m_currentFrame,
        fh * clip.m_row,
        flipX ? -fw : fw,
        fh
    };
    DrawTexturePro(m_texture, src, dest, Vector2{ 0, 0 }, 0.0f, tint);
}