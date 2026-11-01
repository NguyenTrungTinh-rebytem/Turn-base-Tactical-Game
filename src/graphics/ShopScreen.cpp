// src/graphics/ShopScreen.cpp
#include "graphics/ShopScreen.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include <algorithm>

namespace {
constexpr Color COL_TITLE    { 255, 214, 96, 255 };
constexpr Color COL_PANEL    { 16, 13, 36, 245 };
constexpr Color COL_EDGE     { 140, 120, 180, 255 };
constexpr Color COL_EDGE_SEL { 255, 214, 96, 255 };
constexpr Color COL_LABEL    { 190, 185, 220, 255 };
constexpr Color COL_TEXT     { 238, 235, 252, 255 };
constexpr Color COL_DIM      { 130, 126, 160, 255 };
constexpr Color COL_ERR      { 255, 110, 110, 255 };
constexpr Color COL_OK       { 120, 230, 140, 255 };
constexpr Color COL_GOLD     { 255, 205, 60, 255 };

const char* categoryText(int c)
{
    switch (c) {
        case 0:  return u8"Tiêu hao (dùng ngoài và trong trận)";
        case 1:  return u8"Tăng sức mạnh (dùng trong trận, tốn 1 lượt)";
        default: return u8"Trang bị (đeo trước trận)";
    }
}

void drawCentered(const char* text, float cx, float y, int size, Color c)
{
    UiText::draw(text, static_cast<int>(cx - UiText::measure(text, size) / 2.0f), static_cast<int>(y), size, c);
}
void drawRight(const char* text, float right, float y, int size, Color c)
{
    UiText::draw(text, static_cast<int>(right - UiText::measure(text, size)), static_cast<int>(y), size, c);
}

// Tu dong xuong dong theo do rong (px). Tra ve so dong da ve.
int drawWrapped(const std::string& text, float x, float y, float maxW, int size, Color c, int lineH)
{
    std::string line, word;
    int lines = 0;
    auto flush = [&]() {
        if (line.empty()) return;
        UiText::draw(line.c_str(), static_cast<int>(x), static_cast<int>(y) + lines * lineH, size, c);
        ++lines;
        line.clear();
    };
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == ' ') {
            if (!word.empty()) {
                const std::string trial = line.empty() ? word : line + " " + word;
                if (!line.empty() && UiText::measure(trial.c_str(), size) > maxW) { flush(); line = word; }
                else line = trial;
                word.clear();
            }
        } else word += text[i];
    }
    flush();
    return lines;
}

void drawButton(Rectangle r, const char* label, bool enabled, bool hover, Color accent)
{
    DrawRectangleRec(Rectangle{ r.x + 3, r.y + 4, r.width, r.height }, Color{ 0, 0, 0, 110 });
    DrawRectangleRec(r, !enabled ? Color{ 40, 38, 56, 235 } : (hover ? Color{ 90, 60, 20, 240 } : Color{ 50, 40, 14, 235 }));
    DrawRectangleLinesEx(r, hover && enabled ? 3.0f : 2.0f, enabled ? accent : COL_DIM);
    drawCentered(label, r.x + r.width / 2.0f, r.y + (r.height - 20.0f) / 2.0f, 20, enabled ? COL_TEXT : COL_DIM);
}
} // namespace

// ============ Du lieu ============
void ShopScreen::setInfo(const ShopInfo& info)
{
    m_info = info;
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
    return { p.x + 24.0f + static_cast<float>(i) * 260.0f, p.y + 64.0f, 250.0f, 40.0f };
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
    return { m.x + m.width - 84.0f, m.y + 22.0f, 74.0f, 32.0f };
}
Rectangle ShopScreen::closeRect() const
{
    const Rectangle p = panelRect();
    return { p.x + p.width - 200.0f, p.y + p.height - 56.0f, 176.0f, 40.0f };
}

