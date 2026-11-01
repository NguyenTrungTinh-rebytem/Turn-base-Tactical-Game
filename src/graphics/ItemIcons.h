#pragma once
// src/graphics/ItemIcons.h
// Kho icon vat pham: assets/images/items/<id>.png (id = id trong items.csv). Thieu anh -> ve o mau thay the.
#include "raylib.h"
#include <string>
#include <unordered_map>

class ItemIcons {
public:
    ItemIcons() = default;
    ~ItemIcons();
    ItemIcons(const ItemIcons&) = delete;
    ItemIcons& operator=(const ItemIcons&) = delete;

    // category: 0 = tieu hao, 1 = booster, 2 = trang bi (chi de chon mau o thay the)
    void load(const std::string& itemId, const std::string& path, int category);
    void unloadAll();                                 // goi truoc CloseWindow()
    bool has(const std::string& itemId) const;

    // Ve vao dest (giu ti le, can giua). Khong co anh -> ve o mau + chu cai dau.
    void draw(const std::string& itemId, Rectangle dest, Color tint = WHITE) const;

private:
    struct Icon { Texture2D m_texture{}; bool m_loaded = false; int m_category = 0; };
    std::unordered_map<std::string, Icon> m_icons;
};
