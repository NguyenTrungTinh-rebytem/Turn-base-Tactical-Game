#pragma once
// src/data/GameSettings.h
// CAI DAT NGUOI CHOI: am luong nhac/hieu ung, bat tat shader canh, ngon ngu.
// Luu file settings.cfg (key=value) -> lan sau mo game giu nguyen. Thieu/loi file -> dung gia tri mac dinh, KHONG crash.
#include <string>

enum class Language { VI = 0, EN };

class GameSettings {
public:
    static GameSettings& get();                    // 1 ban duy nhat cho ca game

    void load(const std::string& path);            // khong co file -> giu mac dinh
    bool save(const std::string& path) const;      // false = khong ghi duoc (game van chay)

    float    m_musicVolume = 0.45f;                // 0..1
    float    m_sfxVolume = 0.80f;                  // 0..1
    bool     m_shaderEnabled = true;               // shader canh chien dau (F9)
    Language m_language = Language::VI;

    bool isEnglish() const { return m_language == Language::EN; }

private:
    GameSettings() = default;
};
