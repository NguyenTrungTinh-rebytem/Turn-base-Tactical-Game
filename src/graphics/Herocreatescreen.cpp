#include "HeroCreateScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float AREA_X = 28.0f, AREA_W = 1224.0f;
constexpr float CARD_Y = 86.0f, CARD_AREA_H = 196.0f, CARD_MAX_W = 236.0f, CARD_GAP = 12.0f, CARD_GAP_COMPACT = 12.0f;
constexpr size_t MAX_COLUMNS = 5;          // nhieu hon 5 tuong -> xuong hang (the nho gon)
constexpr float DETAIL_Y = 296.0f, DETAIL_H = 316.0f;
constexpr float BTN_W = 220.0f, BTN_H = 48.0f, BTN_GAP = 30.0f, BTN_Y = 642.0f;
constexpr int   MAX_SKILL_LINES = 4;
constexpr float IDLE_FPS = 6.0f;
constexpr float SPRITE_SCALE = 1.5f;     // 128px * 1.5 = 192px

// ---- Mau ----
constexpr Color COL_TITLE = UiTheme::silver;
constexpr Color COL_PANEL = UiTheme::panel;
constexpr Color COL_EDGE = UiTheme::ash;
constexpr Color COL_EDGE_FOCUS = UiTheme::crimson;
constexpr Color COL_LABEL = UiTheme::ashText;
constexpr Color COL_TEXT = UiTheme::silver;
constexpr Color COL_DIM = UiTheme::ashText;
constexpr Color COL_BAR = UiTheme::hpHigh;

const char* const* const ELEMENT_NAMES = Strings::elements;
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

void drawCenteredText(const char* text, float cx, float y, int size, Color c,
                      float trackingEm = 0.0f) {
    UiText::draw(text, static_cast<int>(cx -
        UiText::measure(text, size, trackingEm) / 2.0f), static_cast<int>(y), size, c, trackingEm);
}

void drawStatBar(const char* label, int value, int maxValue, float x, float y, float barW) {
    UiText::draw(label, static_cast<int>(x), static_cast<int>(y), 14, COL_LABEL);
    const float bx = x + 48.0f, by = y + 5.0f, bh = 10.0f;
    const float ratio = clamp01(static_cast<float>(value) / static_cast<float>(std::max(1, maxValue)));
    UiShapes::runeBar(Rectangle{ bx, by, barW, bh }, ratio, COL_BAR);
    UiText::draw(TextFormat("%d", value), static_cast<int>(bx + barW + 8.0f), static_cast<int>(y - 1.0f), 18, COL_TEXT);
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
    const size_t cols = std::max<size_t>(1, std::min(m_presets.size(), MAX_COLUMNS));
    const size_t row = index / cols, col = index % cols;
    const size_t inRow = std::min(cols, m_presets.size() - row * cols);
    const float w = std::min(CARD_MAX_W,
        (AREA_W - CARD_GAP * static_cast<float>(cols - 1)) / static_cast<float>(cols));
    const float total = w * static_cast<float>(inRow) + CARD_GAP * static_cast<float>(inRow - 1);
    const float x0 = (static_cast<float>(UiCanvas::width()) - total) * 0.5f;
    const float rowH = (CARD_AREA_H - CARD_GAP_COMPACT) * 0.5f;
    return Rectangle{ x0 + static_cast<float>(col) * (w + CARD_GAP),
                      CARD_Y + static_cast<float>(row) * (rowH + CARD_GAP_COMPACT),
                      w, rowH };
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

    const Vector2 mouse = UiCanvas::mousePosition();
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
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(UiCanvas::width()), static_cast<float>(UiCanvas::height()) });
        DrawRectangle(0, 0, 1280, 720, Color{ 0, 0, 0, 120 });
    } else {
        DrawRectangleGradientV(0, 0, 1280, 720, Color{ 25, 16, 23, 255 }, UiTheme::ink);
    }
}

