#include "HeroCreateScreen.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float AREA_X = 40.0f, AREA_W = 1200.0f;
constexpr float CARD_Y = 95.0f, CARD_AREA_H = 305.0f, CARD_MAX_W = 380.0f, CARD_GAP = 30.0f, CARD_GAP_COMPACT = 14.0f;
constexpr size_t MAX_COLUMNS = 5;          // nhieu hon 5 tuong -> xuong hang (the nho gon)
constexpr float COMPACT_BELOW_H = 200.0f;  // the thap hon muc nay -> bo cuc nho gon
constexpr float DETAIL_Y = 410.0f, DETAIL_H = 210.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr int   MAX_SKILL_LINES = 4;
constexpr float IDLE_FPS = 6.0f;
constexpr float SPRITE_SCALE = 1.5f;     // 128px * 1.5 = 192px

// ---- Mau ----
constexpr Color COL_TITLE      { 255, 214, 96, 255 };
constexpr Color COL_PANEL      { 16, 13, 36, 215 };
constexpr Color COL_EDGE       { 140, 120, 180, 255 };
constexpr Color COL_EDGE_FOCUS { 255, 214, 96, 255 };
constexpr Color COL_LABEL      { 190, 185, 220, 255 };
constexpr Color COL_TEXT       { 238, 235, 252, 255 };
constexpr Color COL_DIM        { 130, 126, 160, 255 };
constexpr Color COL_BAR_BG     { 34, 30, 60, 255 };
constexpr Color COL_BAR        { 255, 190, 70, 255 };

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

void drawStatBar(const char* label, int value, int maxValue, float x, float y, float barW) {
    DrawText(label, static_cast<int>(x), static_cast<int>(y), 16, COL_LABEL);
    const float bx = x + 70.0f, by = y + 3.0f, bh = 12.0f;
    DrawRectangle(static_cast<int>(bx), static_cast<int>(by), static_cast<int>(barW), static_cast<int>(bh), COL_BAR_BG);
    const float ratio = clamp01(static_cast<float>(value) / static_cast<float>(std::max(1, maxValue)));
    DrawRectangle(static_cast<int>(bx), static_cast<int>(by), static_cast<int>(barW * ratio), static_cast<int>(bh), COL_BAR);
    DrawRectangleLines(static_cast<int>(bx), static_cast<int>(by), static_cast<int>(barW), static_cast<int>(bh), COL_EDGE);
    DrawText(TextFormat("%d", value), static_cast<int>(bx + barW + 10.0f), static_cast<int>(y), 16, COL_TEXT);
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
    if (m_hasBackground) SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);
    return m_hasBackground;
}

bool HeroCreateScreen::loadPreviewSprite(const std::string& path, int frameWidth, int frameHeight) {
    const bool ok = m_sprite.load(path, frameWidth, frameHeight);   // that bai => placeholder
    m_sprite.play(AnimState::IDLE);
    return ok;
}

void HeroCreateScreen::unloadPresetTextures() {
    for (Texture2D& t : m_presetTextures)
        if (t.id != 0 && IsWindowReady()) UnloadTexture(t);
    m_presetTextures.clear();
}

void HeroCreateScreen::unloadAll() {
    m_sprite.unload();
    unloadPresetTextures();
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
}

void HeroCreateScreen::setPresets(const std::vector<HeroPreset>& presets) {
    unloadPresetTextures();
    m_presets = presets;
    for (const HeroPreset& p : m_presets) {
        Texture2D tex{};
        if (IsWindowReady() && !p.m_idleSpritePath.empty() && FileExists(p.m_idleSpritePath.c_str())) {
            tex = LoadTexture(p.m_idleSpritePath.c_str());
            if (tex.id != 0) SetTextureFilter(tex, TEXTURE_FILTER_POINT);   // pixel art: khong lam mo
        }
        m_presetTextures.push_back(tex);
    }
    m_selected = std::min(m_selected, m_presets.empty() ? size_t{ 0 } : m_presets.size() - 1);
}

void HeroCreateScreen::reset(const std::string&) {
    m_selected = 0;
    m_hoverCard = -1;
    m_hoverButton = -1;
    m_time = 0.0f;
}

