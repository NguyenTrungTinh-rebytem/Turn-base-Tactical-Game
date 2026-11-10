#pragma once
// src/graphics/HeroPortraits.h
// Anh chan dung tuong (assets/images/portraits/portrait_<id>.png) cho man hinh chon doi hinh.
// Thieu anh -> draw() tra ve false de man hinh tu ve sprite nho nhu cu.
#include "raylib.h"
#include <string>
#include <unordered_map>

class HeroPortraits {
public:
    HeroPortraits() = default;
    ~HeroPortraits() { unloadAll(); }
    HeroPortraits(const HeroPortraits&) = delete;
    HeroPortraits& operator=(const HeroPortraits&) = delete;

    // Tai anh theo id tuong (viet thuong: "lyra", "kael"...). Tra ve false neu khong co file.
    bool load(const std::string& heroId, const std::string& path);
    bool has(const std::string& heroId) const { return m_textures.count(heroId) > 0; }
    void unloadAll();

    // Ve anh phu kin khung `dest` (cat giua, giu ti le), co vien + toi dan o day.
    // Tra ve false neu khong co anh cho heroId.
    bool draw(const std::string& heroId, Rectangle dest, Color tint = WHITE) const;

private:
    std::unordered_map<std::string, Texture2D> m_textures;
};
