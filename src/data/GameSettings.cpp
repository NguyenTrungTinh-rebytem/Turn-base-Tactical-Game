#include "data/GameSettings.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace {
float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
}

GameSettings& GameSettings::get() {
    static GameSettings instance;
    return instance;
}

void GameSettings::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        if (key == "music")       m_musicVolume = clamp01(static_cast<float>(std::atof(val.c_str())));
        else if (key == "sfx")    m_sfxVolume = clamp01(static_cast<float>(std::atof(val.c_str())));
        else if (key == "shader") m_shaderEnabled = (std::atoi(val.c_str()) != 0);
        else if (key == "lang")   m_language = (val.rfind("en", 0) == 0) ? Language::EN : Language::VI;
    }
}

bool GameSettings::save(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << "music=" << m_musicVolume << "\n"
        << "sfx=" << m_sfxVolume << "\n"
        << "shader=" << (m_shaderEnabled ? 1 : 0) << "\n"
        << "lang=" << (m_language == Language::EN ? "en" : "vi") << "\n";
    return static_cast<bool>(out);
}
