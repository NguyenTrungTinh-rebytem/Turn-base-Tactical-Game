// src/data/ShopConfig.cpp
#include "data/ShopConfig.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

namespace {
std::string trim(const std::string& s)
{
    const char* ws = " \t\r\n";
    const size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    return s.substr(b, s.find_last_not_of(ws) - b + 1);
}
std::string lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
bool parseInt(const std::string& s, int& out)
{
    try {
        size_t pos = 0;
        const int v = std::stoi(s, &pos);
        if (pos != s.size()) return false;
        out = v;
        return true;
    } catch (const std::exception&) { return false; }
}

// Doc file 2 cot "a,b". Tra ve false neu khong mo duoc file.
bool readPairs(const std::string& path, std::vector<std::pair<std::string, std::string>>& out)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) {
            first = false;
            if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF)
                line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        const size_t comma = line.find(',');
        if (comma == std::string::npos) continue;
        out.emplace_back(trim(line.substr(0, comma)), trim(line.substr(comma + 1)));
    }
    return true;
}
} // namespace

bool ShopConfig::load(const std::string& dataDir)
{
    std::string dir = dataDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';
    bool ok = true;

    std::vector<std::pair<std::string, std::string>> rows;
    if (readPairs(dir + "shop.csv", rows)) {
        std::vector<ShopEntry> entries;
        for (const auto& kv : rows) {
            if (lower(kv.first) == "id") continue;   // header
            int price = 0;
            if (kv.first.empty() || !parseInt(kv.second, price) || price < 0) {
                std::cerr << "shop.csv: bo qua dong '" << kv.first << "," << kv.second << "'" << std::endl;
                continue;
            }
            entries.push_back({ kv.first, price });
        }
        m_entries = std::move(entries);
    } else {
        std::cerr << "Khong doc duoc " << dir << "shop.csv" << std::endl;
        ok = false;
    }

    rows.clear();
    if (readPairs(dir + "economy.csv", rows)) {
        for (const auto& kv : rows) {
            const std::string key = lower(kv.first);
            int v = 0;
            if (key == "key" || !parseInt(kv.second, v)) continue;
            if (v < 0) v = 0;
            if (key == "start_gold")             m_economy.m_startGold = v;
            else if (key == "enemy_base_gold")   m_economy.m_enemyBaseGold = v;
            else if (key == "enemy_hp_divisor")  m_economy.m_enemyHpDivisor = v;
            else if (key == "boss_multiplier")   m_economy.m_bossMultiplier = std::max(1, v);
            else if (key == "wave_clear_bonus")  m_economy.m_waveClearBonus = v;
            else if (key == "variance_percent")  m_economy.m_variancePercent = std::min(v, 90);
            else if (key == "skip_wave_gold")    m_economy.m_skipWaveGold = v;
        }
    } else {
        std::cerr << "Khong doc duoc " << dir << "economy.csv (dung gia tri mac dinh)" << std::endl;
        ok = false;
    }
    return ok;
}

bool ShopConfig::isSold(const std::string& itemId) const
{
    return std::any_of(m_entries.begin(), m_entries.end(), [&](const ShopEntry& e) { return e.m_id == itemId; });
}

int ShopConfig::getPrice(const std::string& itemId) const
{
    for (const ShopEntry& e : m_entries)
        if (e.m_id == itemId) return e.m_price;
    return 0;
}
