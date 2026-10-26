// Member 4 - Huynh Quang Khai | src/graphics/HeroCreateScreen.cpp
#include "HeroCreateScreen.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float LEFT_X = 60.0f, ROW_W = 560.0f, ROW_Y0 = 130.0f, ROW_PITCH = 92.0f;
constexpr float LABEL_H = 22.0f, BOX_H = 44.0f, ARROW_W = 48.0f;
constexpr float PANEL_X = 660.0f, PANEL_Y = 110.0f, PANEL_W = 560.0f, PANEL_H = 500.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr int   MAX_SKILL_LINES = 5;
constexpr float BACKSPACE_DELAY = 0.4f, BACKSPACE_RATE = 0.05f;

// ---- Mau ----
constexpr Color COL_TITLE      { 255, 214, 96, 255 };
constexpr Color COL_PANEL      { 16, 13, 36, 215 };
constexpr Color COL_EDGE       { 140, 120, 180, 255 };
constexpr Color COL_EDGE_FOCUS { 255, 214, 96, 255 };
constexpr Color COL_LABEL      { 190, 185, 220, 255 };
constexpr Color COL_TEXT       { 238, 235, 252, 255 };
constexpr Color COL_DIM        { 130, 126, 160, 255 };
constexpr Color COL_BAR_BG     { 34, 30, 60, 255 };
constexpr Color COL_ERR        { 255, 110, 110, 255 };

const char* const ROW_LABELS[] = { "TEN ANH HUNG", "CHUNG TOC", "LOP NHAN VAT", "HE (NGU HANH)" };
const char* const ELEMENT_NAMES[] = { "Kim", "Moc", "Thuy", "Hoa", "Tho" };
constexpr size_t ELEMENT_COUNT = 5;

Color elementColor(size_t i) {
    switch (i) {
        case 0: return Color{ 215, 215, 230, 255 };   // Kim
        case 1: return Color{ 100, 210, 100, 255 };   // Moc
        case 2: return Color{ 90, 160, 255, 255 };    // Thuy
        case 3: return Color{ 255, 110, 80, 255 };    // Hoa
        default: return Color{ 205, 160, 90, 255 };   // Tho
    }
}

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

void drawCenteredText(const char* text, float cx, float y, int size, Color c) {
    DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
}

std::string trimmed(const std::string& s) {
    const size_t a = s.find_first_not_of(' ');
    if (a == std::string::npos) return "";
    const size_t b = s.find_last_not_of(' ');
    return s.substr(a, b - a + 1);
}

const char* statsLine(const Stats& s) {
    return TextFormat("HP %d  MP %d  ATK %d  MATK %d  DEF %d  MDEF %d  SPD %d",
                      s.m_hp, s.m_mp, s.m_a, s.m_ma, s.m_d, s.m_md, s.m_spd);
}
} // namespace

HeroCreateScreen::HeroCreateScreen() { m_sprite.play(AnimState::IDLE); }
HeroCreateScreen::~HeroCreateScreen() { unloadAll(); }

// ============ Tai nguyen ============
bool HeroCreateScreen::loadBackground(const std::string& path) {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
    if (path.empty() || !FileExists(path.c_str())) return false;
    m_background = LoadTexture(path.c_str());
    m_hasBackground = (m_background.id != 0);
    if (m_hasBackground) SetTextureFilter(m_background, TEXTURE_FILTER_POINT);
    return m_hasBackground;
}

bool HeroCreateScreen::loadPreviewSprite(const std::string& path, int frameWidth, int frameHeight) {
    const bool ok = m_sprite.load(path, frameWidth, frameHeight);   // that bai => placeholder
    m_sprite.play(AnimState::IDLE);
    return ok;
}

