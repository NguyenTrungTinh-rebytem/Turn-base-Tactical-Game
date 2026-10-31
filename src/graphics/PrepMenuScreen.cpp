#include "graphics/ui/UiInput.h"
#include "graphics/ui/UiFrame.h"
#include "graphics/ui/UiWidgets.h"
#include "graphics/ui/UiTheme.h"
#include "graphics/ui/UiText.h"
#include "graphics/ui/Strings.h"
#include "graphics/ui/UiIcons.h"
// src/graphics/PrepMenuScreen.cpp
#include "PrepMenuScreen.h"
#include "SkillTooltip.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho 1280x720) ----
constexpr float TAB_W = 250.0f, TAB_H = 46.0f, TAB_GAP = 12.0f, TAB_Y = 92.0f;
constexpr float PANEL_Y = 138.0f, PANEL_H = 482.0f, PANEL_W = 1036.0f;
constexpr float BTN_W = 220.0f, BTN_H = 52.0f, BTN_GAP = 30.0f, BTN_Y = 640.0f;
constexpr size_t MAX_SKILL_ROWS = 6;
constexpr int    ITEM_SLOTS = 8;

// ---- Mau (dong bo voi WaveSelectScreen) ----
constexpr Color COL_TITLE = UiTheme::AGED_GOLD;
constexpr Color COL_PANEL    { 28, 25, 40, 255 };
constexpr Color COL_EDGE = UiTheme::SLATE;
constexpr Color COL_EDGE_SEL = UiTheme::AGED_GOLD;
constexpr Color COL_LABEL = UiTheme::MUTED;
constexpr Color COL_TEXT = UiTheme::TEXT;
constexpr Color COL_DIM = UiTheme::DISABLED;
constexpr Color COL_ERR = UiTheme::DANGER;
constexpr Color COL_OK = UiTheme::HP;
constexpr Color COL_BOSS     { 200, 50, 60, 255 };

Color elementColor(Element e) {
    switch (e) {
        case Element::KIM:  return UiTheme::elementColor(Element::KIM);
        case Element::MOC:  return UiTheme::elementColor(Element::MOC);
        case Element::THUY: return UiTheme::elementColor(Element::THUY);
        case Element::HOA:  return UiTheme::elementColor(Element::HOA);
        case Element::THO:  return UiTheme::elementColor(Element::THO);
    }
    return WHITE;
}
const char* elementName(Element e) {
    switch (e) {
        case Element::KIM:  return "Kim";
        case Element::MOC:  return "Mộc";
        case Element::THUY: return "Thủy";
        case Element::HOA:  return "Hỏa";
        case Element::THO:  return "Thổ";
    }
    return "?";
}
SkillTipInfo toTipInfo(const PrepSkillInfo& s) {
    SkillTipInfo t;
    t.m_name = s.m_name;
    t.m_element = s.m_element;
    t.m_typeText = s.m_typeText;
    t.m_power = s.m_power;
    t.m_mpCost = s.m_mpCost;
    t.m_effectText = s.m_effectText;
    t.m_isMagic = s.m_isMagic;
    t.m_isHeal = (s.m_typeText.find("Hoi") != std::string::npos);   // "Hồi máu"
    return t;
}
const char* tabLabel(int i) {
    static const Strings::Key keys[] = {Strings::Key::SQUAD,Strings::Key::SKILLS,
        Strings::Key::INVENTORY,Strings::Key::WAVE_INFO};
    return Strings::get(keys[i]);
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
    const float W = static_cast<float>(UiTheme::WIDTH);
    const float total = TAB_COUNT * TAB_W + (TAB_COUNT - 1) * TAB_GAP;
    return { (W - total) / 2.0f + static_cast<float>(index) * (TAB_W + TAB_GAP), TAB_Y, TAB_W, TAB_H };
}

Rectangle PrepMenuScreen::contentRect() const {
    const float W = static_cast<float>(UiTheme::WIDTH);
    return { (W - PANEL_W) / 2.0f, PANEL_Y, PANEL_W, PANEL_H };
}

