// src/graphics/ShopScreen.cpp
#include "graphics/ShopScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiInput.h"
#include "graphics/UiIcons.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include <algorithm>

namespace {
constexpr Color COL_TITLE    = UiTheme::accent;
constexpr Color COL_PANEL    { 20, 17, 31, 235 };
constexpr Color COL_EDGE     = UiTheme::border;
constexpr Color COL_EDGE_SEL = UiTheme::ember;
constexpr Color COL_LABEL    = UiTheme::ash;
constexpr Color COL_TEXT     = UiTheme::text;
constexpr Color COL_DIM      = UiTheme::ash;
constexpr Color COL_ERR      = UiTheme::danger;
constexpr Color COL_OK       = UiTheme::verdigris;
constexpr Color COL_GOLD     = UiTheme::currency;

const char* categoryText(int c)
{
    switch (c) {
        case 0:  return Strings::consumablesCategory;
        case 1:  return Strings::powerCategory;
        default: return Strings::equipmentCategory;
    }
}

void drawCentered(const char* text, float cx, float y, int size, Color c,
                  float letterSpacing = 0.0f, UiFontRole role = UiFontRole::AUTO)
{
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size, role, letterSpacing) / 2.0f),
                 static_cast<int>(y), size, c, role, letterSpacing);
}
void drawRight(const char* text, float right, float y, int size, Color c)
{
    UiText::draw(text, static_cast<int>(right - UiText::measure(text, size)), static_cast<int>(y), size, c);
}

void drawChip(Rectangle bounds, const char* text, Color fill, Color textColor) {
    UiShapes::roundedRect(bounds, UiTheme::radiusChip, fill, UiTheme::secondaryBorder, 1.0f);
    UiText::draw(text, static_cast<int>(bounds.x + 8.0f),
                 static_cast<int>(bounds.y + (bounds.height - 16.0f) / 2.0f),
                 16, textColor, UiFontRole::BODY);
}

Color withOpacity(Color color, float opacity)
{
    color.a = static_cast<unsigned char>(255.0f * std::clamp(opacity, 0.0f, 1.0f));
    return color;
}

void drawButton(Rectangle r, const char* label, bool enabled, bool hover,
                Color accent, bool primary = true)
{
    if (hover)
        UiInput::setCursorType(enabled ? UiInput::CursorType::POINTER
                                       : UiInput::CursorType::DISABLED);
    Rectangle body = r;
    if (enabled && hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) body.y += 2.0f;
    UiShapeEffects effects;
    effects.gradientBottom = enabled
        ? (primary ? withOpacity(UiTheme::primaryBottom, UiTheme::primaryButtonAlpha)
                   : withOpacity(UiTheme::buttonGlassBottom, UiTheme::buttonAlpha))
        : UiTheme::panelBottom;
    effects.shadowBlur = 6.0f;
    effects.glow = UiTheme::ember;
    effects.glowRadius = hover && enabled ? 5.0f : 0.0f;
    effects.glowStrength = hover && enabled ? 0.4f : 0.0f;
    UiShapes::roundedRect(body, UiTheme::radiusSmall,
        enabled ? (primary ? withOpacity(UiTheme::primaryTop, UiTheme::primaryButtonAlpha)
                           : withOpacity(UiTheme::buttonGlassTop, UiTheme::buttonAlpha))
                : UiTheme::panel,
        enabled ? (hover ? UiTheme::accent : accent) : COL_DIM, 1.0f, effects);
    drawCentered(label, body.x + body.width / 2.0f, body.y + (body.height - 20.0f) / 2.0f, 20,
                 enabled ? (primary ? UiTheme::primaryText : COL_TEXT) : COL_DIM,
                 20.0f * 0.04f, UiFontRole::HUD);
}
} // namespace

// ============ Du lieu ============
void ShopScreen::setInfo(const ShopInfo& info)
{
    m_info = info;
    for (ShopStockInfo& stock : m_info.m_stock)
        stock.m_description = Strings::localizedItemDescription(stock.m_description);
    for (ShopBagInfo& item : m_info.m_bag)
        item.m_description = Strings::localizedItemDescription(item.m_description);
    clampSelection();
}

void ShopScreen::open(int tab)
{
    m_tab = std::max(0, std::min(tab, static_cast<int>(TAB_COUNT) - 1));
    m_stockSel = m_bagSel = 0;
    m_scroll[0] = m_scroll[1] = 0;
    m_actionMember = 0;
    m_message.clear();
    m_messageTime = 0.0f;
}