HeroSelection HeroCreateScreen::getSelection() const {
    HeroSelection s;
    if (m_selected < m_presets.size()) {
        const HeroPreset& p = m_presets[m_selected];
        s.m_name = p.m_name;
        s.m_raceIndex = p.m_raceIndex;
        s.m_classIndex = p.m_classIndex;
        s.m_element = p.m_element;
    }
    return s;
}

Stats HeroCreateScreen::totalStats(const HeroPreset& preset) const {
    if (preset.m_raceIndex >= m_races.size() || preset.m_classIndex >= m_classes.size()) return Stats{};
    return m_races[preset.m_raceIndex].m_stats + m_classes[preset.m_classIndex].m_stats;
}

// ============ Bo cuc ============
size_t HeroCreateScreen::columnCount() const {
    return std::max<size_t>(1, std::min(m_presets.size(), MAX_COLUMNS));
}

Rectangle HeroCreateScreen::cardRect(size_t index) const {
    const size_t n = std::max<size_t>(1, m_presets.size());
    const size_t cols = columnCount();
    const size_t rows = (n + cols - 1) / cols;
    const float gapX = (rows == 1) ? CARD_GAP : CARD_GAP_COMPACT;
    const float gapY = CARD_GAP_COMPACT;
    const float w = std::min(CARD_MAX_W, (AREA_W - gapX * static_cast<float>(cols - 1)) / static_cast<float>(cols));
    const float h = (rows == 1) ? CARD_AREA_H - 5.0f
                                : (CARD_AREA_H - gapY * static_cast<float>(rows - 1)) / static_cast<float>(rows);
    const size_t row = index / cols, col = index % cols;
    const size_t inRow = std::min(cols, n - row * cols);                 // so the o hang nay (hang cuoi co the thieu)
    const float total = w * static_cast<float>(inRow) + gapX * static_cast<float>(inRow - 1);
    const float x0 = AREA_X + (AREA_W - total) / 2.0f;
    return Rectangle{ x0 + static_cast<float>(col) * (w + gapX), CARD_Y + static_cast<float>(row) * (h + gapY), w, h };
}

Rectangle HeroCreateScreen::buttonRect(int index) const {
    const float total = BTN_W * BTN_COUNT + BTN_GAP * (BTN_COUNT - 1);
    const float x0 = (1280.0f - total) / 2.0f;
    return Rectangle{ x0 + static_cast<float>(index) * (BTN_W + BTN_GAP), BTN_Y, BTN_W, BTN_H };
}

void HeroCreateScreen::moveSelectionRow(int rowDelta) {
    if (m_presets.empty()) return;
    const long cols = static_cast<long>(columnCount());
    const long n = static_cast<long>(m_presets.size());
    long target = static_cast<long>(m_selected) + rowDelta * cols;
    if (target >= 0 && target < n) m_selected = static_cast<size_t>(target);   // het hang -> dung yen
}

void HeroCreateScreen::moveSelection(int delta) {
    if (m_presets.empty()) return;
    const int n = static_cast<int>(m_presets.size());
    m_selected = static_cast<size_t>((static_cast<int>(m_selected) + delta % n + n) % n);
}

// ============ Update / Input ============
void HeroCreateScreen::update(float deltaTime) {
    m_time += deltaTime;
    m_sprite.update(deltaTime);
}

HeroCreateAction HeroCreateScreen::handleInput() {
    if (!hasOptions()) return HeroCreateAction::NONE;

    if (IsKeyPressed(KEY_ESCAPE)) return HeroCreateAction::BACK;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) moveSelection(-1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) moveSelection(+1);
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) moveSelectionRow(-1);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) moveSelectionRow(+1);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
        return HeroCreateAction::CONFIRM;

    const Vector2 mouse = GetMousePosition();
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    m_hoverCard = -1;
    for (size_t i = 0; i < m_presets.size(); ++i) {
        if (CheckCollisionPointRec(mouse, cardRect(i))) {
            m_hoverCard = static_cast<int>(i);
            if (clicked) m_selected = i;
        }
    }

    m_hoverButton = -1;
    for (int b = 0; b < BTN_COUNT; ++b) {
        if (CheckCollisionPointRec(mouse, buttonRect(b))) {
            m_hoverButton = b;
            if (clicked) return b == BTN_BACK ? HeroCreateAction::BACK : HeroCreateAction::CONFIRM;
        }
    }
    return HeroCreateAction::NONE;
}