Rectangle PrepMenuScreen::buttonRect(int index) const {
    const float total = BTN_COUNT * BTN_W + (BTN_COUNT - 1) * BTN_GAP;
    const float startX = (static_cast<float>(UiTheme::WIDTH) - total) / 2.0f;
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
    const Vector2 mouse = UiInput::instance().mouse();
    m_hoverTab = -1;
    for (int i = 0; i < TAB_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, tabRect(i))) m_hoverTab = i;
    m_hoverButton = -1;
    for (int i = 0; i < BTN_COUNT; ++i)
        if (CheckCollisionPointRec(mouse, buttonRect(i))) m_hoverButton = i;

    if (UiInput::instance().leftPressed()) {
        if (m_tab == TAB_PARTY) {
            for (int i = 0; i < 3; ++i)
                if (UiInput::instance().click(partyCardRect(contentRect(), i)))
                    return static_cast<PrepMenuAction>(static_cast<int>(PrepMenuAction::EDIT_MEMBER_1) + i);
        }
        if (m_hoverTab >= 0 && UiInput::instance().click(tabRect(m_hoverTab))) m_tab = m_hoverTab;
        if (m_hoverButton == BTN_BACK && UiInput::instance().click(buttonRect(BTN_BACK))) return PrepMenuAction::BACK;
        if (m_hoverButton == BTN_START && UiInput::instance().click(buttonRect(BTN_START))) return PrepMenuAction::START;
    }
    return PrepMenuAction::NONE;
}

void PrepMenuScreen::update(float deltaTime) { m_time += deltaTime; }

// ============ Ve ============
void PrepMenuScreen::draw() const {
    m_hoverSkill = -1;
    drawBackground();
    const float cx = UiTheme::WIDTH / 2.0f;
    drawCenteredText("Chuẩn bị trận đấu", cx, 26.0f, 40, COL_TITLE);
    if (!m_info.m_gate.m_title.empty())
        drawCenteredText(TextFormat("%s  -  Anh hùng: %s", m_info.m_gate.m_title.c_str(), m_info.m_hero.m_name.c_str()),
                         cx, 66.0f, 16, COL_LABEL);

    drawTabs();

    const Rectangle area = contentRect();
    UiFrame::panel(area);

    switch (m_tab) {
        case TAB_PARTY:  drawPartyTab(area);  break;
        case TAB_SKILLS: drawSkillsTab(area); break;
        case TAB_ITEMS:  drawItemsTab(area);  break;
        case TAB_GATE:   drawGateTab(area);   break;
        default: break;
    }

    drawButtons();
    drawCenteredText("Nhấp các thẻ và nút để chuẩn bị chiến đấu", cx, 702.0f, 16, COL_LABEL);

    // Tooltip ky nang: ve CUOI CUNG de nam tren moi thu
    if (m_tab == TAB_SKILLS && m_hoverSkill >= 0 &&
        static_cast<size_t>(m_hoverSkill) < m_info.m_skills.size()) {
        SkillTooltip::draw(toTipInfo(m_info.m_skills[static_cast<size_t>(m_hoverSkill)]),
                           UiInput::instance().mouse(), &m_info.m_hero.m_stats);
    }
}

void PrepMenuScreen::drawBackground() const {
    const int W = UiTheme::WIDTH, H = UiTheme::HEIGHT;
    if (m_hasBackground) {
        DrawTexturePro(m_background, Rectangle{ 0, 0, static_cast<float>(m_background.width), static_cast<float>(m_background.height) },
                       Rectangle{ 0, 0, static_cast<float>(W), static_cast<float>(H) }, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, Color{ 18, 22, 30, 255 }, Color{ 54, 57, 70, 255 });
    }
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 160 });
}