void ShopScreen::setMessage(const std::string& text, bool ok)
{
    m_message = text;
    m_messageOk = ok;
    m_messageTime = 3.0f;
}

int ShopScreen::listCount() const
{
    return static_cast<int>(m_tab == TAB_SHOP ? m_info.m_stock.size() : m_info.m_bag.size());
}
int& ShopScreen::selection() { return m_tab == TAB_SHOP ? m_stockSel : m_bagSel; }
int ShopScreen::selection() const { return m_tab == TAB_SHOP ? m_stockSel : m_bagSel; }

void ShopScreen::clampSelection()
{
    const int stock = static_cast<int>(m_info.m_stock.size());
    const int bag = static_cast<int>(m_info.m_bag.size());
    m_stockSel = std::max(0, std::min(m_stockSel, std::max(0, stock - 1)));
    m_bagSel = std::max(0, std::min(m_bagSel, std::max(0, bag - 1)));
    m_scroll[TAB_SHOP] = std::max(0, std::min(m_scroll[TAB_SHOP], std::max(0, stock - VISIBLE_ROWS)));
    m_scroll[TAB_BAG] = std::max(0, std::min(m_scroll[TAB_BAG], std::max(0, bag - VISIBLE_ROWS)));
}

void ShopScreen::ensureVisible()
{
    int& scroll = m_scroll[m_tab];
    const int sel = selection();
    if (sel < scroll) scroll = sel;
    if (sel >= scroll + VISIBLE_ROWS) scroll = sel - VISIBLE_ROWS + 1;
    scroll = std::max(0, std::min(scroll, std::max(0, listCount() - VISIBLE_ROWS)));
}

// ============ Bo cuc (1280x720) ============
Rectangle ShopScreen::panelRect() const
{
    const float W = static_cast<float>(UiCanvas::width());
    return { (W - 1100.0f) / 2.0f, 30.0f, 1100.0f, 660.0f };
}
Rectangle ShopScreen::tabRect(int i) const
{
    const Rectangle p = panelRect();
    return { p.x + 24.0f + static_cast<float>(i) * 260.0f, p.y + 64.0f, 250.0f, 48.0f };
}
Rectangle ShopScreen::listRect() const
{
    const Rectangle p = panelRect();
    return { p.x + 24.0f, p.y + 118.0f, 640.0f, static_cast<float>(ROW_H * VISIBLE_ROWS) };
}
Rectangle ShopScreen::rowRect(int visibleRow) const
{
    const Rectangle l = listRect();
    return { l.x, l.y + static_cast<float>(visibleRow * ROW_H), l.width, static_cast<float>(ROW_H - 4) };
}
Rectangle ShopScreen::detailRect() const
{
    const Rectangle p = panelRect(), l = listRect();
    return { l.x + l.width + 20.0f, l.y, p.x + p.width - 24.0f - (l.x + l.width + 20.0f), l.height };
}
Rectangle ShopScreen::buyButtonRect() const
{
    const Rectangle d = detailRect();
    return { d.x + 20.0f, d.y + d.height - 64.0f, d.width - 40.0f, 48.0f };
}
Rectangle ShopScreen::memberRect(int i) const
{
    const Rectangle d = detailRect();
    return { d.x + 12.0f, d.y + 190.0f + static_cast<float>(i) * 84.0f, d.width - 24.0f, 76.0f };
}
Rectangle ShopScreen::unequipRect(int i) const
{
    const Rectangle m = memberRect(i);
    return { m.x + m.width - 84.0f, m.y + 14.0f, 74.0f, 48.0f };
}
Rectangle ShopScreen::closeRect() const
{
    const Rectangle p = panelRect();
    return { p.x + p.width - 200.0f, p.y + p.height - 64.0f, 176.0f, 48.0f };
}

