#include "HeroCreateScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiInput.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
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
constexpr Color COL_TITLE      = UiTheme::accent;
constexpr Color COL_PANEL      = UiTheme::panel;
constexpr Color COL_EDGE       = UiTheme::border;
constexpr Color COL_EDGE_FOCUS = UiTheme::ember;
constexpr Color COL_LABEL      = UiTheme::ash;
constexpr Color COL_TEXT       = UiTheme::text;
constexpr Color COL_DIM        = UiTheme::ash;
constexpr Color COL_BAR_BG     = UiTheme::panelBottom;
constexpr Color COL_BAR        = UiTheme::accent;

const char* const* const ELEMENT_NAMES = Strings::elements;
constexpr size_t ELEMENT_COUNT = 5;

Color elementColor(size_t i) {
    switch (i) {
        case 0: return UiTheme::kim;
        case 1: return UiTheme::moc;
        case 2: return UiTheme::thuy;
        case 3: return UiTheme::hoa;
        default: return UiTheme::tho;
    }
}

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

void drawCenteredText(const char* text, float cx, float y, int size, Color c,
                      float letterSpacing = 0.0f, UiFontRole role = UiFontRole::AUTO) {
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size, role, letterSpacing) / 2.0f),
                 static_cast<int>(y), size, c, role, letterSpacing);
}

void drawStatBar(const char* label, int value, int maxValue, float x, float y, float barW) {
    UiText::draw(label, static_cast<int>(x), static_cast<int>(y), 16, COL_LABEL);
    const float bx = x + 112.0f, by = y + 3.0f, bh = 12.0f;
    const float ratio = clamp01(static_cast<float>(value) / static_cast<float>(std::max(1, maxValue)));
    UiShapes::roundedRect(Rectangle{ bx, by, barW, bh }, UiTheme::radiusBar,
                          COL_BAR_BG, COL_EDGE, 1.0f);
    if (ratio > 0.0f) {
        UiShapeEffects effects;
        effects.gradientBottom = UiTheme::primaryBottom;
        UiShapes::roundedRect(Rectangle{ bx, by, barW * ratio, bh }, UiTheme::radiusBar,
                              COL_BAR, BLANK, 0.0f, effects);
    }
    UiText::draw(TextFormat("%d", value), static_cast<int>(bx + barW + 10.0f), static_cast<int>(y), 16, COL_TEXT);
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

    if (UiInput::rightClicked()) {
        UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
        return HeroCreateAction::BACK;
    }
    if (IsKeyPressed(KEY_ESCAPE)) return HeroCreateAction::BACK;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) moveSelection(-1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) moveSelection(+1);
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) moveSelectionRow(-1);
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) moveSelectionRow(+1);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
        return HeroCreateAction::CONFIRM;

    const Vector2 mouse = UiInput::mousePosition();
    const bool clicked = UiInput::pressed(MOUSE_BUTTON_LEFT);

    m_hoverCard = -1;
    for (size_t i = 0; i < m_presets.size(); ++i) {
        if (CheckCollisionPointRec(mouse, cardRect(i))) {
            m_hoverCard = static_cast<int>(i);
            UiInput::setCursorType(UiInput::CursorType::POINTER);
            if (clicked) UiInput::consumeClick();
            if (clicked) m_selected = i;
        }
    }

    m_hoverButton = -1;
    for (int b = 0; b < BTN_COUNT; ++b) {
        if (CheckCollisionPointRec(mouse, buttonRect(b))) {
            m_hoverButton = b;
            UiInput::setCursorType(UiInput::CursorType::POINTER);
            if (clicked) {
                UiInput::consumeClick();
                return b == BTN_BACK ? HeroCreateAction::BACK : HeroCreateAction::CONFIRM;
            }
        }
    }
    return HeroCreateAction::NONE;
}