void PrepMenuScreen::drawTabs() const {
    for (int i = 0; i < TAB_COUNT; ++i) {
        const Rectangle r = tabRect(i);
        const bool active = (i == m_tab);
        const bool hover = (i == m_hoverTab);
        UiFrame::plaque(r, active, false, hover ? 1.0f : 0.0f);
        drawCenteredText(tabLabel(i), r.x + r.width / 2.0f, r.y + (r.height - 20.0f) / 2.0f, 20,
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
        const bool hovered = CheckCollisionPointRec(UiInput::instance().mouse(), r);
        const float cx = r.x + r.width / 2.0f;
        if (h) {
            UiFrame::panel(r, hovered);
            DrawRectangleRec(Rectangle{ r.x+12, r.y+9, r.width-24, 42.0f }, Color{ 47, 40, 46, 255 });
            drawCenteredText(h->m_name.c_str(), cx, r.y + 14.0f, 24, COL_TEXT);
            drawCenteredText(TextFormat("%s %s", h->m_race.c_str(), h->m_class.c_str()), cx, r.y + 68.0f, 17, COL_LABEL);
            DrawCircle(static_cast<int>(cx - 30), static_cast<int>(r.y + 106), 7.0f, elementColor(h->m_element));
            UiText::draw(TextFormat("Hệ %s", elementName(h->m_element)), static_cast<int>(cx - 16), static_cast<int>(r.y + 97), 18, elementColor(h->m_element));
            float y = r.y + 140.0f;
            const float x = r.x + 22.0f, w = r.width - 44.0f;
            drawStatRow("HP", h->m_stats.m_hp, x, y, w); y += 30.0f;
            drawStatRow("MP", h->m_stats.m_mp, x, y, w); y += 30.0f;
            drawStatRow("ATK", h->m_stats.m_a, x, y, w); y += 30.0f;
            drawStatRow("MATK", h->m_stats.m_ma, x, y, w); y += 30.0f;
            drawStatRow("DEF", h->m_stats.m_d, x, y, w); y += 30.0f;
            drawStatRow("MDEF", h->m_stats.m_md, x, y, w); y += 30.0f;
            drawStatRow("SPD", h->m_stats.m_spd, x, y, w);
            drawCenteredText(i == 0 ? "Thủ lĩnh · Nhấp để đổi" : "Đồng đội · Nhấp để đổi",
                             cx, r.y + r.height - 29.0f, 14, COL_OK);
        } else {
            UiFrame::panel(r,hovered);
            const float cy = r.y + r.height / 2.0f;
            DrawRectangle(static_cast<int>(cx - 3), static_cast<int>(cy - 28), 6, 56, COL_DIM);
            DrawRectangle(static_cast<int>(cx - 28), static_cast<int>(cy - 3), 56, 6, COL_DIM);
            drawCenteredText(TextFormat("Đồng đội %d", i + 1), cx, cy + 46.0f, 20, COL_LABEL);
            drawCenteredText("Nhấp để chọn", cx, cy + 74.0f, 16, COL_OK);
        }
        // Shard frame already marks hover/selection, no rectangular outlines.
    }
}

// ---------------- Tab 2: Kỹ năng ----------------
void PrepMenuScreen::drawSkillsTab(const Rectangle& area) const {
    UiText::draw("Kỹ năng của anh hùng", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
    drawRightText("MP", area.x + area.width - 250.0f, area.y + 24.0f, 14, COL_DIM);
    drawRightText("Sát thương", area.x + area.width - 120.0f, area.y + 24.0f, 14, COL_DIM);

    if (m_info.m_skills.empty()) {
        drawCenteredText("Chưa có kỹ năng", area.x + area.width / 2.0f, area.y + area.height / 2.0f, 22, COL_DIM);
        return;
    }

    const float rowH = 64.0f, rowGap = 10.0f;
    float y = area.y + 56.0f;
    for (size_t i = 0; i < m_info.m_skills.size() && i < MAX_SKILL_ROWS; ++i) {
        const PrepSkillInfo& s = m_info.m_skills[i];
        const Rectangle r = { area.x + 24.0f, y, area.width - 48.0f, rowH };
        const bool rowHover = CheckCollisionPointRec(UiInput::instance().mouse(), r);
        if (rowHover) m_hoverSkill = static_cast<int>(i);
        UiFrame::plaque(r,rowHover,false,rowHover?1.f:0.f);
        DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y), 6, static_cast<int>(r.height), elementColor(s.m_element));

        UiText::draw(s.m_name.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 10), 22, COL_TEXT);
        std::string sub = s.m_typeText;
        if (!s.m_effectText.empty()) sub += "  |  " + s.m_effectText;
        UiText::draw(sub.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 38), 16, COL_LABEL);

        UiText::draw(elementName(s.m_element), static_cast<int>(r.x + r.width - 410), static_cast<int>(r.y + 20), 18, elementColor(s.m_element));
        drawRightText(TextFormat("%d", s.m_mpCost), r.x + r.width - 226.0f, r.y + 20.0f, 20, Color{ 110, 170, 255, 255 });
        drawRightText(s.m_power > 0 ? TextFormat("%d%%", s.m_power) : "-", r.x + r.width - 24.0f, r.y + 20.0f, 20, COL_TEXT);
        y += rowH + rowGap;
    }
}

