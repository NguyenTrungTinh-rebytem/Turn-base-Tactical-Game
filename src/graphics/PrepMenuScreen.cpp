// src/graphics/PrepMenuScreen.cpp
#include "PrepMenuScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "SkillTooltip.h"
#include "graphics/ItemIcons.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiWidgets.h"
#include "graphics/UiTheme.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float TAB_W = 250.0f, TAB_H = 46.0f, TAB_GAP = 12.0f, TAB_Y = 92.0f;
constexpr float PANEL_Y = 138.0f, PANEL_H = 482.0f, PANEL_W = 1036.0f;
constexpr float BTN_W = 210.0f, BTN_H = 48.0f, BTN_GAP = 24.0f, BTN_Y = 642.0f;
constexpr size_t MAX_SKILL_ROWS = 6;
constexpr int    ITEM_SLOTS = 8;

// ---- Mau (dong bo voi WaveSelectScreen) ----
constexpr Color COL_TITLE = UiTheme::silver;
constexpr Color COL_PANEL = UiTheme::panel;
constexpr Color COL_EDGE = UiTheme::ash;
constexpr Color COL_EDGE_SEL = UiTheme::crimson;
constexpr Color COL_LABEL = UiTheme::ashText;
constexpr Color COL_TEXT = UiTheme::silver;
constexpr Color COL_DIM = UiTheme::ashText;
constexpr Color COL_ERR = UiTheme::danger;
constexpr Color COL_OK = UiTheme::heal;
constexpr Color COL_BOSS = UiTheme::crimson;

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return Color{ 215, 215, 230, 255 };
        case Element::MOC:  return Color{ 100, 210, 100, 255 };
        case Element::THUY: return Color{ 90, 160, 255, 255 };
        case Element::HOA:  return Color{ 255, 110, 80, 255 };
        case Element::THO:  return Color{ 205, 160, 90, 255 };
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
    t.m_isHeal = (s.m_typeText.find(u8"Hồi") != std::string::npos);
    return t;
}
const char* tabLabel(int i) {
    static const char* labels[] = { "ĐỘI HÌNH", "KỸ NĂNG", "TÚI ĐỒ", "MÀN CHƠI" };
    return labels[i];
}

void drawCenteredText(const char* text, float cx, float y, int size, Color c) {
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size) / 2.0f), static_cast<int>(y), size, c);
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
void drawStatRow(const char* label, int value, int maximum, float x, float y, float w) {
    UiText::draw(label, static_cast<int>(x), static_cast<int>(y), 15, COL_LABEL);
    const float barX = x + 48.0f;
    const float barWidth = std::max(18.0f, w - 86.0f);
    const Rectangle bar{ barX, y + 5.0f, barWidth, 10.0f };
    const float fraction = std::max(0.0f, std::min(1.0f,
        static_cast<float>(value) / static_cast<float>(std::max(1, maximum))));
    UiShapes::runeBar(bar, fraction, UiTheme::hpHigh);
    drawRightText(TextFormat("%d", value), x + w, y - 4.0f, 22, COL_TEXT);
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
    unloadPortraits();
    if (m_hasBackground) {
        if (IsWindowReady()) UnloadTexture(m_background);
        m_hasBackground = false;
    }
    m_background = Texture2D{};
}

void PrepMenuScreen::unloadPortraits() {
    for (Texture2D& portrait : m_memberPortraits) {
        if (portrait.id != 0 && IsWindowReady()) UnloadTexture(portrait);
        portrait = Texture2D{};
    }
}

// ============ Du lieu ============
void PrepMenuScreen::setInfo(const PrepInfo& info) {
    unloadPortraits();
    m_info = info;
    if (!IsWindowReady()) return;
    for (int i = 0; i < 3; ++i) {
        const PrepHeroInfo* hero = static_cast<std::size_t>(i) < m_info.m_members.size()
            ? &m_info.m_members[static_cast<std::size_t>(i)]
            : (i == 0 ? &m_info.m_hero : nullptr);
        if (!hero || hero->m_portraitPath.empty() ||
            !FileExists(hero->m_portraitPath.c_str())) continue;
        m_memberPortraits[i] = LoadTexture(hero->m_portraitPath.c_str());
        if (m_memberPortraits[i].id != 0)
            SetTextureFilter(m_memberPortraits[i], TEXTURE_FILTER_POINT);
    }
}

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
    const Vector2 mouse = UiCanvas::mousePosition();
    m_hoverTab = -1;
    for (int i = 0; i < TAB_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, tabRect(i))) m_hoverTab = i;
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (m_tab == TAB_PARTY) {
            for (int i = 0; i < 3; ++i)
                if (CheckCollisionPointRec(mouse, partyCardRect(contentRect(), i)))
                    return static_cast<PrepMenuAction>(static_cast<int>(PrepMenuAction::EDIT_MEMBER_1) + i);
        }
        if (m_hoverTab >= 0) m_tab = m_hoverTab;
        if (m_hoverButton == BTN_BACK)  return PrepMenuAction::BACK;
        if (m_hoverButton == BTN_SHOP)  return PrepMenuAction::SHOP;
        if (m_hoverButton == BTN_START) return PrepMenuAction::START;
    }
    return PrepMenuAction::NONE;
}