void HeroCreateScreen::drawCards() const {
    for (size_t i = 0; i < m_presets.size(); ++i) {
        const HeroPreset& p = m_presets[i];
        Rectangle r = cardRect(i);
        const bool selected = (i == m_selected);
        const bool hover = (static_cast<int>(i) == m_hoverCard);
        if (selected) r.y -= 4.0f;
        UiShapes::plate(r,
            selected ? Color{ 50, 10, 17, 224 } : UiTheme::panel,
            selected ? COL_EDGE_FOCUS : (hover ? COL_LABEL : COL_EDGE),
            selected ? 2.0f : 1.0f);

        const float spriteSize = 64.0f;
        const Vector2 spritePos{ r.x + 10.0f, r.y + 10.0f };
        const Texture2D& tex = m_presetTextures[i];
        if (tex.id != 0 && tex.height > 0) {
            const int fs = tex.height;
            const int frames = std::max(1, tex.width / fs);
            const int frame = static_cast<int>(m_time * IDLE_FPS) % frames;
            DrawTexturePro(tex,
                           Rectangle{ static_cast<float>(frame * fs), 0.0f, static_cast<float>(fs), static_cast<float>(fs) },
                           Rectangle{ spritePos.x, spritePos.y, spriteSize, spriteSize },
                           Vector2{ 0, 0 }, 0.0f,
                           selected ? WHITE : Color{ 175, 175, 185, 255 });
        } else {
            const float scale = spriteSize / static_cast<float>(std::max(1, m_sprite.getFrameWidth()));
            m_sprite.drawFrame(spritePos, false, scale,
                               selected ? WHITE : Color{ 175, 175, 185, 255 });
        }

        const bool validIdx = p.m_raceIndex < m_races.size() && p.m_classIndex < m_classes.size();
        const size_t e = static_cast<size_t>(p.m_element);
        const Color nameCol = selected ? COL_TITLE : Color{ 190, 188, 205, 255 };
        const float tx = r.x + spriteSize + 22.0f;
        const float textWidth = r.width - spriteSize - 30.0f;
        const std::string heroName = Strings::localizedName(p.m_name);
        UiText::draw(heroName.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 12.0f), 17, nameCol);
        if (validIdx) {
            const std::string race = Strings::localizedName(m_races[p.m_raceIndex].m_name);
            const std::string heroClass = Strings::localizedName(m_classes[p.m_classIndex].m_name);
            const std::string role = race + u8" · " + heroClass;
            if (UiText::measure(role, 15) <= textWidth) {
                UiText::draw(role.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 39.0f), 15,
                             selected ? COL_TEXT : COL_LABEL);
            } else {
                UiText::draw(race.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 35.0f), 14, COL_LABEL);
                UiText::draw(heroClass.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 54.0f), 14, COL_LABEL);
            }
        }
        if (e < ELEMENT_COUNT) {
            UiIcons::draw(std::string("element:") + (e == 0 ? "kim" : e == 1 ? "moc" :
                e == 2 ? "thuy" : e == 3 ? "hoa" : "tho"),
                Rectangle{ tx, r.y + 74.0f, 16.0f, 16.0f }, elementColor(e));
            UiText::draw(ELEMENT_NAMES[e], static_cast<int>(tx + 22.0f),
                         static_cast<int>(r.y + 74.0f), 13, selected ? COL_TEXT : COL_LABEL);
        }
    }
}

