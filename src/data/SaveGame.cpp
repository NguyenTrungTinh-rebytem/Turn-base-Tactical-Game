#include "data/SaveGame.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

namespace {
std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> parts;
    std::string cur;
    std::istringstream in(s);
    while (std::getline(in, cur, sep)) parts.push_back(cur);
    if (!s.empty() && s.back() == sep) parts.emplace_back();
    return parts;
}
std::string nowText() {
    const std::time_t t = std::time(nullptr);
    const std::tm* lt = std::localtime(&t);
    char buf[32] = {};
    if (lt) std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", lt);
    return buf;
}
}

bool SaveGame::exists(const std::string& path) {
    std::ifstream in(path);
    return static_cast<bool>(in);
}

bool SaveGame::save(const std::string& path, SaveData data) {
    data.m_savedAt = nowText();
    const std::string tmp = path + ".tmp";          // ghi file tam roi doi ten -> crash giua chung khong lam hong save cu
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) return false;
        out << "version=" << data.m_version << "\n"
            << "savedAt=" << data.m_savedAt << "\n"
            << "nextWave=" << data.m_nextWave << "\n"
            << "gold=" << data.m_gold << "\n";
        for (const SaveHero& h : data.m_heroes)
            out << "hero=" << h.m_name << "|" << h.m_race << "|" << h.m_class << "|"
                << h.m_element << "|" << h.m_hp << "|" << h.m_mp << "\n";
        for (const auto& b : data.m_bag) out << "bag=" << b.first << "|" << b.second << "\n";
        for (int i = 0; i < 3; ++i)
            if (!data.m_gear[i].empty()) out << "gear=" << i << "|" << data.m_gear[i] << "\n";
        out << "end=1\n";                            // dau hieu file ghi tron ven
        if (!out) return false;
    }
    std::remove(path.c_str());
    return std::rename(tmp.c_str(), path.c_str()) == 0;
}

bool SaveGame::load(const std::string& path, SaveData& outData) {
    std::ifstream in(path);
    if (!in) return false;
    SaveData d;
    bool complete = false;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq), val = line.substr(eq + 1);
        if (key == "version") d.m_version = std::atoi(val.c_str());
        else if (key == "savedAt") d.m_savedAt = val;
        else if (key == "nextWave") d.m_nextWave = std::atoi(val.c_str());
        else if (key == "gold") d.m_gold = std::atoi(val.c_str());
        else if (key == "hero") {
            const auto p = split(val, '|');
            if (p.size() < 6 || d.m_heroes.size() >= 3) continue;
            SaveHero h;
            h.m_name = p[0]; h.m_race = p[1]; h.m_class = p[2];
            h.m_element = std::atoi(p[3].c_str());
            h.m_hp = std::atoi(p[4].c_str());
            h.m_mp = std::atoi(p[5].c_str());
            d.m_heroes.push_back(h);
        } else if (key == "bag") {
            const auto p = split(val, '|');
            if (p.size() >= 2) d.m_bag.emplace_back(p[0], std::atoi(p[1].c_str()));
        } else if (key == "gear") {
            const auto p = split(val, '|');
            const int idx = p.size() >= 2 ? std::atoi(p[0].c_str()) : -1;
            if (idx >= 0 && idx < 3) d.m_gear[idx] = p[1];
        } else if (key == "end") complete = true;
    }
    if (!complete || d.m_heroes.empty() || d.m_nextWave < 1) return false;   // thieu "end" = file ghi do / hong
    outData = d;
    return true;
}

bool SaveGame::remove(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}