// ============ Input ============
ShopAction ShopScreen::handleInput()
{
    const Vector2 mouse = UiInput::mousePosition();
    const bool clicked = UiInput::pressed(MOUSE_BUTTON_LEFT);
    clampSelection();

    if (UiInput::rightClicked()) {
        UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
        return ShopAction::CLOSE;
    }

    if (IsKeyPressed(KEY_ONE) && m_tab != TAB_SHOP) { m_tab = TAB_SHOP; }
    if (IsKeyPressed(KEY_TWO) && m_tab != TAB_BAG)  { m_tab = TAB_BAG; }
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E)) m_tab = (m_tab + 1) % TAB_COUNT;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B)) return ShopAction::CLOSE;

    const int count = listCount();
    if (count > 0) {
        int& sel = selection();
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) sel = std::min(sel + 1, count - 1);
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) sel = std::max(sel - 1, 0);
        const float wheel = UiInput::wheel();
        if (wheel != 0.0f && CheckCollisionPointRec(mouse, listRect()))
            m_scroll[m_tab] = std::max(0, std::min(m_scroll[m_tab] - static_cast<int>(wheel), std::max(0, count - VISIBLE_ROWS)));
        else ensureVisible();
    }

    if (count > VISIBLE_ROWS) {
        const Rectangle list = listRect();
        const Rectangle track{ list.x + list.width + 7.0f, list.y, 10.0f, list.height };
        const float thumbHeight = std::max(28.0f, track.height * VISIBLE_ROWS / static_cast<float>(count));
        const float thumbTravel = track.height - thumbHeight;
        const float scrollRange = static_cast<float>(count - VISIBLE_ROWS);
        const float thumbY = track.y + thumbTravel * static_cast<float>(m_scroll[m_tab]) / scrollRange;
        const Rectangle thumb{ track.x, thumbY, track.width, thumbHeight };
        if (clicked && CheckCollisionPointRec(mouse, track)) {
            if (CheckCollisionPointRec(mouse, thumb)) {
                m_draggingScrollbar = true;
                m_scrollbarGrabOffset = mouse.y - thumb.y;
            } else {
                m_scroll[m_tab] = std::clamp(
                    static_cast<int>((mouse.y - track.y - thumbHeight * 0.5f) /
                                     std::max(1.0f, thumbTravel) * scrollRange),
                    0, count - VISIBLE_ROWS);
                m_draggingScrollbar = true;
                m_scrollbarGrabOffset = thumbHeight * 0.5f;
            }
        }
        if (m_draggingScrollbar && UiInput::down()) {
            m_scroll[m_tab] = std::clamp(
                static_cast<int>((mouse.y - track.y - m_scrollbarGrabOffset) /
                                 std::max(1.0f, thumbTravel) * scrollRange),
                0, count - VISIBLE_ROWS);
        }
        if (UiInput::released()) m_draggingScrollbar = false;
        UiInput::hovered(track);
        if (CheckCollisionPointRec(mouse, track) || m_draggingScrollbar)
            UiInput::setCursorType(UiInput::CursorType::POINTER);
    } else {
        m_draggingScrollbar = false;
    }

    // ---- chuot: chon dong ----
    m_hoverRow = -1;
    for (int r = 0; r < VISIBLE_ROWS; ++r) {
        const int idx = m_scroll[m_tab] + r;
        if (idx >= count) break;
        if (CheckCollisionPointRec(mouse, rowRect(r))) {
            m_hoverRow = idx;
            UiInput::setCursorType(UiInput::CursorType::POINTER);
            if (clicked) selection() = idx;
        }
    }
    for (int t = 0; t < TAB_COUNT; ++t)
        if (clicked && CheckCollisionPointRec(mouse, tabRect(t))) m_tab = t;
    if (clicked && CheckCollisionPointRec(mouse, closeRect())) {
        UiInput::consumeClick();
        return ShopAction::CLOSE;
    }

    // ---- hanh dong ----
    ShopAction action = ShopAction::NONE;
    if (m_tab == TAB_SHOP) {
        if (count > 0 && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE) ||
            (clicked && CheckCollisionPointRec(mouse, buyButtonRect()))))
            action = ShopAction::BUY;
    } else {
        const int members = static_cast<int>(m_info.m_members.size());
        if (m_info.m_bag.empty() == false) {
            const KeyboardKey keys[] = { KEY_F1, KEY_F2, KEY_F3 };
            for (int i = 0; i < members && i < 3; ++i) {
                if (IsKeyPressed(keys[i])) { m_actionMember = i; action = ShopAction::EQUIP; break; }
                if (clicked && CheckCollisionPointRec(mouse, memberRect(i)) &&
                    !CheckCollisionPointRec(mouse, unequipRect(i))) {
                    m_actionMember = i;
                    action = ShopAction::EQUIP;
                    break;
                }
            }
        }
        for (int i = 0; i < members && i < 3; ++i)
            if (clicked && CheckCollisionPointRec(mouse, unequipRect(i)) &&
                !m_info.m_members[static_cast<size_t>(i)].m_gearId.empty()) {
                m_actionMember = i;
                action = ShopAction::UNEQUIP;
                break;
            }
        const KeyboardKey ukeys[] = { KEY_Z, KEY_X, KEY_C };   // thao nhanh: Z/X/C = hero 1/2/3
        for (int i = 0; i < members && i < 3; ++i)
            if (IsKeyPressed(ukeys[i]) && !m_info.m_members[static_cast<size_t>(i)].m_gearId.empty()) {
                m_actionMember = i;
                action = ShopAction::UNEQUIP;
                break;
            }
    }
    if (clicked && CheckCollisionPointRec(mouse, panelRect())) UiInput::consumeClick();
    return action;
}