void HeroCreateScreen::drawDetail() const {
    const Rectangle panel{ AREA_X, DETAIL_Y, AREA_W, DETAIL_H };
    UiShapes::plate(panel, COL_PANEL, COL_EDGE, 1.0f);
    if (m_selected >= m_presets.size()) return;

    const HeroPreset& p = m_presets[m_selected];
    const Stats s = totalStats(p);
    const std::string heroName = Strings::localizedName(p.m_name);
    const std::string race = p.m_raceIndex < m_races.size()
        ? Strings::localizedName(m_races[p.m_raceIndex].m_name) : std::string{};
    const std::string heroClass = p.m_classIndex < m_classes.size()
        ? Strings::localizedName(m_classes[p.m_classIndex].m_name) : std::string{};
    const float previewX = panel.x + 28.0f;
    const float previewY = panel.y + 76.0f;
    const float previewSize = 172.0f;
    const Color element = elementColor(static_cast<size_t>(p.m_element));
    DrawEllipse(static_cast<int>(previewX + previewSize * 0.5f),
                static_cast<int>(previewY + previewSize - 8.0f), 76.0f, 16.0f,
                Color{ element.r, element.g, element.b, 105 });
    const Texture2D& preview = m_presetTextures[m_selected];
    if (preview.id != 0 && preview.height > 0) {
        const int frameSize = preview.height;
        const int frames = std::max(1, preview.width / frameSize);
        const int frame = static_cast<int>(m_time * IDLE_FPS) % frames;
        DrawTexturePro(preview,
            Rectangle{ static_cast<float>(frame * frameSize), 0.0f,
                       static_cast<float>(frameSize), static_cast<float>(frameSize) },
            Rectangle{ previewX, previewY, previewSize, previewSize },
            Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        const float scale = previewSize /
            static_cast<float>(std::max(1, m_sprite.getFrameWidth()));
        m_sprite.drawFrame(Vector2{ previewX, previewY }, false, scale);
    }

    const float infoX = panel.x + 220.0f;
    UiText::draw(heroName.c_str(), static_cast<int>(infoX), static_cast<int>(panel.y + 20.0f), 26, COL_TITLE);
    UiText::draw((race + u8" · " + heroClass).c_str(), static_cast<int>(infoX),
                 static_cast<int>(panel.y + 54.0f), 19, COL_TEXT);
    UiIcons::draw(std::string("element:") + (static_cast<int>(p.m_element) == 0 ? "kim" :
        static_cast<int>(p.m_element) == 1 ? "moc" : static_cast<int>(p.m_element) == 2 ? "thuy" :
        static_cast<int>(p.m_element) == 3 ? "hoa" : "tho"),
        Rectangle{ infoX, panel.y + 80.0f, 20.0f, 20.0f }, element);
    UiText::draw(TextFormat(Strings::elementPrefix, ELEMENT_NAMES[static_cast<size_t>(p.m_element)]),
                 static_cast<int>(infoX + 28.0f), static_cast<int>(panel.y + 80.0f), 16, element);

    const float statX = infoX;
    const float y0 = panel.y + 116.0f;
    const float pitch = 26.0f;
    const float barW = 96.0f;
    drawStatBar("HP", s.m_hp, m_statMax.m_hp, statX, y0, barW);
    drawStatBar("MP", s.m_mp, m_statMax.m_mp, statX, y0 + pitch, barW);
    drawStatBar("ATK", s.m_a, m_statMax.m_a, statX, y0 + pitch * 2, barW);
    drawStatBar("MATK", s.m_ma, m_statMax.m_ma, statX, y0 + pitch * 3, barW);
    drawStatBar("DEF", s.m_d, m_statMax.m_d, statX + 210.0f, y0, barW);
    drawStatBar("MDEF", s.m_md, m_statMax.m_md, statX + 210.0f, y0 + pitch, barW);
    drawStatBar("SPD", s.m_spd, m_statMax.m_spd, statX + 210.0f, y0 + pitch * 2, barW);

    const float sx = panel.x + 650.0f;
    UiText::draw(Strings::skillMenuTitle, static_cast<int>(sx), static_cast<int>(panel.y + 20), 18, COL_TITLE);
    struct SkillEntry { std::string name; const char* source; Color color; };
    std::vector<SkillEntry> skills;
    if (p.m_raceIndex < m_races.size())
        for (const std::string& name : m_races[p.m_raceIndex].m_skillNames)
            if (skills.size() < MAX_SKILL_LINES) skills.push_back({ name, u8"Tộc", Color{ 120, 230, 160, 255 } });
    if (p.m_classIndex < m_classes.size())
        for (const std::string& name : m_classes[p.m_classIndex].m_skillNames)
            if (skills.size() < MAX_SKILL_LINES) skills.push_back({ name, u8"Lớp", Color{ 120, 170, 255, 255 } });
    m_hoverSkill = -1;
    const Vector2 mouse = UiCanvas::mousePosition();
    for (size_t i = 0; i < skills.size(); ++i) {
        const float y = panel.y + 50.0f + static_cast<float>(i) * 48.0f;
        const Rectangle row{ sx, y, panel.x + panel.width - sx - 24.0f, 40.0f };
        const bool hovered = CheckCollisionPointRec(mouse, row);
        if (hovered) m_hoverSkill = static_cast<int>(i);
        UiShapes::bladeStrip(row,
            hovered ? UiTheme::panelRaised : UiTheme::panel,
            hovered ? COL_EDGE_FOCUS : COL_EDGE, false, hovered, hovered);
        UiIcons::draw("action:skill",
            Rectangle{ row.x + 8.0f, row.y + 7.0f, 26.0f, 26.0f }, element);
        UiText::draw(skills[i].source, static_cast<int>(row.x + 42.0f),
                     static_cast<int>(row.y + 11.0f), 14, skills[i].color);
        const std::string name = Strings::localizedName(skills[i].name);
        UiText::draw(name.c_str(), static_cast<int>(row.x + 86.0f),
                     static_cast<int>(row.y + 10.0f), 15, COL_TEXT);
    }
    if (m_hoverSkill >= 0) {
        const SkillEntry& skill = skills[static_cast<size_t>(m_hoverSkill)];
        UiText::draw(Strings::localizedName(skill.name).c_str(), static_cast<int>(sx),
                     static_cast<int>(panel.y + panel.height - 28.0f), 14, COL_LABEL);
    }
}

void HeroCreateScreen::drawButtons() const {
    const char* const labels[BTN_COUNT] = { Strings::back, Strings::confirm };
    for (int b = 0; b < BTN_COUNT; ++b) {
        const Rectangle r = buttonRect(b);
        const bool hover = (m_hoverButton == b);
        const bool primary = (b == BTN_CONFIRM);
        UiShapes::bladeStrip(r, primary ? Color{ 50, 10, 17, 224 } : COL_PANEL,
                             primary ? COL_EDGE_FOCUS : COL_EDGE, false,
                             primary, hover);
        drawCenteredText(labels[b], r.x + r.width / 2.0f, r.y + 14.0f, 24,
                         primary ? COL_TITLE : COL_TEXT, UiTheme::displayTracking);
    }
}

void HeroCreateScreen::draw() const {
    drawBackground();
    drawCenteredText(m_slotLabel.c_str(), 640.0f, 28.0f, 44, COL_TITLE,
                     UiTheme::displayTracking);
    if (!hasOptions()) {
        drawCenteredText(u8"Chưa có dữ liệu tướng (data/heroes.csv, races.csv, classes.csv)", 640.0f, 340.0f, 22, COL_DIM);
        drawButtons();
        return;
    }
    drawCards();
    drawDetail();
    drawButtons();
}