void HeroCreateScreen::unloadAll() {
    m_sprite.unload();
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

// ============ Du lieu ============
void HeroCreateScreen::setOptions(const std::vector<HeroOption>& races, const std::vector<HeroOption>& classes) {
    m_races = races;
    m_classes = classes;

    // Gia tri toi da cua moi chi so = max Race + max Class (de ve thanh chi so cho hop ly)
    auto maxOf = [](const std::vector<HeroOption>& v, int Stats::* field) {
        int m = 0;
        for (const auto& o : v) m = std::max(m, o.m_stats.*field);
        return m;
    };
    auto cap = [&](int Stats::* f) { return std::max(1, maxOf(m_races, f) + maxOf(m_classes, f)); };
    m_statMax = Stats{ cap(&Stats::m_hp), cap(&Stats::m_mp), cap(&Stats::m_a), cap(&Stats::m_ma),
                       cap(&Stats::m_d), cap(&Stats::m_md), cap(&Stats::m_spd) };

    m_raceIdx = std::min(m_raceIdx, m_races.empty() ? size_t{ 0 } : m_races.size() - 1);
    m_classIdx = std::min(m_classIdx, m_classes.empty() ? size_t{ 0 } : m_classes.size() - 1);
}

void HeroCreateScreen::reset(const std::string& defaultName) {
    m_name = defaultName.substr(0, MAX_NAME_LENGTH);
    m_raceIdx = m_classIdx = m_elementIdx = 0;
    m_focus = ROW_NAME;
    m_hoverButton = -1;
    m_message.clear();
    m_messageTimer = 0.0f;
    m_backspaceHold = m_backspaceRepeat = 0.0f;
    // Xoa ky tu con ton trong hang doi (vd phim vua bam o man hinh truoc)
    while (GetCharPressed() > 0) {}
}

HeroSelection HeroCreateScreen::getSelection() const {
    HeroSelection s;
    s.m_name = trimmed(m_name);
    s.m_raceIndex = m_raceIdx;
    s.m_classIndex = m_classIdx;
    s.m_element = static_cast<Element>(m_elementIdx);
    return s;
}

Stats HeroCreateScreen::totalStats() const {
    if (!hasOptions()) return Stats{};
    return m_races[m_raceIdx].m_stats + m_classes[m_classIdx].m_stats;   // Base = Race + Class
}

// ============ Bo cuc ============
Rectangle HeroCreateScreen::rowRect(int row) const {
    return { LEFT_X, ROW_Y0 + row * ROW_PITCH + LABEL_H, ROW_W, BOX_H };
}
Rectangle HeroCreateScreen::leftArrowRect(int row) const {
    const Rectangle r = rowRect(row);
    return { r.x, r.y, ARROW_W, r.height };
}
Rectangle HeroCreateScreen::rightArrowRect(int row) const {
    const Rectangle r = rowRect(row);
    return { r.x + r.width - ARROW_W, r.y, ARROW_W, r.height };
}
Rectangle HeroCreateScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(GetScreenWidth()) - total) / 2.0f;
    return { startX + index * (BTN_W + BTN_GAP), BTN_Y, BTN_W, BTN_H };
}

int HeroCreateScreen::optionCount(int row) const {
    switch (row) {
        case ROW_RACE:    return static_cast<int>(m_races.size());
        case ROW_CLASS:   return static_cast<int>(m_classes.size());
        case ROW_ELEMENT: return static_cast<int>(ELEMENT_COUNT);
        default:          return 0;
    }
}

void HeroCreateScreen::changeValue(int row, int delta) {
    const int n = optionCount(row);
    if (n <= 0) return;
    size_t* target = (row == ROW_RACE) ? &m_raceIdx : (row == ROW_CLASS) ? &m_classIdx : &m_elementIdx;
    const int next = (static_cast<int>(*target) + delta % n + n) % n;   // xoay vong
    *target = static_cast<size_t>(next);
}

void HeroCreateScreen::randomize() {
    if (!hasOptions()) return;
    m_raceIdx = static_cast<size_t>(GetRandomValue(0, static_cast<int>(m_races.size()) - 1));
    m_classIdx = static_cast<size_t>(GetRandomValue(0, static_cast<int>(m_classes.size()) - 1));
    m_elementIdx = static_cast<size_t>(GetRandomValue(0, static_cast<int>(ELEMENT_COUNT) - 1));
}

// ============ Input ============
void HeroCreateScreen::handleTextInput() {
    // Luon rut het ky tu cho (de khong bi dong ky tu cu khi doi o)
    std::string typed;
    for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
        if (ch >= 32 && ch <= 125) typed.push_back(static_cast<char>(ch));   // font mac dinh chi co ASCII

    if (m_focus != ROW_NAME) { m_backspaceHold = m_backspaceRepeat = 0.0f; return; }

    for (char c : typed)
        if (m_name.size() < MAX_NAME_LENGTH) m_name.push_back(c);

    // Backspace: bam 1 lan xoa 1 ky tu; giu lau thi xoa lien tuc
    auto eraseLast = [&]() { if (!m_name.empty()) m_name.pop_back(); };
    if (IsKeyPressed(KEY_BACKSPACE)) {
        eraseLast();
        m_backspaceHold = m_backspaceRepeat = 0.0f;
    } else if (IsKeyDown(KEY_BACKSPACE)) {
        m_backspaceHold += GetFrameTime();
        if (m_backspaceHold > BACKSPACE_DELAY) {
            m_backspaceRepeat += GetFrameTime();
            while (m_backspaceRepeat >= BACKSPACE_RATE) { eraseLast(); m_backspaceRepeat -= BACKSPACE_RATE; }
        }
    } else {
        m_backspaceHold = m_backspaceRepeat = 0.0f;
    }
}

