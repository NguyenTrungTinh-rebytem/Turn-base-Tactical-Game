// src/audio/AudioManager.cpp
#include "audio/AudioManager.h"
#include <algorithm>

namespace {
struct MusicFile { MusicId m_id; const char* m_file; };
const MusicFile MUSIC_FILES[] = {
    { MusicId::MENU,      "music_menu.mp3"     },   // man hinh Start
    { MusicId::AMBIENT,   "music_ambient.mp3"  },   // chon tuong / chon wave / chuan bi / thang
    { MusicId::BATTLE,    "music_battle.mp3"   },   // trong tran
    { MusicId::GAME_OVER, "music_gameover.mp3" },
};
struct SfxFile { SfxId m_id; const char* m_file; float m_gain; };
const SfxFile SFX_FILES[] = {
    { SfxId::CLICK,              "click.ogg",              0.7f },
    { SfxId::HERO_MALE_ATTACK,   "hero_male_attack.wav",   1.0f },
    { SfxId::HERO_MALE_HURT,     "hero_male_hurt.wav",     1.0f },
    { SfxId::HERO_MALE_DEATH,    "hero_male_death.wav",    1.0f },
    { SfxId::HERO_FEMALE_ATTACK, "hero_female_attack.wav", 1.0f },
    { SfxId::HERO_FEMALE_HURT,   "hero_female_hurt.wav",   1.0f },
    { SfxId::HERO_FEMALE_DEATH,  "hero_female_death.wav",  1.0f },
    { SfxId::MONSTER_ATTACK,     "monster_attack.wav",     0.9f },
    { SfxId::MONSTER_HURT,       "monster_hurt.wav",       0.9f },
    { SfxId::DRAGON_ATTACK,      "dragon_attack.wav",      1.0f },
    { SfxId::DRAGON_HURT,        "dragon_hurt.wav",        1.0f },
};
float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
}

AudioManager::~AudioManager() { unloadAll(); }

bool AudioManager::init()
{
    if (m_ready) return true;
    InitAudioDevice();
    m_ready = IsAudioDeviceReady();
    if (!m_ready) TraceLog(LOG_WARNING, "AudioManager: khong mo duoc thiet bi am thanh -> game chay khong tieng.");
    return m_ready;
}

void AudioManager::loadAll(const std::string& audioDir)
{
    if (!m_ready) return;
    std::string dir = audioDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';

    for (const MusicFile& f : MUSIC_FILES) {
        const std::string path = dir + f.m_file;
        if (!FileExists(path.c_str())) { TraceLog(LOG_WARNING, "AudioManager: thieu '%s'", path.c_str()); continue; }
        Track t;
        t.m_music = LoadMusicStream(path.c_str());
        t.m_loaded = (t.m_music.frameCount > 0);
        if (t.m_loaded) t.m_music.looping = true;
        m_tracks[static_cast<int>(f.m_id)] = t;
    }
    for (const SfxFile& f : SFX_FILES) {
        const std::string path = dir + f.m_file;
        if (!FileExists(path.c_str())) { TraceLog(LOG_WARNING, "AudioManager: thieu '%s'", path.c_str()); continue; }
        Sound s = LoadSound(path.c_str());
        if (s.frameCount == 0) continue;
        m_sounds[static_cast<int>(f.m_id)] = s;
        m_sfxGain[static_cast<int>(f.m_id)] = f.m_gain;
        SetSoundVolume(s, m_sfxVolume * f.m_gain);
    }
}

void AudioManager::unloadAll()
{
    if (!m_ready) return;
    for (auto& kv : m_tracks) {
        if (!kv.second.m_loaded) continue;
        StopMusicStream(kv.second.m_music);
        UnloadMusicStream(kv.second.m_music);
    }
    for (auto& kv : m_sounds) UnloadSound(kv.second);
    m_tracks.clear();
    m_sounds.clear();
    m_sfxGain.clear();
    m_current = m_next = MusicId::NONE;
    CloseAudioDevice();
    m_ready = false;
}

AudioManager::Track* AudioManager::trackOf(MusicId id)
{
    auto it = m_tracks.find(static_cast<int>(id));
    return (it != m_tracks.end() && it->second.m_loaded) ? &it->second : nullptr;
}

void AudioManager::playMusic(MusicId id)
{
    if (!m_ready) return;
    const MusicId target = m_fadingOut ? m_next : m_current;
    if (id == target) return;                       // dang phat / dang cho dung bai nay roi

    if (m_current == MusicId::NONE || !trackOf(m_current)) {   // chua co bai nao -> vao thang
        m_current = id;
        m_next = MusicId::NONE;
        m_fadingOut = false;
        m_fade = 0.0f;
        if (Track* t = trackOf(id)) { StopMusicStream(t->m_music); PlayMusicStream(t->m_music); }
        applyMusicVolume();
        return;
    }
    m_next = id;                                    // tat dan bai cu roi doi
    m_fadingOut = true;
}

void AudioManager::playSfx(SfxId id)
{
    if (!m_ready) return;
    auto it = m_sounds.find(static_cast<int>(id));
    if (it == m_sounds.end()) return;
    PlaySound(it->second);
}

void AudioManager::update(float deltaTime)
{
    if (!m_ready) return;

    if (m_fadingOut) {
        m_fade -= deltaTime / FADE_OUT_SECONDS;
        if (m_fade <= 0.0f) {
            m_fade = 0.0f;
            if (Track* old = trackOf(m_current)) StopMusicStream(old->m_music);
            m_current = m_next;
            m_next = MusicId::NONE;
            m_fadingOut = false;
            if (Track* t = trackOf(m_current)) { StopMusicStream(t->m_music); PlayMusicStream(t->m_music); }
        }
    } else if (m_fade < 1.0f) {
        m_fade = std::min(1.0f, m_fade + deltaTime / FADE_IN_SECONDS);
    }

    applyMusicVolume();
    if (Track* t = trackOf(m_current)) UpdateMusicStream(t->m_music);
}

void AudioManager::applyMusicVolume()
{
    if (Track* t = trackOf(m_current)) SetMusicVolume(t->m_music, clamp01(m_musicVolume * m_fade));
}

void AudioManager::setMusicVolume(float volume) { m_musicVolume = clamp01(volume); }

void AudioManager::setSfxVolume(float volume)
{
    m_sfxVolume = clamp01(volume);
    for (auto& kv : m_sounds) {
        const auto g = m_sfxGain.find(kv.first);
        SetSoundVolume(kv.second, m_sfxVolume * (g != m_sfxGain.end() ? g->second : 1.0f));
    }
}
