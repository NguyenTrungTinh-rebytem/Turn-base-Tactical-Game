#pragma once
// src/graphics/CursorSkin.h
// CON TRO CHUOT TUY CHINH (bo "Dragon's Jewel": ngoc rong) - ve thay con tro he dieu hanh.
// Anh: assets/images/cursor/cursor_<normal|link|busy|unavailable|text>.png = dai khung 32x32 xep ngang (chuyen tu .ani).
// Thieu anh -> initialize() tra false, game dung UiCursor cu nhu truoc (KHONG crash).
#include <string>

namespace CursorSkin {
    enum class Kind { NORMAL = 0, LINK, BUSY, UNAVAILABLE, TEXT };

    bool initialize(const std::string& directory);   // sau InitWindow
    void shutdown();                                 // TRUOC CloseWindow
    void setHover(bool overClickable);               // UI goi khi chuot dang tren nut -> con tro "link" (het hieu luc sau 1 khung hinh)
    void setOverride(Kind kind, bool active);        // ep kieu con tro (vd BUSY khi dang tai); active=false de bo ep
    bool draw();                                     // ve tai vi tri chuot (toa do canvas). false = chua nap duoc -> dung con tro cu
}
