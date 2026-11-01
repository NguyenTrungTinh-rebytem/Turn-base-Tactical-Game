// src/graphics/PrepMenuScreen.cpp
#include "PrepMenuScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiInput.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "SkillTooltip.h"
#include "graphics/ItemIcons.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float TAB_W = 250.0f, TAB_H = 48.0f, TAB_GAP = 12.0f, TAB_Y = 90.0f;
constexpr float PANEL_Y = 138.0f, PANEL_H = 482.0f, PANEL_W = 1036.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr size_t MAX_SKILL_ROWS = 6;
constexpr int    ITEM_SLOTS = 8;

// ---- Mau (dong bo voi WaveSelectScreen) ----
constexpr Color COL_TITLE    = UiTheme::accent;
constexpr Color COL_PANEL    = UiTheme::panel;
constexpr Color COL_EDGE     = UiTheme::border;
constexpr Color COL_EDGE_SEL = UiTheme::ember;
constexpr Color COL_LABEL    = UiTheme::ash;
constexpr Color COL_TEXT     = UiTheme::text;
constexpr Color COL_DIM      = UiTheme::ash;
constexpr Color COL_ERR      = UiTheme::danger;
constexpr Color COL_OK       = UiTheme::verdigris;
constexpr Color COL_BOSS     = UiTheme::danger;

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return UiTheme::kim;
        case Element::MOC:  return UiTheme::moc;
        case Element::THUY: return UiTheme::thuy;
        case Element::HOA:  return UiTheme::hoa;
        case Element::THO:  return UiTheme::tho;
    }
    return WHITE;
}
const char* elementName(Element e) {
    switch (e) {
        case Element::KIM:  return Strings::elements[0];
        case Element::MOC:  return Strings::elements[1];
        case Element::THUY: return Strings::elements[2];
        case Element::HOA:  return Strings::elements[3];
        case Element::THO:  return Strings::elements[4];
    }
    return "?";
}
SkillTipInfo toTipInfo(const PrepSkillInfo& s) {
    SkillTipInfo t;
    t.m_name = Strings::localizedName(s.m_name);
    t.m_element = s.m_element;
    t.m_typeText = s.m_typeText;
    t.m_power = s.m_power;
    t.m_mpCost = s.m_mpCost;
    t.m_effectText = s.m_effectText;
    t.m_isMagic = s.m_isMagic;
    t.m_isHeal = (s.m_typeText.find(Strings::skillTypeHeal) != std::string::npos);
    return t;
}
const char* tabLabel(int i) {
    static const char* labels[] = { Strings::formation, Strings::skillMenuTitle,
                                    Strings::itemMenuTitle, "THÔNG TIN CỔNG" };
    return labels[i];
}
const char* tabIcon(int i) {
    static const char* icons[] = { "action:crown", "action:skill", "action:item", "action:target" };
    return icons[i];
}

void drawCenteredText(const char* text, float cx, float y, int size, Color c,
                      float letterSpacing = 0.0f, UiFontRole role = UiFontRole::AUTO) {
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size, role, letterSpacing) / 2.0f),
                 static_cast<int>(y), size, c, role, letterSpacing);
}
void drawRightText(const char* text, float right, float y, int size, Color c) {
    UiText::draw(text, static_cast<int>(right - UiText::measure(text, size)), static_cast<int>(y), size, c);
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        const size_t nl = s.find('\n', start);
        out.push_back(s.substr(start, nl == std::string::npos ? std::string::npos : nl - start));
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return out;
}

Rectangle partyCardRect(Rectangle area, int index) {
    constexpr float gap = 24.0f, pad = 24.0f;
    const float w = (area.width - 2.0f * pad - 2.0f * gap) / 3.0f;
    return Rectangle{area.x + pad + index * (w + gap), area.y + pad, w, area.height - 2.0f * pad};
}
void drawStatRow(const char* label, int value, float x, float y, float w) {
    UiText::draw(label, static_cast<int>(x), static_cast<int>(y), 18, COL_LABEL);
    drawRightText(TextFormat("%d", value), x + w, y, 18, COL_TEXT);
}
} // namespace