// ============ Ve ============
void ShopScreen::draw() const
{
    const int W = UiCanvas::width(), H = UiCanvas::height();
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 190 });

    const Rectangle p = panelRect();
    UiShapeEffects panelEffects;
    panelEffects.gradientBottom = UiTheme::panelBottom;
    panelEffects.shadowBlur = UiTheme::floatingShadowSize;
    UiShapes::roundedRect(p, UiTheme::radiusLarge, COL_PANEL, COL_EDGE, 1.0f, panelEffects);

    UiText::draw(Strings::shopTitle, static_cast<int>(p.x + 24), static_cast<int>(p.y + 16), 34, COL_TITLE);
    const char* gold = TextFormat(Strings::goldBalanceTemplate, m_info.m_gold);
    UiIcons::draw("action:coin",
        Rectangle{ p.x + p.width - 58.0f - UiText::measure(gold, 30),
                   p.y + 20.0f, 26.0f, 26.0f });
    drawRight(gold, p.x + p.width - 28.0f, p.y + 18.0f, 30, COL_GOLD);

    const char* tabNames[TAB_COUNT] = { Strings::shopTab, Strings::inventoryEquipmentTab };
    const Vector2 mouse = UiInput::mousePosition();
    for (int t = 0; t < TAB_COUNT; ++t) {
        const Rectangle r = tabRect(t);
        const bool active = (t == m_tab), hover = UiInput::hovered(r);
        if (hover) UiInput::setCursorType(UiInput::CursorType::POINTER);
        UiShapeEffects effects;
        effects.flatBottom = true;
        effects.gradientBottom = active ? UiTheme::primaryBottom : UiTheme::panelBottom;
        UiShapes::roundedRect(r, UiTheme::radiusSmall,
            active ? withOpacity(UiTheme::primaryTop, UiTheme::buttonAlpha) : UiTheme::panel,
            active || hover ? UiTheme::accent : COL_EDGE, 1.0f, effects);
        if (active)
            UiShapes::roundedRect(Rectangle{ r.x + 8.0f, r.y + r.height - 2.0f,
                                             r.width - 16.0f, 2.0f },
                                  UiTheme::radiusBar, UiTheme::ember);
        drawCentered(tabNames[t], r.x + r.width / 2.0f, r.y + 10.0f, 20,
                     active ? UiTheme::primaryText : COL_LABEL, 20.0f * 0.04f, UiFontRole::HUD);
    }

    drawList();
    if (m_tab == TAB_SHOP) drawShopDetail(); else drawBagDetail();

    // thong bao
    if (m_messageTime > 0.0f && !m_message.empty())
        UiText::drawEllipsis(m_message, static_cast<int>(p.x + 24), static_cast<int>(p.y + p.height - 48),
                             820, 18, m_messageOk ? COL_OK : COL_ERR, UiFontRole::BODY);
    drawButton(closeRect(), Strings::close, true, CheckCollisionPointRec(mouse, closeRect()),
               UiTheme::secondaryBorder, false);
}