HeroCreateAction HeroCreateScreen::tryConfirm() {
    if (trimmed(m_name).empty()) {
        m_message = "Hay nhap ten anh hung!";
        m_messageTimer = 2.0f;
        m_focus = ROW_NAME;
        return HeroCreateAction::NONE;
    }
    return HeroCreateAction::CONFIRM;
}

HeroCreateAction HeroCreateScreen::handleInput() {
    handleTextInput();
    if (!hasOptions()) return HeroCreateAction::NONE;

    // ---- Ban phim ----
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_DOWN)) m_focus = (m_focus + 1) % ROW_COUNT;
    if (IsKeyPressed(KEY_UP)) m_focus = (m_focus + ROW_COUNT - 1) % ROW_COUNT;
    if (m_focus != ROW_NAME) {
        if (IsKeyPressed(KEY_LEFT))  changeValue(m_focus, -1);
        if (IsKeyPressed(KEY_RIGHT)) changeValue(m_focus, +1);
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        const HeroCreateAction a = tryConfirm();
        if (a != HeroCreateAction::NONE) return a;
    }

    // ---- Chuot ----
    const Vector2 mouse = GetMousePosition();
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int row = 0; row < ROW_COUNT; ++row) {
            if (!CheckCollisionPointRec(mouse, rowRect(row))) continue;
            m_focus = row;
            if (row != ROW_NAME) {
                if (CheckCollisionPointRec(mouse, leftArrowRect(row)))  changeValue(row, -1);
                if (CheckCollisionPointRec(mouse, rightArrowRect(row))) changeValue(row, +1);
            }
        }
        switch (m_hoverButton) {
            case BTN_BACK:    return HeroCreateAction::BACK;
            case BTN_RANDOM:  randomize(); break;
            case BTN_CONFIRM: { const HeroCreateAction a = tryConfirm(); if (a != HeroCreateAction::NONE) return a; break; }
            default: break;
        }
    }
    return HeroCreateAction::NONE;
}

void HeroCreateScreen::update(float deltaTime) {
    m_time += deltaTime;
    m_sprite.update(deltaTime);
    if (m_messageTimer > 0.0f) m_messageTimer = std::max(0.0f, m_messageTimer - deltaTime);
}

// ============ Ve ============
void HeroCreateScreen::draw() const {
    drawBackground();
    drawCenteredText("TAO ANH HUNG", GetScreenWidth() / 2.0f, 34.0f, 44, COL_TITLE);

    if (!hasOptions()) {
        drawCenteredText("Khong co du lieu chung toc / lop (kiem tra thu muc data/)", GetScreenWidth() / 2.0f, 320.0f, 22, COL_ERR);
        drawButtons();
        return;
    }
    drawSelectors();
    drawPreview();
    drawButtons();

    if (m_messageTimer > 0.0f)
        drawCenteredText(m_message.c_str(), GetScreenWidth() / 2.0f, 614.0f, 20, COL_ERR);
    drawCenteredText("[TAB/UP/DOWN] Chon muc   [LEFT/RIGHT] Doi gia tri   [ENTER] Xac nhan   [ESC] Quay lai",
                     GetScreenWidth() / 2.0f, 702.0f, 16, Color{ 200, 195, 225, 220 });
}

void HeroCreateScreen::drawBackground() const {
    const int W = GetScreenWidth(), H = GetScreenHeight();
    if (m_hasBackground) {
        DrawTexturePro(m_background, Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, static_cast<float>(W), static_cast<float>(H) }, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 10, 8, 36, 255 }, Color{ 60, 30, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 150 });   // lam toi nen de chu de doc
}

