#pragma once
// src/audio/AudioManager.h
// Nhac nen (Music stream) + hieu ung (Sound). Thieu file -> im lang, KHONG crash.
// GameEngine goi: init() -> loadAll(thuMucAudio) -> playMusic()/playSfx() -> update(dt) moi frame -> unloadAll() truoc CloseWindow().
#include "raylib.h"
#include <string>
#include <unordered_map>

enum class MusicId { NONE = 0, MENU, AMBIENT, BATTLE, GAME_OVER };

enum class SfxId {
    CLICK = 0,
    HERO_MALE_ATTACK, HERO_MALE_HURT, HERO_MALE_DEATH,
    HERO_FEMALE_ATTACK, HERO_FEMALE_HURT, HERO_FEMALE_DEATH,
    MONSTER_ATTACK, MONSTER_HURT,
    DRAGON_ATTACK, DRAGON_HURT,
    COUNT
};

class AudioManager {
public:
    AudioManager() = default;
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool init();                                   // InitAudioDevice (sau InitWindow). false = khong co thiet bi am thanh
    void loadAll(const std::string& audioDir);     // doc moi file trong <audioDir>/ (xem ten file trong .cpp)
    void unloadAll();                              // giai phong + CloseAudioDevice. An toan khi goi nhieu lan.

    void playMusic(MusicId id);                    // doi bai: tat dan bai cu, vao dan bai moi. Cung bai dang phat -> bo qua
    void playSfx(SfxId id);
    void update(float deltaTime);                  // PHAI goi moi frame (nuoi du lieu cho music stream)

    void setMusicVolume(float volume);             // 0..1
    void setSfxVolume(float volume);               // 0..1
    float getMusicVolume() const { return m_musicVolume; }
    float getSfxVolume() const { return m_sfxVolume; }
    bool isReady() const { return m_ready; }

private:
    struct Track { Music m_music{}; bool m_loaded = false; };

    static constexpr float FADE_IN_SECONDS  = 0.8f;
    static constexpr float FADE_OUT_SECONDS = 0.4f;

    void applyMusicVolume();
    Track* trackOf(MusicId id);

    bool  m_ready = false;
    std::unordered_map<int, Track> m_tracks;
    std::unordered_map<int, Sound> m_sounds;
    std::unordered_map<int, float> m_sfxGain;      // do to rieng tung hieu ung (can bang am luong)

    MusicId m_current = MusicId::NONE;
    MusicId m_next = MusicId::NONE;                // bai dang cho (sau khi bai cu tat dan)
    float   m_fade = 0.0f;                         // 0..1 nhan voi am luong nhac
    bool    m_fadingOut = false;
    float   m_musicVolume = 0.45f;
    float   m_sfxVolume = 0.8f;
};