PrepMenuScreen::PrepMenuScreen() = default;
PrepMenuScreen::~PrepMenuScreen() { unloadAll(); }

// ============ Tai nguyen ============
bool PrepMenuScreen::loadBackground(const std::string& path) {
    unloadAll();
    if (path.empty() || !FileExists(path.c_str())) return false;
    m_background = LoadTexture(path.c_str());
    m_hasBackground = (m_background.id != 0);
    if (!m_hasBackground)
        TraceLog(LOG_WARNING, "PrepMenuScreen: khong doc duoc anh nen '%s'.", path.c_str());
    else
        SetTextureFilter(m_background, TEXTURE_FILTER_BILINEAR);
    return m_hasBackground;
}

void PrepMenuScreen::unloadAll() {
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

// ============ Du lieu ============
void PrepMenuScreen::setInfo(const PrepInfo& info) { m_info = info; }

void PrepMenuScreen::reset(int tab) {
    m_tab = std::max(0, std::min(tab, static_cast<int>(TAB_COUNT) - 1));
    m_hoverTab = -1;
    m_hoverButton = -1;
}

// ============ Bo cuc ============
Rectangle PrepMenuScreen::tabRect(int index) const {
    const float W = static_cast<float>(UiCanvas::width());
    const float total = TAB_COUNT * TAB_W + (TAB_COUNT - 1) * TAB_GAP;
    return { (W - total) / 2.0f + static_cast<float>(index) * (TAB_W + TAB_GAP), TAB_Y, TAB_W, TAB_H };
}

Rectangle PrepMenuScreen::contentRect() const {
    const float W = static_cast<float>(UiCanvas::width());
    return { (W - PANEL_W) / 2.0f, PANEL_Y, PANEL_W, PANEL_H };
}

Rectangle PrepMenuScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(UiCanvas::width()) - total) / 2.0f;
    return { startX + index * (BTN_W + BTN_GAP), BTN_Y, BTN_W, BTN_H };
}

// ============ Input ============
void PrepMenuScreen::selectTab(int delta) {
    m_tab = (m_tab + delta + TAB_COUNT) % TAB_COUNT;   // quay vong
}

PrepMenuAction PrepMenuScreen::handleInput() {
    if (UiInput::rightClicked()) {
        UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
        return PrepMenuAction::BACK;
    }
    // ---- Ban phim ----
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_Q)) selectTab(-1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_TAB)) selectTab(+1);
    if (IsKeyPressed(KEY_ONE))   m_tab = TAB_PARTY;
    if (IsKeyPressed(KEY_TWO))   m_tab = TAB_SKILLS;
    if (IsKeyPressed(KEY_THREE)) m_tab = TAB_ITEMS;
    if (IsKeyPressed(KEY_FOUR))  m_tab = TAB_GATE;
    if (m_tab == TAB_PARTY) {
        if (IsKeyPressed(KEY_F1)) return PrepMenuAction::EDIT_MEMBER_1;
        if (IsKeyPressed(KEY_F2)) return PrepMenuAction::EDIT_MEMBER_2;
        if (IsKeyPressed(KEY_F3)) return PrepMenuAction::EDIT_MEMBER_3;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) return PrepMenuAction::START;

    // ---- Chuot ----
    const Vector2 mouse = UiInput::mousePosition();
    m_hoverTab = -1;
    for (int i = 0; i < TAB_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, tabRect(i))) {
            m_hoverTab = i;
            UiInput::setCursorType(UiInput::CursorType::POINTER);
        }
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) {
            m_hoverButton = i;
            UiInput::setCursorType(UiInput::CursorType::POINTER);
        }

    if (UiInput::pressed(MOUSE_BUTTON_LEFT)) {
        if (m_tab == TAB_PARTY) {
            for (int i = 0; i < 3; ++i)
                if (CheckCollisionPointRec(mouse, partyCardRect(contentRect(), i))) {
                    UiInput::consumeClick();
                    return static_cast<PrepMenuAction>(static_cast<int>(PrepMenuAction::EDIT_MEMBER_1) + i);
                }
        }
        if (m_hoverTab >= 0) {
            UiInput::consumeClick();
            m_tab = m_hoverTab;
        }
        if (m_hoverButton == BTN_BACK) {
            UiInput::consumeClick();
            return PrepMenuAction::BACK;
        }
        if (m_hoverButton == BTN_START) {
            UiInput::consumeClick();
            return PrepMenuAction::START;
        }
    }
    return PrepMenuAction::NONE;
}