void ShopScreen::drawList() const
{
    const Rectangle l = listRect();
    const int count = listCount();
    if (count == 0) {
        drawCentered(m_tab == TAB_SHOP ? Strings::shopEmpty : Strings::inventoryEmptyShopHint,
                     l.x + l.width / 2.0f, l.y + 120.0f, 20, COL_DIM);
        return;
    }
    const Vector2 mouse = UiCanvas::mousePosition();
    for (int r = 0; r < VISIBLE_ROWS; ++r) {
        const int idx = m_scroll[m_tab] + r;
        if (idx >= count) break;
        const Rectangle row = rowRect(r);
        const bool selected = (idx == selection());
        const bool hover = CheckCollisionPointRec(mouse, row);
        UiShapeEffects rowEffects;
        rowEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(row, UiTheme::radiusSmall, UiTheme::panel,
            selected ? COL_EDGE_SEL : (hover ? UiTheme::accent : COL_EDGE), 1.0f, rowEffects);

        if (m_tab == TAB_SHOP) {
            const ShopStockInfo& s = m_info.m_stock[static_cast<size_t>(idx)];
            const bool soldOut = s.m_limit > 0 && s.m_owned >= s.m_limit;
            const bool afford = m_info.m_gold >= s.m_price;
            if (m_icons) m_icons->draw(s.m_id, Rectangle{ row.x + 8, row.y + 4, 52, 52 }, soldOut ? Color{ 120, 120, 120, 255 } : WHITE);
            UiText::drawEllipsis(s.m_name, static_cast<int>(row.x + 74), static_cast<int>(row.y + 8),
                                 static_cast<int>(row.width - 74.0f - 150.0f), 20,
                                 soldOut ? COL_DIM : COL_TEXT, UiFontRole::HUD);
            const char* category = categoryText(s.m_category);
            const float categoryWidth = static_cast<float>(UiText::measure(category, 16, UiFontRole::BODY) + 16);
            drawChip(Rectangle{ row.x + 74.0f, row.y + 36.0f, categoryWidth, 22.0f },
                     category, UiTheme::panelBottom, COL_LABEL);
            if (soldOut) drawRight(Strings::soldOut, row.x + row.width - 14.0f, row.y + 10.0f, 22, COL_DIM);
            else {
                drawRight(TextFormat(Strings::goldPriceTemplate, s.m_price), row.x + row.width - 14.0f, row.y + 10.0f, 24, afford ? COL_GOLD : COL_ERR);
            }
            drawRight(s.m_limit > 0 ? TextFormat(Strings::ownedCountTemplate, s.m_owned, s.m_limit)
                                    : TextFormat(Strings::ownedUnlimitedTemplate, s.m_owned),
                      row.x + row.width - 14.0f, row.y + 36.0f, 16, COL_LABEL);
        } else {
            const ShopBagInfo& b = m_info.m_bag[static_cast<size_t>(idx)];
            if (m_icons) m_icons->draw(b.m_id, Rectangle{ row.x + 8, row.y + 4, 52, 52 }, WHITE);
            UiText::drawEllipsis(b.m_name, static_cast<int>(row.x + 74), static_cast<int>(row.y + 8),
                                 static_cast<int>(row.width - 74.0f - 80.0f), 20,
                                 COL_TEXT, UiFontRole::HUD);
            UiText::draw(categoryText(b.m_category), static_cast<int>(row.x + 74), static_cast<int>(row.y + 36), 16, COL_LABEL,
                         UiFontRole::BODY);
            drawRight(TextFormat("x%d", b.m_count), row.x + row.width - 14.0f, row.y + 16.0f, 26, COL_OK);
        }
    }
    if (count > VISIBLE_ROWS) {
        const Rectangle track{ l.x + l.width + 7.0f, l.y, 5.0f, l.height };
        const float thumbHeight = std::max(28.0f, track.height * VISIBLE_ROWS / static_cast<float>(count));
        const float scrollRange = static_cast<float>(std::max(1, count - VISIBLE_ROWS));
        const float thumbY = track.y + (track.height - thumbHeight) *
            static_cast<float>(m_scroll[m_tab]) / scrollRange;
        UiShapes::roundedRect(track, UiTheme::radiusBar, UiTheme::panelBottom);
        UiShapes::roundedRect(Rectangle{ track.x, thumbY, track.width, thumbHeight },
                              UiTheme::radiusBar, UiTheme::accent);
    }
}