void PrepMenuScreen::update(float deltaTime) { m_time += deltaTime; }

// ============ Ve ============
void PrepMenuScreen::draw() const {
    m_hoverSkill = -1;
    m_hoverElement = -1;
    drawBackground();
    const float cx = UiCanvas::width() / 2.0f;
    drawCenteredText(u8"CHUẨN BỊ TRẬN ĐẤU", cx, 26.0f, 40, COL_TITLE);
    if (!m_info.m_gate.m_title.empty()) {
        const std::string heroName = Strings::localizedName(m_info.m_hero.m_name);
        drawCenteredText(TextFormat(u8"%s  -  Anh hùng: %s", m_info.m_gate.m_title.c_str(), heroName.c_str()),
                         cx, 66.0f, 16, COL_LABEL);
    }

    drawTabs();

    const Rectangle area = contentRect();
    UiShapes::plate(Rectangle{ area.x + 5, area.y + 6, area.width, area.height },
                    Color{ 0, 0, 0, 120 }, BLANK);
    UiShapes::plate(area, COL_PANEL, COL_EDGE, 1.0f);

    switch (m_tab) {
        case TAB_PARTY:  drawPartyTab(area);  break;
        case TAB_SKILLS: drawSkillsTab(area); break;
        case TAB_ITEMS:  drawItemsTab(area);  break;
        case TAB_GATE:   drawGateTab(area);   break;
        default: break;
    }

    drawButtons();
    // Tooltip ky nang: ve CUOI CUNG de nam tren moi thu
    if (m_tab == TAB_SKILLS && m_hoverSkill >= 0 &&
        static_cast<size_t>(m_hoverSkill) < m_info.m_skills.size()) {
        SkillTooltip::draw(toTipInfo(m_info.m_skills[static_cast<size_t>(m_hoverSkill)]),
                           UiCanvas::mousePosition(), &m_info.m_hero.m_stats);
    }
}

void PrepMenuScreen::drawBackground() const {
    const int W = UiCanvas::width(), H = UiCanvas::height();
    if (m_hasBackground) {
        UiCanvas::drawCover(m_background, Rectangle{ 0.0f, 0.0f,
            static_cast<float>(W), static_cast<float>(H) });
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 25, 16, 23, 255 }, UiTheme::ink);
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 160 });
}

void PrepMenuScreen::drawTabs() const {
    static const char* iconKeys[TAB_COUNT] = {
        "action:party", "action:skill", "action:item", "action:banner"
    };
    for (int i = 0; i < TAB_COUNT; ++i) {
        const Rectangle r = tabRect(i);
        const bool active = (i == m_tab);
        const bool hover = (i == m_hoverTab);
        UiShapes::angledTab(r,
            active ? Color{ 50, 10, 17, 224 } :
                (hover ? UiTheme::panelRaised : UiTheme::panel),
            active ? COL_EDGE_SEL : COL_EDGE, active, hover);
        UiIcons::draw(iconKeys[i], Rectangle{ r.x + 14.0f, r.y + 10.0f, 26.0f, 26.0f },
                      active ? COL_TITLE : COL_LABEL);
        const char* label = tabLabel(i);
        UiText::draw(label, static_cast<int>(r.x + 14.0f + 26.0f + 12.0f),
            static_cast<int>(r.y + (r.height - 17.0f) * 0.5f), 17,
            active ? COL_TITLE : (hover ? COL_TEXT : COL_LABEL));
    }
}