// ============ Ve ============
void HeroCreateScreen::drawBackground() const {
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(UiCanvas::width()), static_cast<float>(UiCanvas::height()) });
        DrawRectangle(0, 0, 1280, 720, Color{ 12, 10, 18, 135 });
    } else {
        DrawRectangleGradientV(0, 0, 1280, 720, UiTheme::panelBottom, UiTheme::obsidian);
    }
}

void HeroCreateScreen::drawCards() const {
    for (size_t i = 0; i < m_presets.size(); ++i) {
        const HeroPreset& p = m_presets[i];
        const Rectangle r = cardRect(i);
        const bool compact = r.height < COMPACT_BELOW_H;
        const bool selected = (i == m_selected);
        const bool hover = (static_cast<int>(i) == m_hoverCard);

        UiShapeEffects cardEffects;
        cardEffects.gradientBottom = UiTheme::panelBottom;
        cardEffects.shadowBlur = selected ? 8.0f : 4.0f;
        cardEffects.glow = UiTheme::ember;
        cardEffects.glowRadius = selected ? 4.0f : 0.0f;
        cardEffects.glowStrength = selected ? 0.3f : 0.0f;
        UiShapes::roundedRect(r, UiTheme::radiusCard, selected ? UiTheme::panelBottom : COL_PANEL,
                              selected ? COL_EDGE_FOCUS : (hover ? UiTheme::accent : COL_EDGE),
                              1.0f, cardEffects);

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
            const std::string heroName = Strings::localizedName(p.m_name);
            UiText::draw(heroName.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 14.0f), 20, nameCol);
            const int typeWidth = std::max(64, static_cast<int>(r.x + r.width - tx - 8.0f));
            if (validIdx) {
                const std::string race = Strings::localizedName(m_races[p.m_raceIndex].m_name);
                const std::string heroClass = Strings::localizedName(m_classes[p.m_classIndex].m_name);
                UiText::drawWrapped(race, static_cast<int>(tx), static_cast<int>(r.y + 38.0f),
                                    typeWidth, 14, COL_LABEL, 16, UiFontRole::BODY);
                UiText::drawWrapped(heroClass, static_cast<int>(tx), static_cast<int>(r.y + 69.0f),
                                    typeWidth, 14, COL_LABEL, 16, UiFontRole::BODY);
            }
            if (e < ELEMENT_COUNT) {
                UiElementSigil::draw(static_cast<Element>(e),
                    Rectangle{ tx, r.y + r.height - 30.0f, 20.0f, 20.0f });
                UiText::draw(ELEMENT_NAMES[e], static_cast<int>(tx + 26.0f),
                             static_cast<int>(r.y + r.height - 29.0f), 14, COL_TEXT, UiFontRole::HUD);
            }
        } else {
            const float cx = r.x + r.width / 2.0f;
            const std::string heroName = Strings::localizedName(p.m_name);
            drawCenteredText(heroName.c_str(), cx, r.y + 214.0f, 26, nameCol);
            if (validIdx) {
                const std::string race = Strings::localizedName(m_races[p.m_raceIndex].m_name);
                const std::string heroClass = Strings::localizedName(m_classes[p.m_classIndex].m_name);
                drawCenteredText(race.c_str(), cx, r.y + 244.0f, 16, COL_LABEL);
                drawCenteredText(heroClass.c_str(), cx, r.y + 263.0f, 16, COL_LABEL);
            }
            if (e < ELEMENT_COUNT) {
                const char* text = TextFormat(Strings::elementPrefix, ELEMENT_NAMES[e]);
                const int tw = UiText::measure(text, 18);
                UiElementSigil::draw(static_cast<Element>(e),
                    Rectangle{ cx - tw / 2.0f - 24.0f, r.y + 280.0f, 20.0f, 20.0f });
                UiText::draw(text, static_cast<int>(cx - tw / 2.0f), static_cast<int>(r.y + 282.0f), 18, COL_TEXT);
            }
        }
    }
}