// ============ Input ============
ShopAction ShopScreen::handleInput()
{
    const Vector2 mouse = UiCanvas::mousePosition();
    clampSelection();

    if (IsKeyPressed(KEY_ONE) && m_tab != TAB_SHOP) { m_tab = TAB_SHOP; }
    if (IsKeyPressed(KEY_TWO) && m_tab != TAB_BAG)  { m_tab = TAB_BAG; }
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E)) m_tab = (m_tab + 1) % TAB_COUNT;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B)) return ShopAction::CLOSE;

    const int count = listCount();
    if (count > 0) {
        int& sel = selection();
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) sel = std::min(sel + 1, count - 1);
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) sel = std::max(sel - 1, 0);
        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f && CheckCollisionPointRec(mouse, listRect()))
            m_scroll[m_tab] = std::max(0, std::min(m_scroll[m_tab] - static_cast<int>(wheel), std::max(0, count - VISIBLE_ROWS)));
        else ensureVisible();
    }

    // ---- chuot: chon dong ----
    m_hoverRow = -1;
    for (int r = 0; r < VISIBLE_ROWS; ++r) {
        const int idx = m_scroll[m_tab] + r;
        if (idx >= count) break;
        if (CheckCollisionPointRec(mouse, rowRect(r))) {
            m_hoverRow = idx;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) selection() = idx;
        }
    }
    for (int t = 0; t < TAB_COUNT; ++t)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, tabRect(t))) m_tab = t;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, closeRect())) return ShopAction::CLOSE;

    // ---- hanh dong ----
    if (m_tab == TAB_SHOP) {
        if (count > 0 && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, buyButtonRect()))))
            return ShopAction::BUY;
    } else {
        const int members = static_cast<int>(m_info.m_members.size());
        if (m_info.m_bag.empty() == false) {
            const KeyboardKey keys[] = { KEY_F1, KEY_F2, KEY_F3 };
            for (int i = 0; i < members && i < 3; ++i) {
                if (IsKeyPressed(keys[i])) { m_actionMember = i; return ShopAction::EQUIP; }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, memberRect(i)) &&
                    !CheckCollisionPointRec(mouse, unequipRect(i))) { m_actionMember = i; return ShopAction::EQUIP; }
            }
        }
        for (int i = 0; i < members && i < 3; ++i)
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, unequipRect(i)) &&
                !m_info.m_members[static_cast<size_t>(i)].m_gearId.empty()) { m_actionMember = i; return ShopAction::UNEQUIP; }
        const KeyboardKey ukeys[] = { KEY_Z, KEY_X, KEY_C };   // thao nhanh: Z/X/C = hero 1/2/3
        for (int i = 0; i < members && i < 3; ++i)
            if (IsKeyPressed(ukeys[i]) && !m_info.m_members[static_cast<size_t>(i)].m_gearId.empty()) {
                m_actionMember = i;
                return ShopAction::UNEQUIP;
            }
    }
    return ShopAction::NONE;
}

// ============ Ve ============
void ShopScreen::draw() const
{
    const int W = UiCanvas::width(), H = UiCanvas::height();
    DrawRectangle(0, 0, W, H, Color{ 0, 0, 0, 190 });

    const Rectangle p = panelRect();
    DrawRectangleRec(Rectangle{ p.x + 6, p.y + 8, p.width, p.height }, Color{ 0, 0, 0, 120 });
    DrawRectangleRec(p, COL_PANEL);
    DrawRectangleLinesEx(p, 2.0f, COL_EDGE);

    UiText::draw(Strings::shopTitle, static_cast<int>(p.x + 24), static_cast<int>(p.y + 16), 34, COL_TITLE);
    const char* gold = TextFormat(u8"VÀNG: %d", m_info.m_gold);
    DrawCircle(static_cast<int>(p.x + p.width - 40 - UiText::measure(gold, 30) - 18), static_cast<int>(p.y + 33), 11.0f, COL_GOLD);
    DrawCircleLines(static_cast<int>(p.x + p.width - 40 - UiText::measure(gold, 30) - 18), static_cast<int>(p.y + 33), 11.0f, Color{ 120, 80, 10, 255 });
    drawRight(gold, p.x + p.width - 28.0f, p.y + 18.0f, 30, COL_GOLD);

    const char* tabNames[TAB_COUNT] = { u8"1. CỬA HÀNG", u8"2. TÚI ĐỒ / TRANG BỊ" };
    const Vector2 mouse = UiCanvas::mousePosition();
    for (int t = 0; t < TAB_COUNT; ++t) {
        const Rectangle r = tabRect(t);
        const bool active = (t == m_tab), hover = CheckCollisionPointRec(mouse, r);
        DrawRectangleRec(r, active ? Color{ 60, 46, 18, 240 } : (hover ? Color{ 40, 34, 70, 235 } : Color{ 24, 20, 50, 225 }));
        DrawRectangleLinesEx(r, active ? 3.0f : 2.0f, active ? COL_EDGE_SEL : COL_EDGE);
        drawCentered(tabNames[t], r.x + r.width / 2.0f, r.y + 10.0f, 20, active ? COL_TITLE : COL_LABEL);
    }

    drawList();
    if (m_tab == TAB_SHOP) drawShopDetail(); else drawBagDetail();

    // thong bao
    if (m_messageTime > 0.0f && !m_message.empty())
        UiText::draw(m_message.c_str(), static_cast<int>(p.x + 24), static_cast<int>(p.y + p.height - 48), 20, m_messageOk ? COL_OK : COL_ERR);
    const char* help = (m_tab == TAB_SHOP)
        ? u8"[LÊN/XUỐNG] Chọn   [ENTER] Mua   [TAB] Đổi thẻ   [ESC/B] Đóng"
        : u8"[LÊN/XUỐNG] Chọn đồ   [F1-F3 / nhấp tướng] Trang bị   [Z/X/C] Tháo   [ESC/B] Đóng";
    UiText::draw(help, static_cast<int>(p.x + 24), static_cast<int>(p.y + p.height - 24), 15, COL_DIM);

    drawButton(closeRect(), u8"ĐÓNG", true, CheckCollisionPointRec(mouse, closeRect()), COL_EDGE_SEL);
}