// ---------------- Tab 1: DOI HINH ----------------
void PrepMenuScreen::drawPartyTab(const Rectangle& area) const {
    const auto statMax = [this](int Stats::* field) {
        int maximum = m_info.m_hero.m_stats.*field;
        for (const PrepHeroInfo& member : m_info.m_members)
            maximum = std::max(maximum, member.m_stats.*field);
        return std::max(1, maximum);
    };
    for (int i = 0; i < 3; ++i) {
        const Rectangle r = partyCardRect(area, i);
        const PrepHeroInfo* h = (static_cast<std::size_t>(i) < m_info.m_members.size())
            ? &m_info.m_members[static_cast<std::size_t>(i)]
            : (i == 0 ? &m_info.m_hero : nullptr);
        const bool hovered = CheckCollisionPointRec(UiCanvas::mousePosition(), r);
        const float cx = r.x + r.width / 2.0f;
        if (h) {
            UiShapes::plate(r, UiTheme::panel, COL_EDGE, 1.0f);
            UiShapes::bladeStrip(Rectangle{ r.x, r.y, r.width, 76.0f },
                i == 0 ? Color{ 50, 10, 17, 224 } : UiTheme::panelRaised,
                i == 0 ? UiTheme::crimson : COL_EDGE, false, i == 0);
            const std::string heroName = Strings::localizedName(h->m_name);
            const std::string race = Strings::localizedName(h->m_race);
            const std::string heroClass = Strings::localizedName(h->m_class);
            drawCenteredText(heroName.c_str(), cx, r.y + 9.0f, 22, COL_TEXT);
            drawCenteredText((race + u8" · " + heroClass).c_str(), cx, r.y + 43.0f, 16, COL_LABEL);
            if (i == 0) {
                UiIcons::draw("action:crown",
                    Rectangle{ r.x + 10.0f, r.y + 87.0f, 20.0f, 20.0f }, COL_TITLE);
                UiText::draw(u8"Thủ lĩnh", static_cast<int>(r.x + 36.0f),
                             static_cast<int>(r.y + 88.0f), 16, COL_TITLE);
            }

            const Rectangle portrait{ r.x + 18.0f, r.y + 126.0f, 108.0f, 108.0f };
            const Color tint = elementColor(h->m_element);
            DrawEllipse(static_cast<int>(portrait.x + portrait.width / 2.0f),
                        static_cast<int>(portrait.y + portrait.height - 8.0f),
                        48.0f, 12.0f, Color{ tint.r, tint.g, tint.b, 100 });
            const Texture2D& texture = m_memberPortraits[i];
            if (texture.id != 0 && texture.height > 0) {
                // Sheet hero: o 128x128, nhieu hang/cot -> chi lay o dau tien (idle frame 0)
                const float frame = static_cast<float>(std::min(128, std::min(texture.width, texture.height)));
                DrawTexturePro(texture,
                    Rectangle{ 0.0f, 0.0f, frame, frame }, portrait,
                    Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
            } else {
                UiIcons::draw(std::string("element:") + (static_cast<int>(h->m_element) == 0 ? "kim" :
                    static_cast<int>(h->m_element) == 1 ? "moc" :
                    static_cast<int>(h->m_element) == 2 ? "thuy" :
                    static_cast<int>(h->m_element) == 3 ? "hoa" : "tho"),
                    Rectangle{ portrait.x + 22.0f, portrait.y + 22.0f, 64.0f, 64.0f }, tint);
            }
            UiText::draw(TextFormat(Strings::elementPrefix, elementName(h->m_element)),
                static_cast<int>(portrait.x), static_cast<int>(portrait.y + portrait.height + 8.0f),
                14, tint);

            float y = r.y + 105.0f;
            const float x = r.x + 142.0f, w = r.width - 164.0f;
            drawStatRow("HP", h->m_stats.m_hp, statMax(&Stats::m_hp), x, y, w); y += 36.0f;
            drawStatRow("MP", h->m_stats.m_mp, statMax(&Stats::m_mp), x, y, w); y += 36.0f;
            drawStatRow("ATK", h->m_stats.m_a, statMax(&Stats::m_a), x, y, w); y += 36.0f;
            drawStatRow("MATK", h->m_stats.m_ma, statMax(&Stats::m_ma), x, y, w); y += 36.0f;
            drawStatRow("DEF", h->m_stats.m_d, statMax(&Stats::m_d), x, y, w); y += 36.0f;
            drawStatRow("MDEF", h->m_stats.m_md, statMax(&Stats::m_md), x, y, w); y += 36.0f;
            drawStatRow("SPD", h->m_stats.m_spd, statMax(&Stats::m_spd), x, y, w);
            drawCenteredText(u8"Nhấp để đổi", cx, r.y + r.height - 28.0f, 14, COL_OK);
        } else {
            const float cy = r.y + r.height / 2.0f;
            DrawPoly(Vector2{ cx, cy - 12.0f }, 6, 54.0f, 30.0f, UiTheme::panel);
            DrawPolyLinesEx(Vector2{ cx, cy - 12.0f }, 6, 54.0f, 30.0f, 2.0f, COL_DIM);
            DrawLineEx(Vector2{ cx, cy - 39.0f }, Vector2{ cx, cy + 15.0f }, 2.0f, COL_DIM);
            DrawLineEx(Vector2{ cx - 27.0f, cy - 12.0f },
                       Vector2{ cx + 27.0f, cy - 12.0f }, 2.0f, COL_DIM);
            drawCenteredText(TextFormat(u8"ĐỒNG ĐỘI %d", i + 1), cx, cy + 46.0f, 20, COL_LABEL);
            drawCenteredText(u8"NHẤP ĐỂ CHỌN", cx, cy + 74.0f, 16, COL_OK);
        }
        if (!h)
            UiShapes::plate(r, BLANK, hovered ? COL_EDGE_SEL : COL_DIM,
                            hovered ? 2.0f : 1.0f);
    }
}

// ---------------- Tab 2: KY NANG ----------------
void PrepMenuScreen::drawSkillsTab(const Rectangle& area) const {
    UiText::draw(Strings::heroSkills, static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
    drawRightText("MP", area.x + area.width - 250.0f, area.y + 24.0f, 14, COL_DIM);
    drawRightText(u8"SÁT THƯƠNG", area.x + area.width - 120.0f, area.y + 24.0f, 14, COL_DIM);

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
        UiShapes::bladeStrip(r, rowHover ? UiTheme::panelRaised : UiTheme::panel,
                             rowHover ? COL_EDGE_SEL : COL_EDGE, false, rowHover, rowHover);
        const SkillType skillType = s.m_typeText.find(u8"Hồi") != std::string::npos
            ? SkillType::HEAL : s.m_typeText.find(u8"Tăng") != std::string::npos
            ? SkillType::BUFF : s.m_typeText.find(u8"Giảm") != std::string::npos
            ? SkillType::DEBUFF : SkillType::DAMAGE;
        UiIcons::drawSkill(s.m_element, skillType, s.m_isMagic,
            Rectangle{ r.x + 5.0f, r.y + 18.0f, 28.0f, 28.0f });

        const std::string skillName = Strings::localizedName(s.m_name);
        UiText::draw(skillName.c_str(), static_cast<int>(r.x + 39), static_cast<int>(r.y + 10), 22, COL_TEXT);
        std::string sub = s.m_typeText;
        if (!s.m_effectText.empty()) sub += "  |  " + s.m_effectText;
        UiText::draw(sub.c_str(), static_cast<int>(r.x + 39), static_cast<int>(r.y + 38), 16, COL_LABEL);

        UiText::draw(elementName(s.m_element), static_cast<int>(r.x + r.width - 410), static_cast<int>(r.y + 20), 18, elementColor(s.m_element));
        drawRightText(TextFormat("%d", s.m_mpCost), r.x + r.width - 226.0f, r.y + 20.0f, 20, Color{ 110, 170, 255, 255 });
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
        if (filled) {
            const PrepItemInfo& it = m_info.m_items[static_cast<size_t>(i)];
            UiWidgets::ItemTile(r, "item:" + it.m_id, it.m_name,
                TextFormat("×%d", it.m_count), true,
                [this, &it](Rectangle iconBounds) {
                    if (m_icons && !it.m_id.empty())
                        m_icons->draw(it.m_id, iconBounds, WHITE);
                    else
                        UiIcons::draw("item:" + it.m_id, iconBounds, WHITE);
                });
            UiWidgets::Tooltip(r, it.m_name + TextFormat(" · x%d", it.m_count));
        } else {
            UiShapes::plate(r, UiTheme::panel, COL_EDGE, 1.0f);
            drawCenteredText("Trống", r.x + r.width / 2.0f, r.y + r.height / 2.0f - 9.0f, 18, COL_DIM);
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
    UiText::draw(g.m_title.empty() ? u8"CỔNG" : g.m_title.c_str(), static_cast<int>(leftX), static_cast<int>(area.y + 22), 34, COL_TITLE);
    float y = area.y + 72.0f;
    for (const std::string& line : splitLines(g.m_description)) {
        UiText::draw(line.c_str(), static_cast<int>(leftX), static_cast<int>(y), 18, COL_LABEL);
        y += 26.0f;
    }

    // Thoi han (dong ho cong)
    const Rectangle clockBox = { leftX, area.y + 170.0f, leftW, 120.0f };
    const bool hasDeadline = g.m_deadlineTurns >= 0;
    UiShapes::plate(clockBox, UiTheme::panelRaised,
                    hasDeadline ? COL_EDGE_SEL : COL_EDGE, 1.0f);
    UiText::draw(Strings::turnLimit, static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 16), 16, COL_DIM);
    if (hasDeadline) {
        UiText::draw(TextFormat("%d", g.m_deadlineTurns), static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 44), 56, COL_TITLE);
        UiText::draw(u8"lượt để hạ gác cổng", static_cast<int>(clockBox.x + 100 + (g.m_deadlineTurns >= 100 ? 36 : 0)),
                 static_cast<int>(clockBox.y + 66), 18, COL_TEXT);
    } else {
        UiText::draw(Strings::unlimited, static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 52), 30, COL_OK);
    }

    // Ma trận khắc hệ được lấy trực tiếp từ quy tắc chiến đấu.
    const Rectangle hintBox = { leftX, area.y + 310.0f, leftW, 150.0f };
    UiShapes::plate(hintBox, UiTheme::panelRaised, COL_EDGE, 1.0f);
    UiText::draw(u8"VÒNG NGŨ HÀNH · MA TRẬN SÁT THƯƠNG",
                 static_cast<int>(hintBox.x + 14), static_cast<int>(hintBox.y + 10), 13, COL_DIM);
    const Vector2 center{ hintBox.x + 102.0f, hintBox.y + 85.0f };
    const float ringRadius = 45.0f;
    Vector2 nodes[5]{};
    int hovered = -1;
    const Element ring[] = { Element::KIM, Element::MOC, Element::THO, Element::THUY, Element::HOA };
    const int matrixIndex[] = { 0, 1, 4, 2, 3 };
    for (int i = 0; i < 5; ++i) {
        const float angle = (-90.0f + 72.0f * static_cast<float>(i)) * DEG2RAD;
        nodes[i] = Vector2{ center.x + std::cos(angle) * ringRadius,
                            center.y + std::sin(angle) * ringRadius };
        const Vector2 mouse = UiCanvas::mousePosition();
        if (CheckCollisionPointCircle(mouse, nodes[i], 14.0f)) hovered = i;
    }
    for (int i = 0; i < 5; ++i) {
        const int to = (i + 1) % 5;
        DrawLineEx(nodes[i], nodes[to], 2.0f,
                   hovered == i || hovered == to ? COL_TITLE : Color{ 125, 116, 150, 210 });
    }
    int advantages = 0;
    int disadvantages = 0;
    static const char* elementIcons[] = {
        "element:kim", "element:moc", "element:thuy", "element:hoa", "element:tho"
    };
    for (int i = 0; i < 5; ++i) {
        const bool active = hovered == i;
        DrawCircleV(nodes[i], active ? 15.0f : 12.0f,
                    active ? COL_TITLE : Color{ 24, 22, 46, 255 });
        UiIcons::draw(elementIcons[i],
                      Rectangle{ nodes[i].x - 9.0f, nodes[i].y - 9.0f, 18.0f, 18.0f },
                      elementColor(ring[i]));
        if (active) {
            for (int defender = 0; defender < 5; ++defender) {
                advantages += ELEMENT_MATRIX[matrixIndex[i]][defender] > 1.0f;
                disadvantages += ELEMENT_MATRIX[matrixIndex[i]][defender] < 1.0f;
            }
        }
    }
    m_hoverElement = hovered;
    UiText::draw(TextFormat(u8"Ưu thế %d · Bất lợi %d", advantages, disadvantages),
                 static_cast<int>(hintBox.x + 212.0f), static_cast<int>(hintBox.y + 61.0f),
                 15, hovered >= 0 ? elementColor(ring[hovered]) : COL_LABEL);
    UiText::draw(hovered >= 0 ? elementName(ring[hovered]) : u8"Di chuột lên hệ",
                 static_cast<int>(hintBox.x + 212.0f), static_cast<int>(hintBox.y + 87.0f),
                 17, hovered >= 0 ? elementColor(ring[hovered]) : COL_DIM);

    // ---- Cot phai: boss + quai ----
    float ry = area.y + 22.0f;
    if (g.m_hasBoss) {
        const Rectangle boss = { rightX, ry, rightW, 130.0f };
        UiShapes::plate(boss, Color{ 43, 9, 15, 235 }, COL_BOSS, 2.0f);
        UiShapes::bladeStrip(Rectangle{ boss.x + 18, boss.y + 14, 68, 22 },
                             COL_BOSS, UiTheme::silver, false, true);
        drawCenteredText(u8"BOSS", boss.x + 18 + 34, boss.y + 17, 16, WHITE);
        UiText::draw(g.m_bossName.c_str(), static_cast<int>(boss.x + 100), static_cast<int>(boss.y + 12), 28, COL_TEXT);
        DrawCircle(static_cast<int>(boss.x + 30), static_cast<int>(boss.y + 76), 8.0f, elementColor(g.m_bossElement));
        UiText::draw(TextFormat(Strings::elementPrefix, elementName(g.m_bossElement)), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 66), 18, elementColor(g.m_bossElement));
        UiText::draw(TextFormat("HP %d", g.m_bossHp), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 94), 18, COL_TEXT);
        ry += 130.0f + 20.0f;
    }

    UiText::draw(g.m_hasBoss ? Strings::bossGuards : Strings::availableEnemies,
                 static_cast<int>(rightX), static_cast<int>(ry), 16, COL_TITLE);
    ry += 28.0f;
    size_t shown = 0;
    for (const PrepEnemyInfo& e : g.m_enemies) {
        if (ry + 26.0f > area.y + area.height - 16.0f) break;
        const Rectangle enemyRow{ rightX, ry - 2.0f, rightW, 26.0f };
        const bool hoverEnemy = CheckCollisionPointRec(UiCanvas::mousePosition(), enemyRow);
        UiShapes::bladeStrip(enemyRow,
            hoverEnemy ? UiTheme::panelRaised : UiTheme::panel,
            hoverEnemy ? COL_EDGE_SEL : COL_EDGE, false, hoverEnemy, hoverEnemy);
        static const char* const icons[] = {
            "element:kim", "element:moc", "element:thuy", "element:hoa", "element:tho"
        };
        const int ei = static_cast<int>(e.m_element);
        UiIcons::draw(icons[std::max(0, std::min(ei, 4))],
            Rectangle{ rightX + 6.0f, ry + 1.0f, 20.0f, 20.0f }, elementColor(e.m_element));
        UiText::draw(e.m_name.c_str(), static_cast<int>(rightX + 34), static_cast<int>(ry), 18, COL_TEXT);
        drawRightText(elementName(e.m_element), rightX + rightW, ry + 3.0f, 16, elementColor(e.m_element));
        ry += 30.0f;
        ++shown;
    }
    if (g.m_enemies.empty())
        UiText::draw(Strings::noData, static_cast<int>(rightX), static_cast<int>(ry), 18, COL_DIM);
    else if (shown < g.m_enemies.size())
        UiText::draw(TextFormat(u8"+%d quái khác", static_cast<int>(g.m_enemies.size() - shown)), static_cast<int>(rightX + 28), static_cast<int>(ry), 14, COL_DIM);
}

void PrepMenuScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::back, u8"CỬA HÀNG", u8"VÀO TRẬN" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        const float pulse = primary ? 0.5f + 0.5f * std::sin(m_time * 4.0f) : 0.0f;
        const Color edge = primary
            ? Color{ UiTheme::crimson.r, UiTheme::crimson.g, UiTheme::crimson.b,
                     static_cast<unsigned char>(190 + 65 * pulse) }
            : ((hover || i == BTN_SHOP) ? COL_EDGE_SEL : COL_EDGE);
        UiShapes::bladeStrip(r, primary ? Color{ 50, 10, 17, 224 } : COL_PANEL,
                             edge, false, primary, hover);
        const char* iconKey = i == BTN_SHOP ? "action:coin" :
                              (primary ? "action:start" : "action:back");
        UiIcons::draw(iconKey, Rectangle{ r.x + 20.0f, r.y + 11.0f, 24.0f, 24.0f },
                      hover ? Color{ 255, 240, 170, 255 } : COL_TITLE);
        UiText::draw(labels[i], static_cast<int>(r.x + 52.0f),
                     static_cast<int>(r.y + (r.height - 20.0f) / 2.0f), 20,
                     hover ? Color{ 255, 240, 170, 255 } : COL_TEXT);
    }
}