// ============ Ve ============
void HeroCreateScreen::drawBackground() const {
    if (m_hasBackground) {
        DrawTexturePro(m_background,
                       Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, 1280.0f, 720.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
        DrawRectangle(0, 0, 1280, 720, Color{ 0, 0, 0, 120 });
    } else {
        DrawRectangleGradientV(0, 0, 1280, 720, Color{ 24, 20, 48, 255 }, Color{ 8, 6, 20, 255 });
    }
}

void HeroCreateScreen::drawCards() const {
    for (size_t i = 0; i < m_presets.size(); ++i) {
        const HeroPreset& p = m_presets[i];
        const Rectangle r = cardRect(i);
        const bool compact = r.height < COMPACT_BELOW_H;
        const bool selected = (i == m_selected);
        const bool hover = (static_cast<int>(i) == m_hoverCard);

        DrawRectangleRec(r, selected ? Color{ 34, 28, 64, 235 } : COL_PANEL);
        DrawRectangleLinesEx(r, selected ? 3.0f : 2.0f, selected ? COL_EDGE_FOCUS : (hover ? COL_LABEL : COL_EDGE));

        // --- Sprite IDLE (dai frame vuong, vd 4 x 128px) ---
        const float spriteSize = compact ? 96.0f : 192.0f;
        const Vector2 spritePos = compact ? Vector2{ r.x + 8.0f, r.y + (r.height - spriteSize) / 2.0f }
                                          : Vector2{ r.x + (r.width - spriteSize) / 2.0f, r.y + 16.0f };
        const Texture2D& tex = m_presetTextures[i];
        if (tex.id != 0 && tex.height > 0) {
            const int fs = tex.height;
            const int frames = std::max(1, tex.width / fs);
            const int frame = static_cast<int>(m_time * IDLE_FPS) % frames;
            DrawTexturePro(tex,
                           Rectangle{ static_cast<float>(frame * fs), 0.0f, static_cast<float>(fs), static_cast<float>(fs) },
                           Rectangle{ spritePos.x, spritePos.y, spriteSize, spriteSize },
                           Vector2{ 0, 0 }, 0.0f, WHITE);
        } else {
            const float scale = spriteSize / static_cast<float>(std::max(1, m_sprite.getFrameWidth()));
            m_sprite.drawFrame(spritePos, false, scale);
        }

        const bool validIdx = p.m_raceIndex < m_races.size() && p.m_classIndex < m_classes.size();
        const size_t e = static_cast<size_t>(p.m_element);
        const Color nameCol = selected ? COL_TITLE : COL_TEXT;

        if (compact) {
            // Chu ben phai sprite: ten / Chung toc / Lop / He
            const float tx = r.x + 8.0f + spriteSize + 8.0f;
            DrawText(p.m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 14.0f), 20, nameCol);
            if (validIdx) {
                DrawText(m_races[p.m_raceIndex].m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 44.0f), 16, COL_LABEL);
                DrawText(m_classes[p.m_classIndex].m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 66.0f), 16, COL_LABEL);
            }
            if (e < ELEMENT_COUNT) {
                DrawCircle(static_cast<int>(tx + 5.0f), static_cast<int>(r.y + 100.0f), 5.0f, elementColor(e));
                DrawText(ELEMENT_NAMES[e], static_cast<int>(tx + 16.0f), static_cast<int>(r.y + 92.0f), 16, COL_TEXT);
            }
        } else {
            const float cx = r.x + r.width / 2.0f;
            drawCenteredText(p.m_name.c_str(), cx, r.y + 214.0f, 26, nameCol);
            if (validIdx) {
                const std::string rc = m_races[p.m_raceIndex].m_name + " " + m_classes[p.m_classIndex].m_name;
                drawCenteredText(rc.c_str(), cx, r.y + 246.0f, 18, COL_LABEL);
            }
            if (e < ELEMENT_COUNT) {
                const char* text = TextFormat("He %s", ELEMENT_NAMES[e]);
                const int tw = MeasureText(text, 18);
                DrawCircle(static_cast<int>(cx - tw / 2.0f - 12.0f), static_cast<int>(r.y + 278.0f), 6.0f, elementColor(e));
                DrawText(text, static_cast<int>(cx - tw / 2.0f), static_cast<int>(r.y + 269.0f), 18, COL_TEXT);
            }
        }
    }
}