void PrepMenuScreen::update(float deltaTime) { m_time += deltaTime; }

// ============ Ve ============
void PrepMenuScreen::draw() const {
    m_hoverSkill = -1;
    drawBackground();
    const float cx = UiCanvas::width() / 2.0f;
    drawCenteredText(Strings::preparationTitle, cx, 26.0f, 40, COL_TITLE);
    if (!m_info.m_gate.m_title.empty()) {
        const std::string heroName = Strings::localizedName(m_info.m_hero.m_name);
        drawCenteredText(TextFormat(Strings::preparationHero, m_info.m_gate.m_title.c_str(), heroName.c_str()),
                         cx, 66.0f, 16, COL_LABEL);
    }

    drawTabs();

    const Rectangle area = contentRect();
    UiShapeEffects panelEffects;
    panelEffects.gradientBottom = UiTheme::panelBottom;
    panelEffects.shadowBlur = 8.0f;
    UiShapes::roundedRect(area, UiTheme::radiusLarge, COL_PANEL, COL_EDGE, 1.0f, panelEffects);

    switch (m_tab) {
        case TAB_PARTY:  drawPartyTab(area);  break;
        case TAB_SKILLS: drawSkillsTab(area); break;
        case TAB_ITEMS:  drawItemsTab(area);  break;
        case TAB_GATE:   drawGateTab(area);   break;
        default: break;
    }

    drawButtons();
}

void PrepMenuScreen::drawTooltip() const {
    if (m_tab == TAB_SKILLS && m_hoverSkill >= 0 &&
        static_cast<size_t>(m_hoverSkill) < m_info.m_skills.size()) {
        const Rectangle area = contentRect();
        const Rectangle skillAnchor{
            area.x + 24.0f,
            area.y + 56.0f + static_cast<float>(m_hoverSkill) * 74.0f,
            area.width - 48.0f,
            64.0f
        };
        SkillTooltip::draw(toTipInfo(m_info.m_skills[static_cast<size_t>(m_hoverSkill)]),
                           UiCanvas::mousePosition(), &m_info.m_hero.m_stats, skillAnchor);
    }
}

void PrepMenuScreen::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(W), static_cast<float>(H) });
    } else {
        DrawRectangleGradientV(0, 0, W, H, UiTheme::obsidian, UiTheme::panelBottom);
    }
    DrawRectangle(0, 0, W, H, Color{ 12, 10, 18, 135 });
}

void PrepMenuScreen::drawTabs() const {
    for (int i = 0; i < TAB_COUNT; ++i) {
        const Rectangle r = tabRect(i);
        const bool active = (i == m_tab);
        const bool hover = (i == m_hoverTab);
        UiShapeEffects effects;
        effects.flatBottom = true;
        effects.gradientBottom = active ? UiTheme::primaryBottom : UiTheme::panelBottom;
        UiShapes::roundedRect(r, UiTheme::radiusSmall,
            active ? UiTheme::primaryTop : UiTheme::panel,
            active || hover ? UiTheme::accent : COL_EDGE, 1.0f, effects);
        if (active)
            UiShapes::roundedRect(Rectangle{ r.x + 8.0f, r.y + r.height - 2.0f,
                                             r.width - 16.0f, 2.0f },
                                  UiTheme::radiusBar, UiTheme::ember);
        UiIcons::draw(tabIcon(i), Rectangle{ r.x + 18.0f, r.y + 12.0f, 24.0f, 24.0f });
        drawCenteredText(tabLabel(i), r.x + r.width / 2.0f + 12.0f,
                         r.y + (r.height - 20.0f) / 2.0f, 20,
                         active ? COL_TITLE : (hover ? COL_TEXT : COL_LABEL));
    }
}