// ---------------- Tab 3: Túi đồ ----------------
void PrepMenuScreen::drawItemsTab(const Rectangle& area) const {
    UiText::draw("Túi đồ mang vào trận", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);

    const int cols = 4;
    const float pad = 28.0f, gap = 18.0f, top = 64.0f;
    const float slotW = (area.width - 2.0f * pad - (cols - 1) * gap) / cols;
    const float slotH = 150.0f;

    for (int i = 0; i < ITEM_SLOTS; ++i) {
        const int col = i % cols, row = i / cols;
        const Rectangle r = { area.x + pad + col * (slotW + gap), area.y + top + row * (slotH + gap), slotW, slotH };
        const bool filled = static_cast<size_t>(i) < m_info.m_items.size();
        UiFrame::panel(r,filled);
        if (filled) {
            const PrepItemInfo& it = m_info.m_items[static_cast<size_t>(i)];
            const float cx = r.x + r.width / 2.0f;
            UiWidgets::itemIcon({cx-30,r.y+20,60,65}, it.m_name);
            UiText::drawEllipsis(it.m_name,{r.x+12,r.y+92.f,r.width-24,26},20,COL_TEXT);
            drawCenteredText(TextFormat("x%d", it.m_count), cx, r.y + 118.0f, 18, COL_OK);
            if (UiInput::instance().hover(r))
                UiWidgets::tooltip(it.m_name, TextFormat("Số lượng: %d", it.m_count), UiInput::instance().mouse());
        } else {
            drawCenteredText("Trống", r.x + r.width / 2.0f, r.y + r.height / 2.0f - 9.0f, 18, Color{ 70, 66, 96, 255 });
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
    UiText::draw(g.m_title.empty() ? "Cổng" : g.m_title.c_str(), static_cast<int>(leftX), static_cast<int>(area.y + 22), 34, COL_TITLE);
    float y = area.y + 72.0f;
    for (const std::string& line : splitLines(g.m_description)) {
        UiText::draw(line.c_str(), static_cast<int>(leftX), static_cast<int>(y), 18, COL_LABEL);
        y += 26.0f;
    }

    // Thoi han (dong ho cong)
    const Rectangle clockBox = { leftX, area.y + 170.0f, leftW, 120.0f };
    const bool hasDeadline = g.m_deadlineTurns >= 0;
    UiFrame::panel(clockBox, hasDeadline);
    UiText::draw("THỜI HẠN", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 16), 16, COL_DIM);
    if (hasDeadline) {
        UiText::draw(TextFormat("%d", g.m_deadlineTurns), static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 44), 56, COL_TITLE);
        UiText::draw("lượt để hạ gác cổng", static_cast<int>(clockBox.x + 100 + (g.m_deadlineTurns >= 100 ? 36 : 0)),
                 static_cast<int>(clockBox.y + 66), 18, COL_TEXT);
    } else {
        UiText::draw("Không giới hạn", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 52), 30, COL_OK);
    }

    // Goi y khac he
    const Rectangle hintBox = { leftX, area.y + 310.0f, leftW, 150.0f };
    UiFrame::panel(hintBox);
    UiText::draw("VÒNG KHẮC HỆ", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 14), 16, COL_DIM);
    const Element ring[]={Element::KIM,Element::MOC,Element::THO,Element::THUY,Element::HOA};
    const char* names[]={"Kim","Mộc","Thổ","Thủy","Hỏa"};
    const Vector2 center={hintBox.x+86,hintBox.y+92};
    Vector2 nodes[5];
    for(int i=0;i<5;++i){
        const float theta=-1.5707963f+static_cast<float>(i)*6.2831853f/5.f;
        nodes[i]={center.x+53.f*std::cos(theta),center.y+53.f*std::sin(theta)};
    }
    for(int i=0;i<5;++i)DrawLineEx(nodes[i],nodes[(i+2)%5],1.5f,UiTheme::EMBER);
    for(int i=0;i<5;++i){
        DrawPoly(nodes[i],5,12,0,UiTheme::IRON);
        DrawPolyLines(nodes[i],5,12,0,elementColor(ring[i]));
        UiIcons::element(ring[i],nodes[i],6);
        if(UiInput::instance().hover({nodes[i].x-16,nodes[i].y-16,32,32})){
            const int e=static_cast<int>(ring[i]);
            UiWidgets::tooltip(names[i],TextFormat("Kim ×%.1f · Mộc ×%.1f · Thủy ×%.1f · Hỏa ×%.1f · Thổ ×%.1f",
                ELEMENT_MATRIX[e][0],ELEMENT_MATRIX[e][1],ELEMENT_MATRIX[e][2],ELEMENT_MATRIX[e][3],ELEMENT_MATRIX[e][4]),UiInput::instance().mouse());
        }
    }
    UiText::draw("Chạm vào biểu tượng để xem hệ số",static_cast<int>(hintBox.x+154),
       static_cast<int>(hintBox.y+52),16,UiTheme::TEXT);
    UiText::draw("Khắc hệ  ×1.5",static_cast<int>(hintBox.x+154),
       static_cast<int>(hintBox.y+84),18,UiTheme::HP);
    UiText::draw("Bị khắc  ×0.5",static_cast<int>(hintBox.x+154),
       static_cast<int>(hintBox.y+112),18,UiTheme::DANGER);

    // ---- Cot phai: boss + quai ----
    float ry = area.y + 22.0f;
    if (g.m_hasBoss) {
        const Rectangle boss = { rightX, ry, rightW, 130.0f };
        UiFrame::panel(boss,true);
        DrawRectangleRec(Rectangle{ boss.x + 18, boss.y + 14, 68, 22 }, COL_BOSS);
        drawCenteredText("BOSS", boss.x + 18 + 34, boss.y + 17, 16, WHITE);
        UiText::draw(g.m_bossName.c_str(), static_cast<int>(boss.x + 100), static_cast<int>(boss.y + 12), 28, COL_TEXT);
        DrawCircle(static_cast<int>(boss.x + 30), static_cast<int>(boss.y + 76), 8.0f, elementColor(g.m_bossElement));
        UiText::draw(TextFormat("Hệ %s", elementName(g.m_bossElement)), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 66), 18, elementColor(g.m_bossElement));
        UiText::draw(TextFormat("HP %d", g.m_bossHp), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 94), 18, COL_TEXT);
        ry += 130.0f + 20.0f;
    }

    UiText::draw(g.m_hasBoss ? "QUÂN HỘ TỐNG" : "QUÁI CÓ THỂ GẶP", static_cast<int>(rightX), static_cast<int>(ry), 16, COL_TITLE);
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
        UiText::draw("Không có dữ liệu", static_cast<int>(rightX), static_cast<int>(ry), 18, COL_DIM);
    else if (shown < g.m_enemies.size())
        UiText::draw(TextFormat("+%d quái khác", static_cast<int>(g.m_enemies.size() - shown)), static_cast<int>(rightX + 28), static_cast<int>(ry), 14, COL_DIM);
}

void PrepMenuScreen::drawButtons() const {
    const char* labels[BTN_COUNT] = { Strings::get(Strings::Key::BACK), "Vào trận" };
    for (int i = 0; i < BTN_COUNT; ++i) {
        const Rectangle r = buttonRect(i);
        const bool hover = (m_hoverButton == i);
        const bool primary = (i == BTN_START);
        UiFrame::plaque(r, primary || hover, false, hover?1.f:0.f);
        drawCenteredText(labels[i], r.x + r.width / 2.0f, r.y + (r.height - 24.0f) / 2.0f, 24,
                         hover ? UiTheme::AGED_GOLD : COL_TEXT);
    }
}