void HeroCreateScreen::drawDetail() const {
    const Rectangle panel{ AREA_X, DETAIL_Y, AREA_W, DETAIL_H };
    DrawRectangleRec(panel, COL_PANEL);
    DrawRectangleLinesEx(panel, 2.0f, COL_EDGE);
    if (m_selected >= m_presets.size()) return;

    const HeroPreset& p = m_presets[m_selected];
    const Stats s = totalStats(p);

    // --- Chi so (2 cot) ---
    DrawText(TextFormat("CHI SO - %s", p.m_name.c_str()), static_cast<int>(panel.x + 24), static_cast<int>(panel.y + 14), 18, COL_TITLE);
    const float col1 = panel.x + 24.0f, col2 = panel.x + 320.0f, y0 = panel.y + 48.0f, pitch = 38.0f, barW = 140.0f;
    drawStatBar("HP",   s.m_hp,  m_statMax.m_hp,  col1, y0,             barW);
    drawStatBar("MP",   s.m_mp,  m_statMax.m_mp,  col1, y0 + pitch,     barW);
    drawStatBar("ATK",  s.m_a,   m_statMax.m_a,   col1, y0 + pitch * 2, barW);
    drawStatBar("MATK", s.m_ma,  m_statMax.m_ma,  col1, y0 + pitch * 3, barW);
    drawStatBar("DEF",  s.m_d,   m_statMax.m_d,   col2, y0,             barW);
    drawStatBar("MDEF", s.m_md,  m_statMax.m_md,  col2, y0 + pitch,     barW);
    drawStatBar("SPD",  s.m_spd, m_statMax.m_spd, col2, y0 + pitch * 2, barW);

    // --- Ky nang: [T] cua Chung toc, [L] cua Lop ---
    const float sx = panel.x + 660.0f;
    DrawText("KY NANG", static_cast<int>(sx), static_cast<int>(panel.y + 14), 18, COL_TITLE);
    int line = 0;
    auto drawSkills = [&](const std::vector<std::string>& names, const char* tag, Color tagColor) {
        for (const std::string& name : names) {
            if (line >= MAX_SKILL_LINES) return;
            const float y = panel.y + 48.0f + static_cast<float>(line) * 34.0f;
            DrawText(tag, static_cast<int>(sx), static_cast<int>(y), 20, tagColor);
            DrawText(name.c_str(), static_cast<int>(sx + 40.0f), static_cast<int>(y), 20, COL_TEXT);
            ++line;
        }
    };
    if (p.m_raceIndex < m_races.size())   drawSkills(m_races[p.m_raceIndex].m_skillNames,   "[T]", Color{ 120, 230, 160, 255 });
    if (p.m_classIndex < m_classes.size()) drawSkills(m_classes[p.m_classIndex].m_skillNames, "[L]", Color{ 120, 170, 255, 255 });
}

void HeroCreateScreen::drawButtons() const {
    const char* const labels[BTN_COUNT] = { "QUAY LAI", "XAC NHAN" };
    for (int b = 0; b < BTN_COUNT; ++b) {
        const Rectangle r = buttonRect(b);
        const bool hover = (m_hoverButton == b);
        const bool primary = (b == BTN_CONFIRM);
        DrawRectangleRec(r, hover ? Color{ 52, 44, 92, 255 } : COL_PANEL);
        DrawRectangleLinesEx(r, 2.0f, primary ? COL_EDGE_FOCUS : COL_EDGE);
        drawCenteredText(labels[b], r.x + r.width / 2.0f, r.y + 14.0f, 24, primary ? COL_TITLE : COL_TEXT);
    }
}

void HeroCreateScreen::draw() const {
    drawBackground();
    drawCenteredText("CHON ANH HUNG", 640.0f, 28.0f, 48, COL_TITLE);
    if (!hasOptions()) {
        drawCenteredText("Chua co du lieu tuong (data/heroes.csv, races.csv, classes.csv)", 640.0f, 340.0f, 22, COL_DIM);
        drawButtons();
        return;
    }
    drawCards();
    drawDetail();
    drawButtons();
    drawCenteredText("[Mui ten] Chon tuong   [ENTER] Xac nhan   [ESC] Quay lai", 640.0f, 698.0f, 16, COL_DIM);
}