void ShopScreen::drawShopDetail() const
{
    const Rectangle d = detailRect();
    UiShapeEffects detailEffects;
    detailEffects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(d, UiTheme::radiusCard, UiTheme::panel, COL_EDGE, 1.0f, detailEffects);
    if (m_info.m_stock.empty()) return;
    const ShopStockInfo& s = m_info.m_stock[static_cast<size_t>(m_stockSel)];
    if (m_icons) m_icons->draw(s.m_id, Rectangle{ d.x + d.width / 2.0f - 48.0f, d.y + 16.0f, 96, 96 }, WHITE);
    drawCentered(UiText::ellipsis(s.m_name, static_cast<int>(d.width - 24.0f), 22, UiFontRole::HUD).c_str(),
                 d.x + d.width / 2.0f, d.y + 118.0f, 22, COL_TITLE);
    drawCentered(categoryText(s.m_category), d.x + d.width / 2.0f, d.y + 146.0f, 16, COL_LABEL);
    UiText::drawWrapped(s.m_description, static_cast<int>(d.x + 20.0f), static_cast<int>(d.y + 174.0f),
                        static_cast<int>(d.width - 40.0f), 18, COL_TEXT, 25, UiFontRole::BODY);
    drawRight(TextFormat(Strings::priceTemplate, s.m_price), d.x + d.width - 20.0f, d.y + d.height - 106.0f, 22,
              m_info.m_gold >= s.m_price ? COL_GOLD : COL_ERR);
    UiText::draw(s.m_limit > 0 ? TextFormat(Strings::itemOwnedDetailLimitTemplate, s.m_owned, s.m_limit)
                               : TextFormat(Strings::itemOwnedDetailTemplate, s.m_owned),
             static_cast<int>(d.x + 20), static_cast<int>(d.y + d.height - 104), 18, COL_LABEL);

    const bool soldOut = s.m_limit > 0 && s.m_owned >= s.m_limit;
    const bool afford = m_info.m_gold >= s.m_price;
    const char* label = soldOut ? Strings::soldOutFull
        : (afford ? TextFormat(Strings::buyPriceTemplate, s.m_price) : Strings::insufficientGold);
    drawButton(buyButtonRect(), label, !soldOut && afford, CheckCollisionPointRec(UiCanvas::mousePosition(), buyButtonRect()), COL_EDGE_SEL);
}

void ShopScreen::drawBagDetail() const
{
    const Rectangle d = detailRect();
    UiShapeEffects detailEffects;
    detailEffects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(d, UiTheme::radiusCard, UiTheme::panel, COL_EDGE, 1.0f, detailEffects);

    const bool hasBag = !m_info.m_bag.empty();
    const ShopBagInfo* sel = hasBag ? &m_info.m_bag[static_cast<size_t>(m_bagSel)] : nullptr;
    if (sel) {
        if (m_icons) m_icons->draw(sel->m_id, Rectangle{ d.x + 14.0f, d.y + 12.0f, 64, 64 }, WHITE);
        UiText::drawEllipsis(sel->m_name, static_cast<int>(d.x + 90), static_cast<int>(d.y + 14),
                             static_cast<int>(d.width - 104.0f), 20, COL_TITLE, UiFontRole::HUD);
        UiText::drawWrapped(sel->m_description, static_cast<int>(d.x + 90.0f),
                            static_cast<int>(d.y + 44.0f), static_cast<int>(d.width - 104.0f),
                            16, COL_TEXT, 22, UiFontRole::BODY);
    } else {
        UiText::draw(Strings::noSelectedItem, static_cast<int>(d.x + 20), static_cast<int>(d.y + 30), 20, COL_DIM);
    }

    UiText::draw(Strings::formation, static_cast<int>(d.x + 14), static_cast<int>(d.y + 166), 16, COL_TITLE);
    const bool canEquip = sel && sel->m_category == 2;
    const Vector2 mouse = UiCanvas::mousePosition();
    for (size_t i = 0; i < m_info.m_members.size() && i < 3; ++i) {
        const ShopMemberInfo& m = m_info.m_members[i];
        const Rectangle r = memberRect(static_cast<int>(i));
        const bool hover = CheckCollisionPointRec(mouse, r);
        UiShapeEffects rowEffects;
        rowEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(r, UiTheme::radiusSmall, UiTheme::panel,
            hover && canEquip ? COL_EDGE_SEL : COL_EDGE, 1.0f, rowEffects);
        const std::string memberName = Strings::localizedName(m.m_name);
        UiText::draw(TextFormat("%d. %s", static_cast<int>(i) + 1, memberName.c_str()), static_cast<int>(r.x + 12), static_cast<int>(r.y + 8), 20, COL_TEXT);
        if (m.m_gearId.empty()) {
            UiText::draw(Strings::noEquipmentEquipped, static_cast<int>(r.x + 12), static_cast<int>(r.y + 40), 16, COL_DIM);
        } else {
            if (m_icons) m_icons->draw(m.m_gearId, Rectangle{ r.x + 12, r.y + 34, 36, 36 }, WHITE);
            UiText::draw(m.m_gearName.c_str(), static_cast<int>(r.x + 56), static_cast<int>(r.y + 44), 16, COL_OK);
            drawButton(unequipRect(static_cast<int>(i)), Strings::unequip, true,
                       CheckCollisionPointRec(mouse, unequipRect(static_cast<int>(i))),
                       UiTheme::secondaryBorder, false);
        }
    }
}
