#include "UiText.h"
#include "FontManager.h"
#include "UiTheme.h"
#include <algorithm>
#include <cstring>

namespace {
Font roleFont(int size) {
    return FontManager::instance().font(size >= 32 ? FontManager::Role::DISPLAY : FontManager::Role::BODY, size);
}
float letterSpacing(int size) { return size >= 32 ? 1.0f : 0.3f; }
// Removes a single UTF-8 codepoint at the end without splitting a character.
void popCodepoint(std::string& text) {
    if (text.empty()) return;
    text.pop_back();
    while (!text.empty() && (static_cast<unsigned char>(text.back()) & 0xc0) == 0x80) text.pop_back();
}
}
namespace UiText {
void draw(const char* text, int x, int y, int size, Color color) {
    if (!text || size <= 0) return;
    Font font = roleFont(size);
    DrawTextEx(font, text, Vector2{static_cast<float>(x), static_cast<float>(y)}, static_cast<float>(size), letterSpacing(size), color);
}
int measure(const char* text, int size) {
    if (!text || size <= 0) return 0;
    return static_cast<int>(MeasureTextEx(roleFont(size), text, static_cast<float>(size), letterSpacing(size)).x + 0.5f);
}
Vector2 measureEx(const std::string& text, int size) {
    return MeasureTextEx(roleFont(size), text.c_str(), static_cast<float>(size), letterSpacing(size));
}
void drawShadowed(const std::string& text, int x, int y, int size, Color color) {
    draw(text.c_str(), x + 2, y + 3, size, Color{0, 0, 0, static_cast<unsigned char>(color.a * 0.75f)});
    draw(text.c_str(), x, y, size, color);
}
void drawEllipsis(const std::string& text, Rectangle bounds, int size, Color color) {
    std::string visible = text;
    if (measure(visible.c_str(), size) > bounds.width) {
        const std::string ellipsis = "…";
        while (!visible.empty() && measure((visible + ellipsis).c_str(), size) > bounds.width) popCodepoint(visible);
        visible += ellipsis;
    }
    draw(visible.c_str(), static_cast<int>(bounds.x), static_cast<int>(bounds.y), size, color);
}
void drawWrapped(const std::string& text, Rectangle bounds, int size, Color color) {
    const float lineH = size * 1.5f;
    float y = bounds.y;
    std::string line;
    size_t i = 0;
    while (i < text.size() && y + size <= bounds.y + bounds.height) {
        size_t e = text.find_first_of(" \n", i);
        if (e == std::string::npos) e = text.size();
        const std::string word = text.substr(i, e - i);
        const bool newline = e < text.size() && text[e] == '\n';
        std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && measure(candidate.c_str(), size) > bounds.width) {
            draw(line.c_str(), static_cast<int>(bounds.x), static_cast<int>(y), size, color);
            y += lineH;
            line = word;
        } else line = candidate;
        if (newline) {
            draw(line.c_str(), static_cast<int>(bounds.x), static_cast<int>(y), size, color);
            y += lineH;
            line.clear();
        }
        i = e < text.size() ? e + 1 : text.size();
    }
    if (!line.empty() && y + size <= bounds.y + bounds.height) drawEllipsis(line, {bounds.x, y, bounds.width, static_cast<float>(size)}, size, color);
}
void drawFontTest() {
    DrawRectangle(80, 42, 1120, 638, Color{11, 12, 18, 250});
    DrawRectangleLinesEx({80, 42, 1120, 638}, 2, UiTheme::EMBER);
    draw("FONT TEST · VI / EN", 108, 65, 32, UiTheme::AGED_GOLD);
    const char* pangram = "Chú bé quàng khăn đỏ vượt đèo, ngắm ánh trăng và dòng sông.";
    const char* alphabet = "ÀÁẢÃẠ ĂẰẮẲẴẶ ÂẦẤẨẪẬ Đ ÊỀẾỂỄỆ ÔỒỐỔỖỘ ƠỜỚỞỠỢ ƯỪỨỬỮỰ ỲÝỶỸỴ";
    const int sizes[] = {14, 18, 24, 32, 48};
    float y = 120;
    for (int size : sizes) {
        draw(TextFormat("%d px", size), 108, static_cast<int>(y), 14, UiTheme::MUTED);
        drawEllipsis(pangram, {166, y, 994, static_cast<float>(size + 6)}, size, UiTheme::TEXT);
        y += size + 28;
    }
    drawWrapped(alphabet, {108, y + 8, 1040, 110}, 20, UiTheme::TEXT);
    draw("Nhấp góc trái bên trên để đóng", 108, 648, 14, UiTheme::MUTED);
}
}
