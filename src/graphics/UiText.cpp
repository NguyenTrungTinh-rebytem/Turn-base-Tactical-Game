#include "UiText.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <utility>
#include <vector>

namespace {
std::string stripBom(const char* text) {
    if (!text) return {};
    std::string result(text);
    if (result.size() >= 3 &&
        static_cast<unsigned char>(result[0]) == 0xEF &&
        static_cast<unsigned char>(result[1]) == 0xBB &&
        static_cast<unsigned char>(result[2]) == 0xBF)
        result.erase(0, 3);
    return result;
}

bool numericText(const std::string& text) {
    bool hasDigit = false;
    for (const unsigned char ch : text) {
        if (ch >= '0' && ch <= '9') {
            hasDigit = true;
        } else if (ch != ' ' && ch != ',' && ch != '.' && ch != '/' &&
                   ch != '%' && ch != '+' && ch != '-' && ch != ':' &&
                   ch != 'x' && ch != 'X') {
            return false;
        }
    }
    return hasDigit;
}

UiFontRole resolveRole(UiFontRole role, const std::string& text, int fontSize) {
    if (role != UiFontRole::AUTO) return role;
    if (numericText(text)) return UiFontRole::NUMERALS;
    if (fontSize >= 32) return UiFontRole::DISPLAY;
    if (fontSize >= 20) return UiFontRole::HUD;
    return UiFontRole::BODY;
}

int decodeUtf8(const std::string& text, std::size_t& offset) {
    const unsigned char first = static_cast<unsigned char>(text[offset++]);
    if (first < 0x80) return first;
    int codepoint = 0;
    int continuationCount = 0;
    if ((first & 0xE0) == 0xC0) {
        codepoint = first & 0x1F;
        continuationCount = 1;
    } else if ((first & 0xF0) == 0xE0) {
        codepoint = first & 0x0F;
        continuationCount = 2;
    } else if ((first & 0xF8) == 0xF0) {
        codepoint = first & 0x07;
        continuationCount = 3;
    } else {
        return '?';
    }

    for (int i = 0; i < continuationCount && offset < text.size(); ++i) {
        const unsigned char next = static_cast<unsigned char>(text[offset]);
        if ((next & 0xC0) != 0x80) return '?';
        ++offset;
        codepoint = (codepoint << 6) | (next & 0x3F);
    }
    return codepoint;
}

std::string encodeUtf8(int codepoint) {
    std::string encoded;
    if (codepoint <= 0x7F) {
        encoded.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        encoded.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        encoded.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        encoded.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        encoded.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        encoded.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        encoded.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        encoded.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        encoded.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        encoded.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    return encoded;
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    if (!text.empty() && text.back() == '\n') lines.emplace_back();
    if (lines.empty()) lines.emplace_back();
    return lines;
}

float glyphAdvance(const Font& font, int codepoint, float fontSize) {
    const int glyphIndex = GetGlyphIndex(font, codepoint);
    if (glyphIndex < 0 || glyphIndex >= font.glyphCount) return fontSize * 0.5f;
    const GlyphInfo& glyph = font.glyphs[glyphIndex];
    const Rectangle& rec = font.recs[glyphIndex];
    const int advance = glyph.advanceX > 0 ? glyph.advanceX : static_cast<int>(rec.width + glyph.offsetX);
    return static_cast<float>(advance) * fontSize / static_cast<float>(std::max(1, font.baseSize));
}

float tabularDigitAdvance(const Font& font, float fontSize) {
    float advance = 0.0f;
    for (int digit = '0'; digit <= '9'; ++digit)
        advance = std::max(advance, glyphAdvance(font, digit, fontSize));
    return advance;
}

float measureLine(const std::string& text, int fontSize, UiFontRole role, float letterSpacing) {
    const Font& font = FontManager::instance().font(role, fontSize);
    if (role != UiFontRole::NUMERALS)
        return MeasureTextEx(font, text.c_str(), static_cast<float>(fontSize), letterSpacing).x;

    const float digitWidth = tabularDigitAdvance(font, static_cast<float>(fontSize));
    float width = 0.0f;
    std::size_t offset = 0;
    while (offset < text.size()) {
        const int codepoint = decodeUtf8(text, offset);
        const float advance = (codepoint >= '0' && codepoint <= '9')
            ? digitWidth : glyphAdvance(font, codepoint, static_cast<float>(fontSize));
        width += advance + letterSpacing;
    }
    return width;
}

void drawLine(const std::string& text, int x, int y, int fontSize, Color tint,
              UiFontRole role, float letterSpacing, bool shadow = true) {
    if (text.empty()) return;
    const Font& font = FontManager::instance().font(role, fontSize);
    if (role != UiFontRole::NUMERALS) {
        if (shadow)
            DrawTextEx(font, text.c_str(), Vector2{ static_cast<float>(x + 1), static_cast<float>(y + 1) },
                       static_cast<float>(fontSize), letterSpacing, Color{ 8, 10, 17, 190 });
        DrawTextEx(font, text.c_str(), Vector2{ static_cast<float>(x), static_cast<float>(y) },
                   static_cast<float>(fontSize), letterSpacing, tint);
        return;
    }

    const float digitWidth = tabularDigitAdvance(font, static_cast<float>(fontSize));
    float cursorX = static_cast<float>(x);
    std::size_t offset = 0;
    while (offset < text.size()) {
        const int codepoint = decodeUtf8(text, offset);
        const float advance = (codepoint >= '0' && codepoint <= '9')
            ? digitWidth : glyphAdvance(font, codepoint, static_cast<float>(fontSize));
        if (shadow)
            DrawTextCodepoint(font, codepoint, Vector2{ cursorX + 1.0f, static_cast<float>(y) + 1.0f },
                              static_cast<float>(fontSize), Color{ 8, 10, 17, 190 });
        DrawTextCodepoint(font, codepoint, Vector2{ cursorX, static_cast<float>(y) },
                          static_cast<float>(fontSize), tint);
        cursorX += advance + letterSpacing;
    }
}

std::vector<std::string> utf8Characters(const std::string& text) {
    std::vector<std::string> characters;
    std::size_t offset = 0;
    while (offset < text.size()) {
        const std::size_t start = offset;
        decodeUtf8(text, offset);
        characters.push_back(text.substr(start, offset - start));
    }
    return characters;
}
}

void UiText::draw(const char* text, int x, int y, int fontSize, Color tint,
                  UiFontRole role, float letterSpacing) {
    if (!text || fontSize <= 0) return;
    const std::string cleaned = stripBom(text);
    const UiFontRole resolved = resolveRole(role, cleaned, fontSize);
    const int lineHeight = std::max(fontSize, static_cast<int>(std::ceil(fontSize * 1.35f)));
    const std::vector<std::string> lines = splitLines(cleaned);
    for (std::size_t index = 0; index < lines.size(); ++index)
        drawLine(lines[index], x, y + static_cast<int>(index) * lineHeight,
                 fontSize, tint, resolved, letterSpacing);
}

void UiText::draw(const std::string& text, int x, int y, int fontSize, Color tint,
                  UiFontRole role, float letterSpacing) {
    draw(text.c_str(), x, y, fontSize, tint, role, letterSpacing);
}

void UiText::drawShadowed(const std::string& text, int x, int y, int fontSize, Color tint,
                          UiFontRole role, float letterSpacing) {
    draw(text, x, y, fontSize, tint, role, letterSpacing);
}

void UiText::drawOutlined(const std::string& text, int x, int y, int fontSize, Color tint,
                          Color outline, int thickness, UiFontRole role) {
    if (text.empty() || fontSize <= 0) return;
    const UiFontRole resolved = resolveRole(role, text, fontSize);
    thickness = std::max(1, thickness);
    for (int offset = 1; offset <= thickness; ++offset) {
        drawLine(text, x - offset, y, fontSize, outline, resolved, 0.0f, false);
        drawLine(text, x + offset, y, fontSize, outline, resolved, 0.0f, false);
        drawLine(text, x, y - offset, fontSize, outline, resolved, 0.0f, false);
        drawLine(text, x, y + offset, fontSize, outline, resolved, 0.0f, false);
    }
    draw(text, x, y, fontSize, tint, resolved);
}

int UiText::measure(const char* text, int fontSize, UiFontRole role, float letterSpacing) {
    if (!text || fontSize <= 0) return 0;
    const std::string cleaned = stripBom(text);
    const UiFontRole resolved = resolveRole(role, cleaned, fontSize);
    float maxWidth = 0.0f;
    for (const std::string& line : splitLines(cleaned))
        maxWidth = std::max(maxWidth, measureLine(line, fontSize, resolved, letterSpacing));
    return static_cast<int>(std::ceil(maxWidth));
}

int UiText::measure(const std::string& text, int fontSize, UiFontRole role, float letterSpacing) {
    return measure(text.c_str(), fontSize, role, letterSpacing);
}

int UiText::drawWrapped(const std::string& text, int x, int y, int maxWidth, int fontSize,
                        Color tint, int lineHeight, UiFontRole role) {
    if (maxWidth <= 0 || fontSize <= 0) return 0;
    const std::string cleaned = stripBom(text.c_str());
    const UiFontRole resolved = resolveRole(role, cleaned, fontSize);
    if (lineHeight <= 0) lineHeight = static_cast<int>(std::ceil(fontSize * 1.4f));

    const std::vector<std::string> words = [&cleaned]() {
        std::vector<std::string> result;
        std::string word;
        for (const char ch : cleaned) {
            if (ch == ' ' || ch == '\n' || ch == '\r') {
                if (!word.empty()) result.push_back(std::move(word));
                result.emplace_back();
                word.clear();
            } else {
                word.push_back(ch);
            }
        }
        if (!word.empty()) result.push_back(std::move(word));
        return result;
    }();

    std::vector<std::string> lines;
    std::string current;
    for (const std::string& word : words) {
        if (word.empty()) {
            lines.push_back(current);
            current.clear();
            continue;
        }
        const std::string candidate = current.empty() ? word : current + " " + word;
        if (measureLine(candidate, fontSize, resolved, 0.0f) <= maxWidth) {
            current = candidate;
            continue;
        }
        if (!current.empty()) {
            lines.push_back(current);
            current.clear();
        }
        if (measureLine(word, fontSize, resolved, 0.0f) <= maxWidth) {
            current = word;
            continue;
        }
        for (const std::string& character : utf8Characters(word)) {
            const std::string partial = current + character;
            if (!current.empty() && measureLine(partial, fontSize, resolved, 0.0f) > maxWidth) {
                lines.push_back(current);
                current = character;
            } else {
                current = partial;
            }
        }
    }
    if (!current.empty() || lines.empty()) lines.push_back(current);

    for (std::size_t index = 0; index < lines.size(); ++index)
        drawLine(lines[index], x, y + static_cast<int>(index) * lineHeight,
                 fontSize, tint, resolved, 0.0f);
    return static_cast<int>(lines.size()) * lineHeight;
}

std::string UiText::ellipsis(const std::string& text, int maxWidth, int fontSize, UiFontRole role) {
    const std::string cleaned = stripBom(text.c_str());
    if (measure(cleaned, fontSize, role) <= maxWidth) return cleaned;
    const std::string mark = u8"…";
    if (measure(mark, fontSize, role) > maxWidth) return {};

    std::vector<std::string> characters = utf8Characters(cleaned);
    std::string result = cleaned;
    while (!characters.empty()) {
        characters.pop_back();
        result.clear();
        for (const std::string& character : characters) result += character;
        result += mark;
        if (measure(result, fontSize, role) <= maxWidth) return result;
    }
    return mark;
}

void UiText::drawEllipsis(const std::string& text, int x, int y, int maxWidth, int fontSize,
                          Color tint, UiFontRole role) {
    draw(ellipsis(text, maxWidth, fontSize, role), x, y, fontSize, tint, role);
}
