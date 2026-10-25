#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/SpriteAnimation.h
#include "raylib.h"
#include <array>
#include <string>

// Trang thai hoat anh (UPPER_SNAKE_CASE)
enum class AnimState { IDLE = 0, ATTACK, TAKE_DAMAGE, COUNT };

// Mo ta 1 clip: nam tren 1 hang (row) cua sprite sheet
struct AnimClip {
    int   m_row        = 0;      // hang trong sprite sheet
    int   m_frameCount = 1;      // so frame cua clip
    float m_frameTime  = 0.12f;  // giay / frame
    bool  m_loop       = true;   // IDLE lap, ATTACK/TAKE_DAMAGE chay 1 lan roi ve IDLE
};

class SpriteAnimation {
public:
    SpriteAnimation() = default;
    ~SpriteAnimation();

    // Texture la tai nguyen GPU -> cam copy (dung qua std::unique_ptr)
    SpriteAnimation(const SpriteAnimation&) = delete;
    SpriteAnimation& operator=(const SpriteAnimation&) = delete;

    // Load sprite sheet; frameWidth/frameHeight la kich thuoc 1 o frame.
    // Tra ve false neu khong load duoc (van ve placeholder de test khong can asset).
    bool load(const std::string& path, int frameWidth, int frameHeight);

    // Giai phong texture. Core goi truoc CloseWindow(). Goi nhieu lan van an toan.
    void unload();

    void setClip(AnimState state, const AnimClip& clip);
    void play(AnimState state, bool restart = false);
    void update(float deltaTime);

    // Ve frame hien tai. pos = goc tren-trai; flipX de lat ngang (quai quay mat sang trai)
    void drawFrame(Vector2 pos, bool flipX = false, float scale = 1.0f, Color tint = WHITE) const;

    AnimState getState() const { return m_state; }
    bool      isLoaded() const { return m_loaded; }
    // true khi clip khong-lap (ATTACK/TAKE_DAMAGE) vua chay xong
    bool      isOneShotFinished() const { return m_oneShotFinished; }
    int       getFrameWidth() const { return m_frameWidth; }
    int       getFrameHeight() const { return m_frameHeight; }
    int       getCurrentFrame() const { return m_currentFrame; }
    int       getCurrentRow() const { return currentClip().m_row; }
    const Texture2D& getTexture() const { return m_texture; }

private:
    Texture2D m_texture{};
    bool      m_loaded = false;
    int       m_frameWidth  = 64;
    int       m_frameHeight = 64;

    std::array<AnimClip, static_cast<size_t>(AnimState::COUNT)> m_clips;
    AnimState m_state = AnimState::IDLE;
    int       m_currentFrame = 0;
    float     m_timer = 0.0f;
    bool      m_oneShotFinished = false;

    const AnimClip& currentClip() const { return m_clips[static_cast<size_t>(m_state)]; }
};