void HeroCreateScreen::drawSelectors() const {
    for (int row = 0; row < ROW_COUNT; ++row) {
        const bool focused = (row == m_focus);
        const Rectangle box = rowRect(row);

        DrawText(ROW_LABELS[row], static_cast<int>(box.x), static_cast<int>(box.y - LABEL_H + 2), 16, focused ? COL_TITLE : COL_LABEL);
        DrawRectangleRec(box, focused ? Color{ 60, 44, 20, 230 } : COL_PANEL);
        DrawRectangleLinesEx(box, focused ? 3.0f : 2.0f, focused ? COL_EDGE_FOCUS : COL_EDGE);

        const float textY = box.y + (box.height - 24.0f) / 2.0f;
        const float cx = box.x + box.width / 2.0f;

        if (row == ROW_NAME) {
            const bool blink = focused && (static_cast<int>(m_time * 2.0f) % 2 == 0);
            if (m_name.empty() && !focused) {
                drawCenteredText("(nhap ten)", cx, textY, 24, COL_DIM);
            } else {
                const std::string shown = m_name + (blink ? "_" : "");
                drawCenteredText(shown.c_str(), cx, textY, 24, COL_TEXT);
            }
            continue;
        }

        // Mui ten trai / phai
        const Color arrowColor = focused ? COL_EDGE_FOCUS : COL_EDGE;
        const Rectangle l = leftArrowRect(row), r = rightArrowRect(row);
        DrawTriangle(Vector2{ l.x + 30, l.y + 10 }, Vector2{ l.x + 14, l.y + l.height / 2 }, Vector2{ l.x + 30, l.y + l.height - 10 }, arrowColor);
        DrawTriangle(Vector2{ r.x + 18, r.y + 10 }, Vector2{ r.x + 18, r.y + r.height - 10 }, Vector2{ r.x + 34, r.y + r.height / 2 }, arrowColor);

        const char* value = "";
        Color valueColor = COL_TEXT;
        size_t index = 0, count = 0;
        if (row == ROW_RACE)         { value = m_races[m_raceIdx].m_name.c_str();   index = m_raceIdx;   count = m_races.size(); }
        else if (row == ROW_CLASS)   { value = m_classes[m_classIdx].m_name.c_str(); index = m_classIdx;  count = m_classes.size(); }
        else                         { value = ELEMENT_NAMES[m_elementIdx]; valueColor = elementColor(m_elementIdx); index = m_elementIdx; count = ELEMENT_COUNT; }

        drawCenteredText(value, cx, textY, 24, valueColor);
        const char* counter = TextFormat("%d/%d", static_cast<int>(index) + 1, static_cast<int>(count));
        DrawText(counter, static_cast<int>(r.x - 8 - MeasureText(counter, 14)), static_cast<int>(box.y + 15), 14, COL_DIM);
    }

    // Chi so goc cua Chung toc va Lop dang chon (de thay Tong = Chung toc + Lop)
    const float y = ROW_Y0 + ROW_COUNT * ROW_PITCH + 6.0f;
    DrawText(TextFormat("Chung toc %s:", m_races[m_raceIdx].m_name.c_str()), static_cast<int>(LEFT_X), static_cast<int>(y), 15, COL_LABEL);
    DrawText(statsLine(m_races[m_raceIdx].m_stats), static_cast<int>(LEFT_X), static_cast<int>(y + 18), 15, COL_TEXT);
    DrawText(TextFormat("Lop %s:", m_classes[m_classIdx].m_name.c_str()), static_cast<int>(LEFT_X), static_cast<int>(y + 44), 15, COL_LABEL);
    DrawText(statsLine(m_classes[m_classIdx].m_stats), static_cast<int>(LEFT_X), static_cast<int>(y + 62), 15, COL_TEXT);
}