// ---------------- Tab 1: DOI HINH ----------------
void PrepMenuScreen::drawPartyTab(const Rectangle& area) const {
    for (int i = 0; i < 3; ++i) {
        const Rectangle r = partyCardRect(area, i);
        const PrepHeroInfo* h = (static_cast<std::size_t>(i) < m_info.m_members.size())
            ? &m_info.m_members[static_cast<std::size_t>(i)]
            : (i == 0 ? &m_info.m_hero : nullptr);
        const bool hovered = CheckCollisionPointRec(UiCanvas::mousePosition(), r);
        const float cx = r.x + r.width / 2.0f;
        if (h) {
            UiShapeEffects cardEffects;
            cardEffects.gradientBottom = UiTheme::panelBottom;
            UiShapes::roundedRect(r, UiTheme::radiusCard, UiTheme::panel,
                                  hovered ? COL_EDGE_SEL : COL_EDGE, 1.0f, cardEffects);
            UiShapeEffects headerEffects;
            headerEffects.flatBottom = true;
            headerEffects.gradientBottom = UiTheme::panelBottom;
            UiShapes::roundedRect(Rectangle{ r.x, r.y, r.width, 54.0f },
                                  UiTheme::radiusCard, UiTheme::panelBottom,
                                  BLANK, 0.0f, headerEffects);
            const std::string heroName = Strings::localizedName(h->m_name);
            const std::string race = Strings::localizedName(h->m_race);
            const std::string heroClass = Strings::localizedName(h->m_class);
            drawCenteredText(heroName.c_str(), cx, r.y + 14.0f, 24, COL_TEXT);
            drawCenteredText(TextFormat("%s %s", race.c_str(), heroClass.c_str()), cx, r.y + 68.0f, 17, COL_LABEL);
            UiElementSigil::draw(h->m_element, Rectangle{ cx - 40.0f, r.y + 96.0f, 24.0f, 24.0f });
            UiText::draw(TextFormat(Strings::elementPrefix, elementName(h->m_element)), static_cast<int>(cx - 10), static_cast<int>(r.y + 97), 18, elementColor(h->m_element));
            float y = r.y + 140.0f;
            const float x = r.x + 22.0f, w = r.width - 44.0f;
            drawStatRow(Strings::statHp, h->m_stats.m_hp, x, y, w); y += 30.0f;
            drawStatRow(Strings::statMp, h->m_stats.m_mp, x, y, w); y += 30.0f;
            drawStatRow(Strings::statAttack, h->m_stats.m_a, x, y, w); y += 30.0f;
            drawStatRow(Strings::statMagic, h->m_stats.m_ma, x, y, w); y += 30.0f;
            drawStatRow(Strings::statDefense, h->m_stats.m_d, x, y, w); y += 30.0f;
            drawStatRow(Strings::statMagicDefense, h->m_stats.m_md, x, y, w); y += 30.0f;
            drawStatRow(Strings::statSpeed, h->m_stats.m_spd, x, y, w);
            drawCenteredText(i == 0 ? Strings::partyLeaderChange : Strings::partyMemberChange,
                             cx, r.y + r.height - 29.0f, 16, COL_OK);
        } else {
            UiShapes::roundedRect(r, UiTheme::radiusCard, UiTheme::panel);
            const float cy = r.y + r.height / 2.0f;
            DrawRectangle(static_cast<int>(cx - 3), static_cast<int>(cy - 28), 6, 56, COL_DIM);
            DrawRectangle(static_cast<int>(cx - 28), static_cast<int>(cy - 3), 56, 6, COL_DIM);
            drawCenteredText(TextFormat(Strings::partyMember, i + 1), cx, cy + 46.0f, 20, COL_LABEL);
        }
        if (!h)
            UiShapes::roundedRect(r, UiTheme::radiusCard, BLANK, COL_DIM, 1.0f);
    }
}