void ShopScreen::drawList() const
{
    const Rectangle l = listRect();
    const int count = listCount();
    if (count == 0) {
        drawCentered(m_tab == TAB_SHOP ? u8"Cửa hàng đang trống" : u8"Túi đồ trống - hãy mua vật phẩm ở cửa hàng",
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
        DrawRectangleRec(row, selected ? Color{ 52, 44, 92, 250 } : (hover ? Color{ 40, 34, 80, 245 } : Color{ 28, 24, 58, 235 }));
        DrawRectangleLinesEx(row, selected ? 3.0f : 2.0f, selected ? COL_EDGE_SEL : COL_EDGE);

        if (m_tab == TAB_SHOP) {
            const ShopStockInfo& s = m_info.m_stock[static_cast<size_t>(idx)];
            const bool soldOut = s.m_limit > 0 && s.m_owned >= s.m_limit;
            const bool afford = m_info.m_gold >= s.m_price;
            if (m_icons) m_icons->draw(s.m_id, Rectangle{ row.x + 8, row.y + 4, 52, 52 }, soldOut ? Color{ 120, 120, 120, 255 } : WHITE);
            UiText::draw(s.m_name.c_str(), static_cast<int>(row.x + 74), static_cast<int>(row.y + 8), 22, soldOut ? COL_DIM : COL_TEXT);
            UiText::draw(categoryText(s.m_category), static_cast<int>(row.x + 74), static_cast<int>(row.y + 36), 14, COL_LABEL);
            if (soldOut) drawRight(u8"ĐÃ ĐỦ", row.x + row.width - 14.0f, row.y + 10.0f, 22, COL_DIM);
            else {
                drawRight(TextFormat(u8"%d vàng", s.m_price), row.x + row.width - 14.0f, row.y + 10.0f, 24, afford ? COL_GOLD : COL_ERR);
            }
            drawRight(s.m_limit > 0 ? TextFormat(u8"Có %d/%d", s.m_owned, s.m_limit) : TextFormat(u8"Có %d", s.m_owned),
                      row.x + row.width - 14.0f, row.y + 38.0f, 15, COL_LABEL);
        } else {
            const ShopBagInfo& b = m_info.m_bag[static_cast<size_t>(idx)];
            if (m_icons) m_icons->draw(b.m_id, Rectangle{ row.x + 8, row.y + 4, 52, 52 }, WHITE);
            UiText::draw(b.m_name.c_str(), static_cast<int>(row.x + 74), static_cast<int>(row.y + 8), 22, COL_TEXT);
            UiText::draw(categoryText(b.m_category), static_cast<int>(row.x + 74), static_cast<int>(row.y + 36), 14, COL_LABEL);
            drawRight(TextFormat("x%d", b.m_count), row.x + row.width - 14.0f, row.y + 16.0f, 26, COL_OK);
        }
    }
    if (count > VISIBLE_ROWS)
        drawRight(TextFormat("%d-%d / %d", m_scroll[m_tab] + 1, std::min(count, m_scroll[m_tab] + VISIBLE_ROWS), count),
                  l.x + l.width, l.y + l.height + 4.0f, 14, COL_DIM);
}

void ShopScreen::drawShopDetail() const
{
    const Rectangle d = detailRect();
    DrawRectangleRec(d, Color{ 24, 20, 50, 235 });
    DrawRectangleLinesEx(d, 2.0f, COL_EDGE);
    if (m_info.m_stock.empty()) return;
    const ShopStockInfo& s = m_info.m_stock[static_cast<size_t>(m_stockSel)];
    if (m_icons) m_icons->draw(s.m_id, Rectangle{ d.x + d.width / 2.0f - 48.0f, d.y + 16.0f, 96, 96 }, WHITE);
    drawCentered(s.m_name.c_str(), d.x + d.width / 2.0f, d.y + 124.0f, 24, COL_TITLE);
    drawCentered(categoryText(s.m_category), d.x + d.width / 2.0f, d.y + 154.0f, 13, COL_LABEL);
    drawWrapped(s.m_description, d.x + 20.0f, d.y + 184.0f, d.width - 40.0f, 18, COL_TEXT, 24);
    drawRight(TextFormat(u8"Giá: %d vàng", s.m_price), d.x + d.width - 20.0f, d.y + d.height - 106.0f, 22,
              m_info.m_gold >= s.m_price ? COL_GOLD : COL_ERR);
    UiText::draw(s.m_limit > 0 ? TextFormat(u8"Sở hữu: %d/%d", s.m_owned, s.m_limit) : TextFormat(u8"Sở hữu: %d", s.m_owned),
             static_cast<int>(d.x + 20), static_cast<int>(d.y + d.height - 104), 18, COL_LABEL);

    const bool soldOut = s.m_limit > 0 && s.m_owned >= s.m_limit;
    const bool afford = m_info.m_gold >= s.m_price;
    const char* label = soldOut ? u8"ĐÃ ĐỦ SỐ LƯỢNG" : (afford ? u8"MUA  (ENTER)" : u8"KHÔNG ĐỦ VÀNG");
    drawButton(buyButtonRect(), label, !soldOut && afford, CheckCollisionPointRec(UiCanvas::mousePosition(), buyButtonRect()), COL_EDGE_SEL);
}

void ShopScreen::drawBagDetail() const
{
    const Rectangle d = detailRect();
    DrawRectangleRec(d, Color{ 24, 20, 50, 235 });
    DrawRectangleLinesEx(d, 2.0f, COL_EDGE);

    const bool hasBag = !m_info.m_bag.empty();
    const ShopBagInfo* sel = hasBag ? &m_info.m_bag[static_cast<size_t>(m_bagSel)] : nullptr;
    if (sel) {
        if (m_icons) m_icons->draw(sel->m_id, Rectangle{ d.x + 14.0f, d.y + 12.0f, 64, 64 }, WHITE);
        UiText::draw(sel->m_name.c_str(), static_cast<int>(d.x + 90), static_cast<int>(d.y + 14), 22, COL_TITLE);
        drawWrapped(sel->m_description, d.x + 90.0f, d.y + 44.0f, d.width - 104.0f, 15, COL_TEXT, 19);
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
        DrawRectangleRec(r, hover && canEquip ? Color{ 52, 44, 92, 250 } : Color{ 28, 24, 58, 235 });
        DrawRectangleLinesEx(r, hover && canEquip ? 3.0f : 2.0f, hover && canEquip ? COL_EDGE_SEL : COL_EDGE);
        const std::string memberName = Strings::localizedName(m.m_name);
        UiText::draw(TextFormat("%d. %s", static_cast<int>(i) + 1, memberName.c_str()), static_cast<int>(r.x + 12), static_cast<int>(r.y + 8), 20, COL_TEXT);
        if (m.m_gearId.empty()) {
            UiText::draw(u8"(chưa đeo trang bị)", static_cast<int>(r.x + 12), static_cast<int>(r.y + 40), 16, COL_DIM);
        } else {
            if (m_icons) m_icons->draw(m.m_gearId, Rectangle{ r.x + 12, r.y + 34, 36, 36 }, WHITE);
            UiText::draw(m.m_gearName.c_str(), static_cast<int>(r.x + 56), static_cast<int>(r.y + 44), 16, COL_OK);
            drawButton(unequipRect(static_cast<int>(i)), "THÁO", true, CheckCollisionPointRec(mouse, unequipRect(static_cast<int>(i))), COL_EDGE);
        }
    }
    UiText::draw(canEquip ? u8"Chọn tướng để đeo trang bị đang chọn" : u8"Chọn trang bị trong danh sách để đeo cho tướng",
             static_cast<int>(d.x + 14), static_cast<int>(d.y + d.height - 26), 14, canEquip ? COL_OK : COL_DIM);
}