void HeroCreateScreen::drawPreview() const {
    const Rectangle panel = { PANEL_X, PANEL_Y, PANEL_W, PANEL_H };
    DrawRectangleRec(panel, COL_PANEL);
    DrawRectangleLinesEx(panel, 2.0f, COL_EDGE);

    // ---- Sprite xem truoc ----
    const float sprScale = 3.5f;
    const float sprSize = 64.0f * sprScale;
    const Vector2 sprPos = { PANEL_X + 20.0f, PANEL_Y + 25.0f };
    DrawRectangleRec(Rectangle{ sprPos.x, sprPos.y, sprSize, sprSize }, Color{ 28, 24, 56, 255 });
    DrawRectangleLinesEx(Rectangle{ sprPos.x, sprPos.y, sprSize, sprSize }, 2.0f, COL_EDGE);
    m_sprite.drawFrame(sprPos, false, sprScale, WHITE);

    const float cx = sprPos.x + sprSize / 2.0f;
    const std::string name = trimmed(m_name).empty() ? std::string("(chua dat ten)") : trimmed(m_name);
    drawCenteredText(name.c_str(), cx, sprPos.y + sprSize + 8.0f, 24, COL_TEXT);
    drawCenteredText(TextFormat("%s %s", m_races[m_raceIdx].m_name.c_str(), m_classes[m_classIdx].m_name.c_str()),
                     cx, sprPos.y + sprSize + 38.0f, 18, COL_LABEL);
    DrawCircle(static_cast<int>(cx - 34), static_cast<int>(sprPos.y + sprSize + 74.0f), 7.0f, elementColor(m_elementIdx));
    DrawText(TextFormat("He %s", ELEMENT_NAMES[m_elementIdx]), static_cast<int>(cx - 22), static_cast<int>(sprPos.y + sprSize + 65.0f), 18, elementColor(m_elementIdx));

    // ---- Chi so (Race + Class) ----
    const Stats t = totalStats();
    const struct { const char* m_label; int m_value; int m_max; } rows[] = {
        { "HP", t.m_hp, m_statMax.m_hp },     { "MP", t.m_mp, m_statMax.m_mp },
        { "ATK", t.m_a, m_statMax.m_a },      { "MATK", t.m_ma, m_statMax.m_ma },
        { "DEF", t.m_d, m_statMax.m_d },      { "MDEF", t.m_md, m_statMax.m_md },
        { "SPD", t.m_spd, m_statMax.m_spd }
    };
    const float sx = PANEL_X + 275.0f;
    DrawText("CHI SO", static_cast<int>(sx), static_cast<int>(PANEL_Y + 22), 18, COL_TITLE);
    float y = PANEL_Y + 54.0f;
    for (const auto& r : rows) {
        DrawText(r.m_label, static_cast<int>(sx), static_cast<int>(y), 16, COL_LABEL);
        const Rectangle bar = { sx + 62.0f, y + 2.0f, 150.0f, 14.0f };
        DrawRectangleRec(bar, COL_BAR_BG);
        DrawRectangleRec(Rectangle{ bar.x, bar.y, bar.width * clamp01(static_cast<float>(r.m_value) / static_cast<float>(std::max(1, r.m_max))), bar.height },
                         Color{ 255, 190, 70, 255 });
        DrawRectangleLinesEx(bar, 1.0f, COL_EDGE);
        DrawText(TextFormat("%d", r.m_value), static_cast<int>(bar.x + bar.width + 8), static_cast<int>(y), 16, COL_TEXT);
        y += 33.0f;
    }

    // ---- Ky nang (Race + Class) ----
    const float ky = PANEL_Y + 340.0f;
    DrawText("KY NANG", static_cast<int>(PANEL_X + 20), static_cast<int>(ky), 18, COL_TITLE);
    int line = 0;
    float yy = ky + 28.0f;
    auto drawSkills = [&](const HeroOption& o, const char* tag, Color tagColor) {
        for (const std::string& s : o.m_skillNames) {
            if (line >= MAX_SKILL_LINES) return;
            DrawText(tag, static_cast<int>(PANEL_X + 24), static_cast<int>(yy), 16, tagColor);
            DrawText(s.c_str(), static_cast<int>(PANEL_X + 70), static_cast<int>(yy), 18, COL_TEXT);
            yy += 24.0f; ++line;
        }
    };
    drawSkills(m_races[m_raceIdx], "[T]", Color{ 120, 220, 140, 255 });     // T = Chung toc
    drawSkills(m_classes[m_classIdx], "[L]", Color{ 120, 170, 255, 255 });   // L = Lop
}

void HeroCreateScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { "QUAY LAI", "NGAU NHIEN", "XAC NHAN" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_CONFIRM);
        DrawRectangleRec(Rectangle{ r.x + 4, r.y + 5, r.width, r.height }, Color{ 0, 0, 0, 110 });
        DrawRectangleRec(r, hover ? Color{ 90, 60, 20, 235 } : (primary ? Color{ 50, 40, 14, 230 } : COL_PANEL));
        DrawRectangleLinesEx(r, hover ? 3.0f : 2.0f, (hover || primary) ? COL_EDGE_FOCUS : COL_EDGE);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24, hover ? Color{ 255, 240, 170, 255 } : COL_TEXT);
    }
}