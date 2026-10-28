#pragma once
// src/graphics/TurnOrderBar.h - Thanh thu tu luot (goc phai tren) co hieu ung chuyen dong.
// Chi ve + animation, khong chua logic game. Thay cho BattleUI::drawTurnOrder.
#include "raylib.h"
#include <string>
#include <vector>

class Character;   // Forward declaration

class TurnOrderBar {
public:
    // Ve va cap nhat animation (dung GetFrameTime()).
    // upcoming = thu tu hien thi, current = nguoi dang di (co the nullptr).
    void draw(const std::vector<Character*>& upcoming, const Character* current);

    // Xoa trang thai. Goi khi vao man moi / van moi, vi con tro Character cu khong con hop le.
    void reset();

private:
    struct Row {
        const Character* m_character = nullptr;   // CHI de so sanh dia chi, khong dereference
        std::string m_name;                       // luu ban sao de hang dang mo dan van ve duoc
        int   m_spd = 0;
        int   m_rank = 0;
        bool  m_present = false;                  // con trong danh sach hien tai?
        bool  m_isCurrent = false;
        float m_targetY = 0.0f;
        float m_y = 0.0f;                         // vi tri dang hien thi (truot muot toi m_targetY)
        float m_x = 0.0f;                         // lech ngang: hang dang di nhich ra ben trai
        float m_alpha = 0.0f;
        float m_active = 0.0f;                    // 0..1: muc "dang den luot" (chuyen mau + vien)
    };

    std::vector<Row> m_rows;
    const Character* m_lastCurrent = nullptr;
    float m_flash = 0.0f;                         // nhay sang khi doi luot
    float m_time = 0.0f;
};