// ---------------- Tab 2: KY NANG ----------------
void PrepMenuScreen::drawSkillsTab(const Rectangle& area) const {
    UiText::draw(Strings::heroSkills, static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
    drawRightText(Strings::statMp, area.x + area.width - 250.0f, area.y + 24.0f, 16, COL_DIM);
    drawRightText(Strings::damageLabel, area.x + area.width - 120.0f, area.y + 24.0f, 16, COL_DIM);

    if (m_info.m_skills.empty()) {
        drawCenteredText(Strings::noSkill, area.x + area.width / 2.0f, area.y + area.height / 2.0f, 22, COL_DIM);
        return;
    }

    const float rowH = 64.0f, rowGap = 10.0f;
    float y = area.y + 56.0f;
    for (size_t i = 0; i < m_info.m_skills.size() && i < MAX_SKILL_ROWS; ++i) {
        const PrepSkillInfo& s = m_info.m_skills[i];
        const Rectangle r = { area.x + 24.0f, y, area.width - 48.0f, rowH };
        const bool rowHover = CheckCollisionPointRec(UiCanvas::mousePosition(), r);
        if (rowHover) m_hoverSkill = static_cast<int>(i);
        UiShapeEffects rowEffects;
        rowEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(r, UiTheme::radiusSmall, UiTheme::panel,
                              rowHover ? COL_EDGE_SEL : COL_EDGE, 1.0f, rowEffects);
        UiElementSigil::draw(s.m_element, Rectangle{ r.x + 6.0f, r.y + 14.0f, 30.0f, 30.0f });

        const std::string skillName = Strings::localizedName(s.m_name);
        UiText::draw(skillName.c_str(), static_cast<int>(r.x + 44), static_cast<int>(r.y + 10), 22, COL_TEXT);
        std::string sub = s.m_typeText;
        if (!s.m_effectText.empty()) sub += "  |  " + s.m_effectText;
        UiText::draw(sub.c_str(), static_cast<int>(r.x + 44), static_cast<int>(r.y + 38), 16, COL_LABEL);

        UiText::draw(elementName(s.m_element), static_cast<int>(r.x + r.width - 410), static_cast<int>(r.y + 20), 18, elementColor(s.m_element));
        drawRightText(TextFormat("%d", s.m_mpCost), r.x + r.width - 226.0f, r.y + 20.0f, 20, UiTheme::mpTop);
        drawRightText(s.m_power > 0 ? TextFormat("%d%%", s.m_power) : "-", r.x + r.width - 24.0f, r.y + 20.0f, 20, COL_TEXT);
        y += rowH + rowGap;
    }
}

// ---------------- Tab 3: TUI DO ----------------
void PrepMenuScreen::drawItemsTab(const Rectangle& area) const {
    UiText::draw(Strings::inventoryTitle, static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);

    const int cols = 4;
    const float pad = 28.0f, gap = 18.0f, top = 64.0f;
    const float slotW = (area.width - 2.0f * pad - (cols - 1) * gap) / cols;
    const float slotH = 150.0f;

    for (int i = 0; i < ITEM_SLOTS; ++i) {
        const int col = i % cols, row = i / cols;
        const Rectangle r = { area.x + pad + col * (slotW + gap), area.y + top + row * (slotH + gap), slotW, slotH };
        const bool filled = static_cast<size_t>(i) < m_info.m_items.size();
        UiShapeEffects slotEffects;
        slotEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(r, UiTheme::radiusCard,
            filled ? UiTheme::panel : UiTheme::obsidian,
            filled ? COL_EDGE : UiTheme::disabled, 1.0f, slotEffects);
        if (filled) {
            const PrepItemInfo& it = m_info.m_items[static_cast<size_t>(i)];
            const float cx = r.x + r.width / 2.0f;
            if (m_icons && !it.m_id.empty()) {
                m_icons->draw(it.m_id, Rectangle{ cx - 32.0f, r.y + 14.0f, 64.0f, 64.0f }, WHITE);
            } else {
                UiShapes::roundedRect(Rectangle{ cx - 24.0f, r.y + 28.0f, 48.0f, 48.0f },
                                      UiTheme::radiusCard, UiTheme::danger);
                DrawRectangle(static_cast<int>(cx - 7), static_cast<int>(r.y + 22), 14, 12, Color{ 200, 190, 220, 255 });
            }
            drawCenteredText(it.m_name.c_str(), cx, r.y + 92.0f, 20, COL_TEXT);
            drawCenteredText(TextFormat("x%d", it.m_count), cx, r.y + 118.0f, 18, COL_OK);
        } else {
            drawCenteredText(Strings::emptySlot, r.x + r.width / 2.0f, r.y + r.height / 2.0f - 9.0f, 18, COL_DIM);
        }
    }
}

// ---------------- Tab 4: THONG TIN CONG ----------------
void PrepMenuScreen::drawGateTab(const Rectangle& area) const {
    const PrepGateInfo& g = m_info.m_gate;
    const float pad = 28.0f;
    const float leftX = area.x + pad, leftW = 430.0f;
    const float rightX = area.x + pad + leftW + 36.0f, rightW = area.x + area.width - pad - rightX;

    // ---- Cot trai: cong + thoi han ----
    UiText::draw(g.m_title.empty() ? Strings::gateLabel : g.m_title.c_str(), static_cast<int>(leftX), static_cast<int>(area.y + 22), 34, COL_TITLE);
    float y = area.y + 72.0f;
    for (const std::string& line : splitLines(g.m_description)) {
        UiText::draw(line.c_str(), static_cast<int>(leftX), static_cast<int>(y), 18, COL_LABEL);
        y += 26.0f;
    }

    // Thoi han (dong ho cong)
    const Rectangle clockBox = { leftX, area.y + 170.0f, leftW, 120.0f };
    const bool hasDeadline = g.m_deadlineTurns >= 0;
    UiShapes::roundedRect(clockBox, UiTheme::radiusCard, UiTheme::panel,
                          hasDeadline ? COL_EDGE_SEL : COL_EDGE, 1.0f);
    UiText::draw(Strings::turnLimit, static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 16), 16, COL_DIM);
    if (hasDeadline) {
        UiText::draw(TextFormat("%d", g.m_deadlineTurns), static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 44), 56, COL_TITLE);
        UiText::draw(Strings::turnsToDefeatGate, static_cast<int>(clockBox.x + 100 + (g.m_deadlineTurns >= 100 ? 36 : 0)),
                 static_cast<int>(clockBox.y + 66), 18, COL_TEXT);
    } else {
        UiText::draw(Strings::unlimited, static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 52), 30, COL_OK);
    }

    // Goi y khac he
    const Rectangle hintBox = { leftX, area.y + 310.0f, leftW, 150.0f };
    UiShapes::roundedRect(hintBox, UiTheme::radiusCard, UiTheme::panel, COL_EDGE, 1.0f);
    UiText::draw(Strings::elementCycleTitle, static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 14), 16, COL_DIM);
    const Element ring[] = { Element::KIM, Element::MOC, Element::THO, Element::THUY, Element::HOA };
    float hx = hintBox.x + 20.0f;
    for (int i = 0; i < 5; ++i) {
        UiElementSigil::draw(ring[i], Rectangle{ hx, hintBox.y + 48.0f, 20.0f, 20.0f });
        UiText::draw(elementName(ring[i]), static_cast<int>(hx + 24), static_cast<int>(hintBox.y + 49), 16, elementColor(ring[i]));
        hx += 78.0f;
    }
    UiText::draw(Strings::elementAdvantage, static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 84), 16, COL_OK);
    UiText::draw(Strings::elementDisadvantage, static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 108), 16, COL_ERR);
    UiText::draw(Strings::elementCycle, static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 128), 16, COL_DIM);

    // ---- Cot phai: boss + quai ----
    float ry = area.y + 22.0f;
    if (g.m_hasBoss) {
        const Rectangle boss = { rightX, ry, rightW, 130.0f };
        UiShapes::roundedRect(boss, UiTheme::radiusCard, UiTheme::dangerBottom, COL_BOSS, 1.0f);
        UiShapes::roundedRect(Rectangle{ boss.x + 18, boss.y + 14, 68, 22 },
                              UiTheme::radiusChip, COL_BOSS);
        drawCenteredText(Strings::boss, boss.x + 18 + 34, boss.y + 17, 16, WHITE);
        UiText::draw(g.m_bossName.c_str(), static_cast<int>(boss.x + 100), static_cast<int>(boss.y + 12), 28, COL_TEXT);
        UiElementSigil::draw(g.m_bossElement, Rectangle{ boss.x + 18, boss.y + 66, 24, 24 });
        UiText::draw(TextFormat(Strings::elementPrefix, elementName(g.m_bossElement)), static_cast<int>(boss.x + 50), static_cast<int>(boss.y + 66), 18, elementColor(g.m_bossElement));
        UiText::draw(TextFormat(Strings::healthTemplate, g.m_bossHp), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 94), 18, COL_TEXT);
        ry += 130.0f + 20.0f;
    }

    UiText::draw(g.m_hasBoss ? Strings::bossGuards : Strings::availableEnemies,
                 static_cast<int>(rightX), static_cast<int>(ry), 16, COL_TITLE);
    ry += 28.0f;
    size_t shown = 0;
    for (const PrepEnemyInfo& e : g.m_enemies) {
        if (ry + 26.0f > area.y + area.height - 16.0f) break;
        DrawCircle(static_cast<int>(rightX + 10), static_cast<int>(ry + 11), 6.0f, elementColor(e.m_element));
        UiText::draw(e.m_name.c_str(), static_cast<int>(rightX + 28), static_cast<int>(ry), 20, COL_TEXT);
        drawRightText(elementName(e.m_element), rightX + rightW, ry + 3.0f, 16, elementColor(e.m_element));
        ry += 30.0f;
        ++shown;
    }
    if (g.m_enemies.empty())
        UiText::draw(Strings::noData, static_cast<int>(rightX), static_cast<int>(ry), 18, COL_DIM);
    else if (shown < g.m_enemies.size())
        UiText::draw(TextFormat(Strings::additionalEnemiesTemplate, static_cast<int>(g.m_enemies.size() - shown)), static_cast<int>(rightX + 28), static_cast<int>(ry), 16, COL_DIM);
}

void PrepMenuScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::back, Strings::enterBattle };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        Rectangle body = r;
        if (hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) body.y += 2.0f;
        UiShapeEffects effects;
        effects.gradientBottom = primary ? UiTheme::primaryBottom : UiTheme::panelBottom;
        effects.shadowBlur = 6.0f;
        effects.glow = UiTheme::ember;
        effects.glowRadius = hover ? 5.0f : 0.0f;
        effects.glowStrength = hover ? 0.4f : 0.0f;
        UiShapes::roundedRect(body, UiTheme::radiusSmall,
            primary ? UiTheme::primaryTop : UiTheme::panel,
            hover || primary ? COL_EDGE_SEL : COL_EDGE, 1.0f, effects);
        drawCenteredText(labels[i], body.x + body.width / 2.0f,
                         body.y + (body.height - 24.0f) / 2.0f, 24,
                         primary ? UiTheme::primaryText : COL_TEXT, 24.0f * 0.04f, UiFontRole::HUD);
    }
}