void HeroCreateScreen::drawDetail() const {
    const Rectangle panel{ AREA_X, DETAIL_Y, AREA_W, DETAIL_H };
    UiShapeEffects panelEffects;
    panelEffects.gradientBottom = UiTheme::panelBottom;
    panelEffects.shadowBlur = 8.0f;
    UiShapes::roundedRect(panel, UiTheme::radiusLarge, COL_PANEL, COL_EDGE, 1.0f, panelEffects);
    if (m_selected >= m_presets.size()) return;

    const HeroPreset& p = m_presets[m_selected];
    const Stats s = totalStats(p);

    // --- Preview and hero identity ---
    const std::string heroName = Strings::localizedName(p.m_name);
    const float previewX = panel.x + 20.0f;
    const float previewY = panel.y + 28.0f;
    const float previewSize = 152.0f;
    const Color elementTint = elementColor(static_cast<size_t>(p.m_element));
    DrawEllipse(static_cast<int>(previewX + previewSize * 0.5f),
                static_cast<int>(panel.y + panel.height - 18.0f), 62.0f, 12.0f,
                Color{ elementTint.r, elementTint.g, elementTint.b, 90 });
    const Texture2D& preview = m_presetTextures[m_selected];
    if (preview.id != 0 && preview.height > 0) {
        const int frameSize = preview.height;
        const int frameCount = std::max(1, preview.width / frameSize);
        const int frame = static_cast<int>(m_time * IDLE_FPS) % frameCount;
        DrawTexturePro(preview,
            Rectangle{ static_cast<float>(frame * frameSize), 0.0f,
                       static_cast<float>(frameSize), static_cast<float>(frameSize) },
            Rectangle{ previewX, previewY, previewSize, previewSize },
            Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        const float scale = previewSize / static_cast<float>(std::max(1, m_sprite.getFrameWidth()));
        m_sprite.drawFrame(Vector2{ previewX, previewY }, false, scale);
    }

    const float identityX = panel.x + 184.0f;
    UiText::drawEllipsis(heroName, static_cast<int>(identityX), static_cast<int>(panel.y + 50.0f),
                         150, 25, COL_TITLE, UiFontRole::HUD);
    if (p.m_raceIndex < m_races.size() && p.m_classIndex < m_classes.size()) {
        const std::string race = Strings::localizedName(m_races[p.m_raceIndex].m_name);
        const std::string heroClass = Strings::localizedName(m_classes[p.m_classIndex].m_name);
        UiText::drawEllipsis(race, static_cast<int>(identityX), static_cast<int>(panel.y + 82.0f),
                             150, 17, COL_TEXT, UiFontRole::BODY);
        UiText::drawEllipsis(heroClass, static_cast<int>(identityX), static_cast<int>(panel.y + 106.0f),
                             150, 17, COL_TEXT, UiFontRole::BODY);
    }
    const size_t elementIndex = static_cast<size_t>(p.m_element);
    if (elementIndex < ELEMENT_COUNT) {
        UiElementSigil::draw(p.m_element,
            Rectangle{ identityX, panel.y + 142.0f, 22.0f, 22.0f });
        UiText::draw(ELEMENT_NAMES[elementIndex], static_cast<int>(identityX + 30.0f),
                     static_cast<int>(panel.y + 144.0f), 17, elementTint, UiFontRole::HUD);
    }

    // --- Stats ---
    const float col1 = panel.x + 350.0f, col2 = panel.x + 610.0f;
    const float y0 = panel.y + 34.0f, pitch = 38.0f, barW = 88.0f;
    drawStatBar(Strings::statHp, s.m_hp, m_statMax.m_hp, col1, y0, barW);
    drawStatBar(Strings::statMp, s.m_mp, m_statMax.m_mp, col1, y0 + pitch, barW);
    drawStatBar(Strings::statAttack, s.m_a, m_statMax.m_a, col1, y0 + pitch * 2, barW);
    drawStatBar(Strings::statMagic, s.m_ma, m_statMax.m_ma, col1, y0 + pitch * 3, barW);
    drawStatBar(Strings::statDefense, s.m_d, m_statMax.m_d, col2, y0, barW);
    drawStatBar(Strings::statMagicDefense, s.m_md, m_statMax.m_md, col2, y0 + pitch, barW);
    drawStatBar(Strings::statSpeed, s.m_spd, m_statMax.m_spd, col2, y0 + pitch * 2, barW);

    // --- Ky nang: race/class source tags ---
    const float sx = panel.x + 880.0f;
    UiText::draw(Strings::skillMenuTitle, static_cast<int>(sx), static_cast<int>(panel.y + 18), 18, COL_TITLE);
    int line = 0;
    auto drawSkills = [&](const std::vector<std::string>& names, const char* tag, Color tagColor) {
        for (const std::string& name : names) {
            if (line >= MAX_SKILL_LINES) return;
            const float y = panel.y + 48.0f + static_cast<float>(line) * 36.0f;
            UiShapes::roundedRect(Rectangle{ sx, y + 1.0f, 40.0f, 22.0f },
                                  UiTheme::radiusChip, Color{ tagColor.r, tagColor.g, tagColor.b, 55 },
                                  tagColor, 1.0f);
            drawCenteredText(tag, sx + 20.0f, y + 4.0f, 14, tagColor, 0.3f, UiFontRole::HUD);
            const std::string skillName = Strings::localizedName(name);
            UiText::drawEllipsis(skillName, static_cast<int>(sx + 50.0f), static_cast<int>(y + 3.0f),
                                 270, 17, COL_TEXT, UiFontRole::HUD);
            ++line;
        }
    };
    if (p.m_raceIndex < m_races.size())   drawSkills(m_races[p.m_raceIndex].m_skillNames,   Strings::raceLabel, Color{ 120, 230, 160, 255 });
    if (p.m_classIndex < m_classes.size()) drawSkills(m_classes[p.m_classIndex].m_skillNames, Strings::classLabel, Color{ 120, 170, 255, 255 });
}

void HeroCreateScreen::drawButtons() const {
    const char* const labels[BTN_COUNT] = { Strings::back, Strings::confirm };
    for (int b = 0; b < BTN_COUNT; ++b) {
        const Rectangle r = buttonRect(b);
        const bool hover = (m_hoverButton == b);
        const bool primary = (b == BTN_CONFIRM);
        Rectangle body = r;
        if (hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) body.y += 2.0f;
        UiShapeEffects buttonEffects;
        buttonEffects.gradientBottom = primary ? UiTheme::primaryBottom : UiTheme::panelBottom;
        buttonEffects.glow = UiTheme::ember;
        buttonEffects.glowRadius = hover ? 5.0f : 0.0f;
        buttonEffects.glowStrength = hover ? 0.4f : 0.0f;
        UiShapes::roundedRect(body, UiTheme::radiusSmall,
            primary ? UiTheme::primaryTop : UiTheme::panel,
            primary || hover ? COL_EDGE_FOCUS : COL_EDGE, 1.0f, buttonEffects);
        drawCenteredText(labels[b], body.x + body.width / 2.0f, body.y + 14.0f, 24,
                         primary ? UiTheme::primaryText : COL_TEXT, 24.0f * 0.04f, UiFontRole::HUD);
    }
}

void HeroCreateScreen::draw() const {
    drawBackground();
    drawCenteredText(m_slotLabel.c_str(), 640.0f, 28.0f, 44, COL_TITLE);
    if (!hasOptions()) {
        drawCenteredText(Strings::noHeroData, 640.0f, 340.0f, 22, COL_DIM);
        drawButtons();
        return;
    }
    drawCards();
    drawDetail();
    drawButtons();
}
