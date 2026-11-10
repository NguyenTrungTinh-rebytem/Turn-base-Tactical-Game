#include "graphics/ShopScreen.h"
#include "graphics/UiBlur.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiIcons.h"
#include "graphics/UiInput.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiTheme.h"
#include "graphics/UiWidgets.h"
#include "graphics/Strings.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

namespace {
constexpr Color COL_TITLE = UiTheme::silver;
constexpr Color COL_LABEL = UiTheme::ashText;
constexpr Color COL_TEXT = UiTheme::silver;
constexpr Color COL_DIM = UiTheme::ashText;
constexpr Color COL_ERR = UiTheme::danger;
constexpr Color COL_OK = UiTheme::heal;
constexpr Color COL_GOLD = UiTheme::gold;
constexpr Color COL_AMBER = UiTheme::gold;
constexpr Color COL_PANEL = UiTheme::panel;
constexpr int VISIBLE_ROWS = 7;
constexpr float ROW_HEIGHT = 56.0f;
constexpr float SHOP_SCALE = 0.72f;
constexpr float u(float value) { return value * SHOP_SCALE; }

const char* categoryText(int category) {
    switch (category) {
    case 0: return u8"Tiêu hao";
    case 1: return u8"Tăng sức mạnh";
    default: return u8"Trang bị";
    }
}

std::string usageText(int category) {
    switch (category) {
    case 0: return u8"Dùng để hồi phục hoặc cứu đồng đội.";
    case 1: return u8"Dùng trong trận để nhận hiệu ứng trong thời hạn ghi bên dưới.";
    default: return u8"Trang bị trước trận; mỗi tướng mang tối đa một món.";
    }
}

std::string effectValue(ItemEffect effect, int value, int value2, int duration = 0) {
    switch (effect) {
    case ItemEffect::HEAL_HP_PCT: return TextFormat(u8"Hồi %d%% HP tối đa", value);
    case ItemEffect::RESTORE_MP_PCT: return TextFormat(u8"Hồi %d%% MP tối đa", value);
    case ItemEffect::REVIVE_PCT: return TextFormat(u8"Hồi sinh với %d%% HP", value);
    case ItemEffect::BOOST_DAMAGE_UP:
        return TextFormat(u8"Tăng %d%% sát thương · %d lượt", value, duration);
    case ItemEffect::BOOST_DAMAGE_REDUCE:
        return TextFormat(u8"Giảm %d%% sát thương nhận · %d lượt", value, duration);
    case ItemEffect::BOOST_EXTRA_TURN: return TextFormat(u8"Thêm %d lượt hành động", value);
    case ItemEffect::BOOST_CRIT_RATE: return TextFormat(u8"Tăng %d%% tỉ lệ chí mạng", value);
    case ItemEffect::HELD_REGEN: return TextFormat(u8"Hồi %d%% HP cuối lượt", value);
    case ItemEffect::HELD_LIFESTEAL: return TextFormat(u8"Hút %d%% sát thương thành HP", value);
    case ItemEffect::HELD_SURVIVE: return TextFormat(u8"Giữ lại %d HP một lần mỗi trận", value);
    case ItemEffect::HELD_DAMAGE_REDUCE: return TextFormat(u8"Giảm %d%% sát thương nhận", value);
    case ItemEffect::HELD_BERSERK:
        return TextFormat(u8"Tăng %d%% sát thương · mất %d%% HP", value, value2);
    case ItemEffect::HELD_EXECUTE:
        return TextFormat(u8"Tăng %d%% sát thương · kích hoạt dưới %d%% HP", value, value2);
    case ItemEffect::HELD_CRIT_DAMAGE:
        return TextFormat(u8"Tăng %d%% sát thương chí mạng", value);
    case ItemEffect::HELD_COUNTER:
        return TextFormat(u8"Phản lại %d%% sát thương cận chiến", value);
    }
    return {};
}

std::pair<std::string, Color> compareEffects(bool hasCurrent, ItemEffect currentEffect,
                                             int currentValue, int currentValue2,
                                             ItemEffect nextEffect, int nextValue,
                                             int nextValue2, int currentDuration = 0,
                                             int nextDuration = 0) {
    const std::string next = effectValue(nextEffect, nextValue, nextValue2, nextDuration);
    if (!hasCurrent) return { std::string(u8"Trống  →  ") + next, COL_OK };
    const std::string current = effectValue(currentEffect, currentValue, currentValue2, currentDuration);
    if (currentEffect != nextEffect)
        return { std::string(u8"Đổi hiệu ứng: ") + current + u8"  →  " + next, COL_LABEL };
    if (nextValue > currentValue)
        return { current + u8"  →  ▲ " + next, COL_OK };
    if (nextValue < currentValue)
        return { current + u8"  →  ▼ " + next, COL_ERR };
    return { current + u8"  →  " + next, COL_LABEL };
}

int drawWrapped(const std::string& text, float x, float y, float width,
                int fontSize, int lineHeight, Color color, int maxLines = 5) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    int lines = 0;
    while (words >> word) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && UiText::measure(candidate, fontSize) > width) {
            UiText::draw(line, static_cast<int>(x), static_cast<int>(y + lines * lineHeight),
                         fontSize, color);
            if (++lines >= maxLines) return lines;
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty() && lines < maxLines) {
        UiText::draw(line, static_cast<int>(x), static_cast<int>(y + lines * lineHeight),
                     fontSize, color);
        ++lines;
    }
    return lines;
}

void drawPill(Rectangle bounds, const std::string& label, Color fill, Color text) {
    UiShapes::bladeStrip(bounds, fill, UiTheme::ash, false);
    int fontSize = 13;
    while (fontSize > 9 && UiText::measure(label, fontSize) > bounds.width - u(10.0f))
        --fontSize;
    const int width = UiText::measure(label, fontSize);
    UiText::draw(label, static_cast<int>(bounds.x + (bounds.width - width) * 0.5f),
                 static_cast<int>(bounds.y + (bounds.height - fontSize) * 0.5f), fontSize, text);
}

void drawPanel(Rectangle bounds) {
    UiShapes::plate(bounds, COL_PANEL, UiTheme::ash, 1.0f);
}
}

ShopScreen::~ShopScreen() {
    unloadAll();
}

void ShopScreen::unloadAll() {
    for (int index = 0; index < 3; ++index) {
        if (m_memberPortraits[index].id != 0 && IsWindowReady())
            UnloadTexture(m_memberPortraits[index]);
        m_memberPortraits[index] = Texture2D{};
        m_memberPortraitPaths[index].clear();
    }
}

void ShopScreen::setInfo(const ShopInfo& info) {
    if (!m_hasGold) {
        m_displayGold = static_cast<float>(info.m_gold);
        m_goldStart = m_displayGold;
        m_hasGold = true;
    } else if (info.m_gold != m_info.m_gold) {
        m_goldStart = m_displayGold;
        m_goldDelta = info.m_gold - m_info.m_gold;
        m_goldTween = 0.0f;
        m_goldDeltaTime = m_goldDeltaDuration;
    }
    m_info = info;
    for (int index = 0; index < 3; ++index) {
        const std::string path = static_cast<std::size_t>(index) < m_info.m_members.size()
            ? m_info.m_members[static_cast<std::size_t>(index)].m_portraitPath : std::string{};
        if (path == m_memberPortraitPaths[index]) continue;
        if (m_memberPortraits[index].id != 0 && IsWindowReady())
            UnloadTexture(m_memberPortraits[index]);
        m_memberPortraits[index] = Texture2D{};
        m_memberPortraitPaths[index] = path;
        if (!path.empty() && IsWindowReady() && FileExists(path.c_str())) {
            m_memberPortraits[index] = LoadTexture(path.c_str());
            if (m_memberPortraits[index].id != 0)
                SetTextureFilter(m_memberPortraits[index], TEXTURE_FILTER_POINT);
        }
    }
    clampSelection();
}

void ShopScreen::open(int tab) {
    m_tab = std::max(0, std::min(tab, static_cast<int>(TAB_COUNT) - 1));
    m_stockSel = m_bagSel = 0;
    m_filter[TAB_SHOP] = m_filter[TAB_BAG] = FILTER_ALL;
    m_scroll[TAB_SHOP] = m_scroll[TAB_BAG] = 0;
    m_selectedMember = m_actionMember = 0;
    m_draggingScrollbar = false;
    m_message.clear();
    m_messageTime = 0.0f;
}

void ShopScreen::setMessage(const std::string& text, bool ok) {
    m_message = text;
    m_messageOk = ok;
    m_messageTime = ok ? 1.5f : 2.5f;
}

void ShopScreen::update(float deltaTime) {
    m_time += deltaTime;
    m_messageTime = std::max(0.0f, m_messageTime - deltaTime);
    m_goldDeltaTime = std::max(0.0f, m_goldDeltaTime - deltaTime);
    if (m_goldTween < 1.0f) {
        m_goldTween = std::min(1.0f, m_goldTween + deltaTime / 0.35f);
        const float eased = 1.0f - (1.0f - m_goldTween) * (1.0f - m_goldTween);
        m_displayGold = m_goldStart + (static_cast<float>(m_info.m_gold) - m_goldStart) * eased;
    }
}

Rectangle ShopScreen::panelRect() const {
    return { 210.0f, 105.0f, 860.0f, 510.0f };
}

Rectangle ShopScreen::tabRect(int index) const {
    const Rectangle panel = panelRect();
    return { panel.x + u(24.0f + index * 284.0f), panel.y + u(62.0f), u(276.0f), u(42.0f) };
}

Rectangle ShopScreen::filterRect(int index) const {
    const Rectangle panel = panelRect();
    return { panel.x + u(24.0f + index * 122.0f), panel.y + u(116.0f), u(114.0f), u(28.0f) };
}

Rectangle ShopScreen::listRect() const {
    const Rectangle panel = panelRect();
    return { panel.x + u(24.0f), panel.y + u(160.0f), u(650.0f), u(ROW_HEIGHT * VISIBLE_ROWS) };
}

Rectangle ShopScreen::rowRect(int visibleRow) const {
    const Rectangle list = listRect();
    return { list.x, list.y + u(static_cast<float>(visibleRow) * ROW_HEIGHT),
             list.width - u(14.0f), u(ROW_HEIGHT - 4.0f) };
}

Rectangle ShopScreen::detailRect() const {
    const Rectangle panel = panelRect();
    const Rectangle list = listRect();
    return { list.x + list.width + u(20.0f), panel.y + u(116.0f),
             panel.x + panel.width - u(24.0f) - (list.x + list.width + u(20.0f)),
             panel.height - u(230.0f) };
}

Rectangle ShopScreen::buyButtonRect() const {
    const Rectangle detail = detailRect();
    return { detail.x + u(18.0f), detail.y + detail.height - u(54.0f),
             detail.width - u(36.0f), u(44.0f) };
}

Rectangle ShopScreen::memberRect(int index, bool shopTab) const {
    const Rectangle detail = detailRect();
    const float top = detail.y + u(shopTab ? 280.0f : 14.0f);
    const float width = (detail.width - u(36.0f + 16.0f)) / 3.0f;
    return { detail.x + u(18.0f) + index * (width + u(8.0f)), top, width, u(42.0f) };
}

Rectangle ShopScreen::equipButtonRect() const {
    const Rectangle detail = detailRect();
    return { detail.x + u(18.0f), detail.y + u(350.0f),
             detail.width - u(36.0f), u(44.0f) };
}

Rectangle ShopScreen::unequipButtonRect() const {
    const Rectangle detail = detailRect();
    return { detail.x + detail.width - u(106.0f), detail.y + u(230.0f), u(88.0f), u(34.0f) };
}

Rectangle ShopScreen::scrollbarTrackRect() const {
    const Rectangle list = listRect();
    return { list.x + list.width - u(10.0f), list.y, u(8.0f), list.height };
}

Rectangle ShopScreen::scrollbarThumbRect() const {
    const Rectangle track = scrollbarTrackRect();
    const std::vector<int> indices = visibleIndices();
    const int maxScroll = std::max(0, static_cast<int>(indices.size()) - VISIBLE_ROWS);
    if (maxScroll == 0) return track;
    const float thumbHeight = std::max(36.0f, track.height *
        static_cast<float>(VISIBLE_ROWS) / static_cast<float>(indices.size()));
    const float travel = track.height - thumbHeight;
    const float offset = travel * static_cast<float>(m_scroll[m_tab]) / static_cast<float>(maxScroll);
    return { track.x, track.y + offset, track.width, thumbHeight };
}

Rectangle ShopScreen::closeRect() const {
    const Rectangle panel = panelRect();
    return { panel.x + panel.width - u(176.0f), panel.y + panel.height - u(54.0f),
             u(152.0f), u(40.0f) };
}

std::vector<int> ShopScreen::visibleIndices() const {
    std::vector<int> indices;
    if (m_tab == TAB_SHOP) {
        for (std::size_t index = 0; index < m_info.m_stock.size(); ++index) {
            if (m_filter[TAB_SHOP] == FILTER_ALL ||
                m_info.m_stock[index].m_category == m_filter[TAB_SHOP] - 1)
                indices.push_back(static_cast<int>(index));
        }
    } else {
        for (std::size_t index = 0; index < m_info.m_bag.size(); ++index) {
            if (m_filter[TAB_BAG] == FILTER_ALL ||
                m_info.m_bag[index].m_category == m_filter[TAB_BAG] - 1)
                indices.push_back(static_cast<int>(index));
        }
    }
    return indices;
}

int& ShopScreen::selection() {
    return m_tab == TAB_SHOP ? m_stockSel : m_bagSel;
}

int ShopScreen::selection() const {
    return m_tab == TAB_SHOP ? m_stockSel : m_bagSel;
}

int ShopScreen::selectionPosition(const std::vector<int>& indices) const {
    const auto found = std::find(indices.begin(), indices.end(), selection());
    return found == indices.end() ? 0 : static_cast<int>(found - indices.begin());
}

void ShopScreen::clampSelection() {
    for (int tab = 0; tab < TAB_COUNT; ++tab) {
        const int count = static_cast<int>(tab == TAB_SHOP ? m_info.m_stock.size() : m_info.m_bag.size());
        int& selected = tab == TAB_SHOP ? m_stockSel : m_bagSel;
        selected = std::max(0, std::min(selected, std::max(0, count - 1)));
    }
    const std::vector<int> indices = visibleIndices();
    if (!indices.empty() && std::find(indices.begin(), indices.end(), selection()) == indices.end())
        selection() = indices.front();
    m_scroll[m_tab] = std::max(0, std::min(m_scroll[m_tab],
        std::max(0, static_cast<int>(indices.size()) - VISIBLE_ROWS)));
}

void ShopScreen::ensureVisible() {
    const std::vector<int> indices = visibleIndices();
    if (indices.empty()) {
        m_scroll[m_tab] = 0;
        return;
    }
    int& scroll = m_scroll[m_tab];
    const int position = selectionPosition(indices);
    if (position < scroll) scroll = position;
    if (position >= scroll + VISIBLE_ROWS) scroll = position - VISIBLE_ROWS + 1;
    scroll = std::max(0, std::min(scroll,
        std::max(0, static_cast<int>(indices.size()) - VISIBLE_ROWS)));
}

bool ShopScreen::selectedShopItemUnavailable(std::string* reason) const {
    if (m_stockSel < 0 || static_cast<std::size_t>(m_stockSel) >= m_info.m_stock.size()) {
        if (reason) *reason = u8"Không có vật phẩm để mua.";
        return true;
    }
    const ShopStockInfo& item = m_info.m_stock[static_cast<std::size_t>(m_stockSel)];
    if (item.m_limit > 0 && item.m_owned >= item.m_limit) {
        if (reason) *reason = u8"Đã đạt tối đa số lượng mang theo.";
        return true;
    }
    if (m_info.m_gold < item.m_price) {
        if (reason) *reason = u8"Không đủ vàng.";
        return true;
    }
    return false;
}

ShopAction ShopScreen::handleInput() {
    const Vector2 mouse = UiInput::mousePosition();
    clampSelection();
    const Rectangle panel = panelRect();
    const Rectangle topClose{ panel.x + panel.width - u(54.0f), panel.y + u(14.0f),
                              u(36.0f), u(36.0f) };

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B)) return ShopAction::CLOSE;
    if (IsKeyPressed(KEY_ONE)) m_tab = TAB_SHOP;
    if (IsKeyPressed(KEY_TWO)) m_tab = TAB_BAG;
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E))
        m_tab = (m_tab + 1) % TAB_COUNT;

    for (int tab = 0; tab < TAB_COUNT; ++tab) {
        if (UiInput::pressed() && CheckCollisionPointRec(mouse, tabRect(tab))) {
            m_tab = tab;
            clampSelection();
        }
    }

    const std::vector<int> indices = visibleIndices();
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        if (!indices.empty()) {
            const int position = std::min(selectionPosition(indices) + 1,
                                           static_cast<int>(indices.size()) - 1);
            selection() = indices[static_cast<std::size_t>(position)];
            ensureVisible();
        }
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        if (!indices.empty()) {
            const int position = std::max(0, selectionPosition(indices) - 1);
            selection() = indices[static_cast<std::size_t>(position)];
            ensureVisible();
        }
    }
    if (UiInput::wheel() != 0.0f && CheckCollisionPointRec(mouse, listRect())) {
        m_scroll[m_tab] -= static_cast<int>(std::lround(UiInput::wheel() * 2.0f));
        m_scroll[m_tab] = std::max(0, std::min(m_scroll[m_tab],
            std::max(0, static_cast<int>(indices.size()) - VISIBLE_ROWS)));
    }

    bool hasCategory[3] = { false, false, false };
    const int sourceCount = static_cast<int>(m_tab == TAB_SHOP
        ? m_info.m_stock.size() : m_info.m_bag.size());
    for (int index = 0; index < sourceCount; ++index) {
        const int category = m_tab == TAB_SHOP
            ? m_info.m_stock[static_cast<std::size_t>(index)].m_category
            : m_info.m_bag[static_cast<std::size_t>(index)].m_category;
        if (category >= 0 && category < 3) hasCategory[category] = true;
    }
    for (int filter = 0; filter < FILTER_COUNT; ++filter) {
        const bool available = filter == FILTER_ALL || hasCategory[filter - 1];
        if (available && UiInput::pressed() &&
            CheckCollisionPointRec(mouse, filterRect(filter))) {
            m_filter[m_tab] = filter;
            m_scroll[m_tab] = 0;
            clampSelection();
        }
    }

    const Rectangle track = scrollbarTrackRect();
    const Rectangle thumb = scrollbarThumbRect();
    if (UiInput::pressed() && CheckCollisionPointRec(mouse, thumb) &&
        static_cast<int>(indices.size()) > VISIBLE_ROWS) {
        m_draggingScrollbar = true;
        m_scrollDragOffset = mouse.y - thumb.y;
    } else if (UiInput::pressed() && CheckCollisionPointRec(mouse, track) &&
               static_cast<int>(indices.size()) > VISIBLE_ROWS) {
        const float thumbHeight = thumb.height;
        const float trackTravel = std::max(1.0f, track.height - thumbHeight);
        const float fraction = std::clamp((mouse.y - track.y - thumbHeight * 0.5f) / trackTravel,
                                          0.0f, 1.0f);
        m_scroll[m_tab] = static_cast<int>(std::lround(
            fraction * (static_cast<int>(indices.size()) - VISIBLE_ROWS)));
        m_draggingScrollbar = true;
        m_scrollDragOffset = thumbHeight * 0.5f;
    }
    if (m_draggingScrollbar && UiInput::down()) {
        const Rectangle currentTrack = scrollbarTrackRect();
        const Rectangle currentThumb = scrollbarThumbRect();
        const float travel = std::max(1.0f, currentTrack.height - currentThumb.height);
        const float fraction = std::clamp((mouse.y - currentTrack.y - m_scrollDragOffset) / travel,
                                          0.0f, 1.0f);
        m_scroll[m_tab] = static_cast<int>(std::lround(
            fraction * (static_cast<int>(indices.size()) - VISIBLE_ROWS)));
    }
    if (UiInput::released()) m_draggingScrollbar = false;

    const int first = std::min(m_scroll[m_tab], std::max(0, static_cast<int>(indices.size()) - 1));
    for (int row = 0; row < VISIBLE_ROWS && first + row < static_cast<int>(indices.size()); ++row) {
        if (UiInput::pressed() && CheckCollisionPointRec(mouse, rowRect(row)))
            selection() = indices[static_cast<std::size_t>(first + row)];
    }

    for (int member = 0; member < static_cast<int>(m_info.m_members.size()) && member < 3; ++member) {
        if (UiInput::pressed() && CheckCollisionPointRec(mouse, memberRect(member, m_tab == TAB_SHOP)))
            m_selectedMember = member;
    }

    if (UiInput::pressed() && CheckCollisionPointRec(mouse, topClose))
        return ShopAction::CLOSE;
    if (UiInput::pressed() && CheckCollisionPointRec(mouse, closeRect()))
        return ShopAction::CLOSE;

    if (m_tab == TAB_SHOP) {
        std::string reason;
        const bool unavailable = selectedShopItemUnavailable(&reason);
        if (unavailable && UiInput::pressed() &&
            CheckCollisionPointRec(mouse, buyButtonRect()))
            setMessage(reason, false);
        if (!unavailable && ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
                              IsKeyPressed(KEY_SPACE)) ||
            (UiInput::pressed() && CheckCollisionPointRec(mouse, buyButtonRect()))))
            return ShopAction::BUY;
    } else {
        const bool hasItem = m_bagSel >= 0 &&
            static_cast<std::size_t>(m_bagSel) < m_info.m_bag.size();
        const bool canEquip = hasItem && m_info.m_bag[static_cast<std::size_t>(m_bagSel)].m_category == 2;
        if (canEquip && ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
                          IsKeyPressed(KEY_SPACE)) ||
            (UiInput::pressed() && CheckCollisionPointRec(mouse, equipButtonRect())))) {
            if (m_selectedMember < static_cast<int>(m_info.m_members.size())) {
                m_actionMember = m_selectedMember;
                return ShopAction::EQUIP;
            }
        }
        if (m_selectedMember < static_cast<int>(m_info.m_members.size()) &&
            !m_info.m_members[static_cast<std::size_t>(m_selectedMember)].m_gearId.empty() &&
            UiInput::pressed() && CheckCollisionPointRec(mouse, unequipButtonRect())) {
            m_actionMember = m_selectedMember;
            return ShopAction::UNEQUIP;
        }
    }
    return ShopAction::NONE;
}

void ShopScreen::draw() const {
    const Rectangle panel = panelRect();
    const Rectangle closeIcon{ panel.x + panel.width - u(54.0f), panel.y + u(14.0f), u(36.0f), u(36.0f) };
    DrawRectangle(0, 0, UiCanvas::width(), UiCanvas::height(), Color{ 0, 0, 0, 150 });
    UiBlur::draw(panel, UiTheme::radiusMedium, Color{ 16, 20, 38, 238 });
    UiShapes::plate(panel, Color{ 11, 9, 15, 248 }, UiTheme::silver, 1.5f);

    UiText::draw(Strings::shopTitle, static_cast<int>(panel.x + u(24.0f)),
                 static_cast<int>(panel.y + u(19.0f)), 22, COL_TITLE);
    const Rectangle goldChip{ panel.x + panel.width - u(242.0f), panel.y + u(14.0f), u(174.0f), u(40.0f) };
    UiShapes::bladeStrip(goldChip, Color{ 31, 25, 16, 255 }, COL_GOLD, false);
    DrawCircle(static_cast<int>(goldChip.x + u(20.0f)), static_cast<int>(goldChip.y + u(20.0f)),
               u(9.0f), COL_GOLD);
    DrawCircleLines(static_cast<int>(goldChip.x + u(20.0f)), static_cast<int>(goldChip.y + u(20.0f)),
                    u(9.0f), Color{ 120, 80, 10, 255 });
    const std::string goldText = TextFormat("%d", static_cast<int>(std::round(m_displayGold)));
    UiText::draw(goldText, static_cast<int>(goldChip.x + u(38.0f)),
                 static_cast<int>(goldChip.y + u(9.0f)), 16, COL_GOLD);
    if (m_goldDeltaTime > 0.0f && m_goldDelta != 0) {
        const float progress = 1.0f - m_goldDeltaTime / m_goldDeltaDuration;
        const Color feedbackColor = m_goldDelta > 0 ? COL_OK : COL_ERR;
        const std::string delta = (m_goldDelta > 0 ? "+" : "−") +
            std::to_string(std::abs(m_goldDelta));
        UiText::draw(delta, static_cast<int>(goldChip.x + u(114.0f)),
                     static_cast<int>(goldChip.y - u(7.0f + progress * 12.0f)), 12,
                     Color{ feedbackColor.r, feedbackColor.g, feedbackColor.b,
                            static_cast<unsigned char>(255.0f * (1.0f - progress)) });
    }
    UiWidgets::IconButton(closeIcon, "action:close");
    const char* tabs[TAB_COUNT] = { u8"Cửa hàng", u8"Kho đồ / Trang bị" };
    for (int tab = 0; tab < TAB_COUNT; ++tab) {
        const Rectangle bounds = tabRect(tab);
        const bool active = m_tab == tab;
        const bool hover = CheckCollisionPointRec(UiInput::mousePosition(), bounds);
        UiShapes::angledTab(bounds,
            active ? Color{ 50, 10, 17, 224 } :
                (hover ? UiTheme::panelRaised : UiTheme::panel),
            active ? UiTheme::crimson : UiTheme::ash, active, hover);
        const int width = UiText::measure(tabs[tab], 14);
        UiText::draw(tabs[tab], static_cast<int>(bounds.x + (bounds.width - width) * 0.5f),
                     static_cast<int>(bounds.y + (bounds.height - 14.0f) * 0.5f), 14,
                     active ? COL_TITLE : COL_TEXT);
    }
    bool categories[3] = { false, false, false };
    for (std::size_t index = 0; index < (m_tab == TAB_SHOP
            ? m_info.m_stock.size() : m_info.m_bag.size()); ++index) {
        const int category = m_tab == TAB_SHOP
            ? m_info.m_stock[index].m_category : m_info.m_bag[index].m_category;
        if (category >= 0 && category < 3) categories[category] = true;
    }
    const char* filterNames[FILTER_COUNT] = {
        u8"Tất cả", u8"Tiêu hao", u8"Tăng sức mạnh", u8"Trang bị"
    };
    for (int filter = 0; filter < FILTER_COUNT; ++filter) {
        if (filter > FILTER_ALL && !categories[filter - 1]) continue;
        const Rectangle bounds = filterRect(filter);
        const bool active = m_filter[m_tab] == filter;
        const bool hover = CheckCollisionPointRec(UiInput::mousePosition(), bounds);
        UiShapes::bladeStrip(bounds,
            active ? Color{ 50, 10, 17, 224 } :
                (hover ? UiTheme::panelRaised : UiTheme::panel),
            active ? UiTheme::crimson : UiTheme::ash, false, active, hover);
        const int width = UiText::measure(filterNames[filter], 10);
        UiText::draw(filterNames[filter],
            static_cast<int>(bounds.x + (bounds.width - width) * 0.5f),
            static_cast<int>(bounds.y + (bounds.height - 10.0f) * 0.5f), 10,
            active ? COL_TITLE : COL_TEXT);
    }
    drawList();
    if (m_tab == TAB_SHOP) drawShopDetail();
    else drawBagDetail();

    const Rectangle close = closeRect();
    UiWidgets::Button(close, u8"Đóng");
    if (m_messageTime > 0.0f && !m_message.empty()) {
        const Rectangle toast{ panel.x + u(394.0f), panel.y + panel.height - u(48.0f),
                                u(440.0f), u(34.0f) };
        UiShapes::plate(toast,
            m_messageOk ? Color{ 12, 41, 28, 248 } : Color{ 55, 14, 20, 248 },
            m_messageOk ? UiTheme::heal : UiTheme::danger, 1.0f);
        int fontSize = 11;
        while (fontSize > 8 && UiText::measure(m_message, fontSize) > toast.width - u(24.0f))
            --fontSize;
        const int width = UiText::measure(m_message, fontSize);
        UiText::draw(m_message, static_cast<int>(toast.x + (toast.width - width) * 0.5f),
                     static_cast<int>(toast.y + (toast.height - fontSize) * 0.5f), fontSize,
                     m_messageOk ? COL_OK : COL_ERR);
    }
}

void ShopScreen::drawList() const {
    const Rectangle list = listRect();
    drawPanel(list);
    const std::vector<int> indices = visibleIndices();
    if (indices.empty()) {
        const std::string empty = m_tab == TAB_SHOP ? u8"Cửa hàng đang trống" : u8"Kho đồ trống";
        const int width = UiText::measure(empty, 18);
        UiText::draw(empty, static_cast<int>(list.x + (list.width - width) * 0.5f),
                     static_cast<int>(list.y + (list.height - 18.0f) * 0.5f), 18, COL_DIM);
        return;
    }

    const Vector2 mouse = UiInput::mousePosition();
    for (int rowIndex = 0; rowIndex < VISIBLE_ROWS; ++rowIndex) {
        const int position = m_scroll[m_tab] + rowIndex;
        if (position >= static_cast<int>(indices.size())) break;
        const int index = indices[static_cast<std::size_t>(position)];
        const Rectangle row = rowRect(rowIndex);
        const bool selected = index == selection();
        const bool hover = CheckCollisionPointRec(mouse, row);
        const std::string id = m_tab == TAB_SHOP
            ? m_info.m_stock[static_cast<std::size_t>(index)].m_id
            : m_info.m_bag[static_cast<std::size_t>(index)].m_id;
        const std::string name = m_tab == TAB_SHOP
            ? m_info.m_stock[static_cast<std::size_t>(index)].m_name
            : m_info.m_bag[static_cast<std::size_t>(index)].m_name;
        const int category = m_tab == TAB_SHOP
            ? m_info.m_stock[static_cast<std::size_t>(index)].m_category
            : m_info.m_bag[static_cast<std::size_t>(index)].m_category;
        const std::string detail = categoryText(category);
        UiWidgets::ListRow(row, name, detail, selected, true, u(58.0f));
        if (m_icons)
            m_icons->draw(id, Rectangle{ row.x + u(8.0f), row.y + u(5.0f), u(42.0f), u(42.0f) }, WHITE);
        else
            UiIcons::draw("item:" + id,
                Rectangle{ row.x + u(8.0f), row.y + u(5.0f), u(42.0f), u(42.0f) });

        if (m_tab == TAB_SHOP) {
            const ShopStockInfo& item = m_info.m_stock[static_cast<std::size_t>(index)];
            const bool maxed = item.m_limit > 0 && item.m_owned >= item.m_limit;
            const bool affordable = m_info.m_gold >= item.m_price;
            if (maxed) {
                drawPill(Rectangle{ row.x + row.width - u(118.0f), row.y + u(6.0f),
                                    u(106.0f), u(20.0f) },
                         u8"Đã đạt tối đa", Color{ 82, 57, 21, 255 }, COL_AMBER);
            } else {
                const Color priceColor = affordable ? COL_GOLD : COL_ERR;
                drawPill(Rectangle{ row.x + row.width - u(108.0f), row.y + u(5.0f),
                                    u(96.0f), u(22.0f) },
                         TextFormat("%d vàng", item.m_price),
                         affordable ? Color{ 60, 49, 23, 255 } : Color{ 78, 35, 42, 255 },
                         priceColor);
            }
            const std::string owned = item.m_limit > 0
                ? TextFormat(u8"Có %d/%d", item.m_owned, item.m_limit)
                : TextFormat(u8"Có %d", item.m_owned);
            const int ownedWidth = UiText::measure(owned, 12);
            UiText::draw(owned, static_cast<int>(row.x + row.width - ownedWidth - u(14.0f)),
                         static_cast<int>(row.y + u(31.0f)), 12, maxed ? COL_AMBER : COL_LABEL);
            std::string reason;
            const bool unavailable =
                (item.m_limit > 0 && item.m_owned >= item.m_limit) || m_info.m_gold < item.m_price;
            if (hover && unavailable) {
                reason = item.m_limit > 0 && item.m_owned >= item.m_limit
                    ? u8"Đã đạt tối đa" : u8"Không đủ vàng";
                UiWidgets::Tooltip(row, reason);
            } else {
                UiWidgets::Tooltip(row, item.m_description);
            }
        } else {
            const ShopBagInfo& item = m_info.m_bag[static_cast<std::size_t>(index)];
            drawPill(Rectangle{ row.x + row.width - u(64.0f), row.y + u(14.0f),
                                u(52.0f), u(24.0f) },
                     TextFormat(u8"×%d", item.m_count), Color{ 27, 68, 51, 255 }, COL_OK);
            UiWidgets::Tooltip(row, item.m_description);
        }
    }

    const Rectangle track = scrollbarTrackRect();
    UiShapes::plate(track, UiTheme::ink, BLANK);
    const Rectangle thumb = scrollbarThumbRect();
    UiShapes::bladeStrip(thumb, m_draggingScrollbar ? UiTheme::crimson : UiTheme::ash,
                         UiTheme::silver, false, m_draggingScrollbar);
}

void ShopScreen::drawMemberSelector(bool shopTab, const Rectangle& bounds) const {
    for (std::size_t index = 0; index < m_info.m_members.size() && index < 3; ++index) {
        const Rectangle memberBounds = memberRect(static_cast<int>(index), shopTab);
        const bool selected = static_cast<int>(index) == m_selectedMember;
        const bool hover = CheckCollisionPointRec(UiInput::mousePosition(), memberBounds);
        const Color cardFill = selected ? Color{ 50, 10, 17, 224 }
            : (hover ? UiTheme::panelRaised : UiTheme::panel);
        UiShapes::bladeStrip(memberBounds, cardFill,
            selected ? UiTheme::crimson : UiTheme::ash, false, selected, hover);
        const Rectangle hex{ memberBounds.x + u(5.0f), memberBounds.y + u(5.0f),
                             u(32.0f), u(32.0f) };
        const Vector2 center{ hex.x + hex.width * 0.5f, hex.y + hex.height * 0.5f };
        if (m_memberPortraits[index].id != 0) {
            // Anh dai nhieu khung (sprite) -> chi lay khung vuong dau tien; anh vuong (chan dung) -> ve ca anh
            const float texW = static_cast<float>(m_memberPortraits[index].width);
            const float texH = static_cast<float>(m_memberPortraits[index].height);
            const float srcW = (texW > texH * 1.5f) ? texH : texW;
            DrawTexturePro(m_memberPortraits[index],
                Rectangle{ 0.0f, 0.0f, srcW, texH },
                hex, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
            DrawTriangle(Vector2{ hex.x, hex.y }, Vector2{ hex.x + hex.width * 0.25f, hex.y },
                         Vector2{ hex.x, hex.y + hex.height * 0.5f }, cardFill);
            DrawTriangle(Vector2{ hex.x + hex.width, hex.y },
                         Vector2{ hex.x + hex.width * 0.75f, hex.y },
                         Vector2{ hex.x + hex.width, hex.y + hex.height * 0.5f }, cardFill);
            DrawTriangle(Vector2{ hex.x, hex.y + hex.height },
                         Vector2{ hex.x + hex.width * 0.25f, hex.y + hex.height },
                         Vector2{ hex.x, hex.y + hex.height * 0.5f }, cardFill);
            DrawTriangle(Vector2{ hex.x + hex.width, hex.y + hex.height },
                         Vector2{ hex.x + hex.width * 0.75f, hex.y + hex.height },
                         Vector2{ hex.x + hex.width, hex.y + hex.height * 0.5f }, cardFill);
            const Color border = selected ? COL_TITLE : Color{ 107, 124, 160, 255 };
            const Vector2 points[] = {
                { hex.x + hex.width * 0.25f, hex.y }, { hex.x + hex.width * 0.75f, hex.y },
                { hex.x + hex.width, hex.y + hex.height * 0.5f },
                { hex.x + hex.width * 0.75f, hex.y + hex.height },
                { hex.x + hex.width * 0.25f, hex.y + hex.height },
                { hex.x, hex.y + hex.height * 0.5f },
                { hex.x + hex.width * 0.25f, hex.y }
            };
            for (int edge = 0; edge < 6; ++edge)
                DrawLineEx(points[edge], points[edge + 1], 1.5f, border);
        } else {
            DrawPoly(center, 6, hex.width * 0.5f, 0.0f,
                     selected ? COL_TITLE : Color{ 107, 124, 160, 255 });
        }
        const std::string name = Strings::localizedName(m_info.m_members[index].m_name);
        const std::string initial = name.empty() ? "?" : name.substr(0, 1);
        if (m_memberPortraits[index].id == 0) {
            const int initialWidth = UiText::measure(initial, 11);
            UiText::draw(initial, static_cast<int>(hex.x + (hex.width - initialWidth) * 0.5f),
                         static_cast<int>(hex.y + (hex.height - 11.0f) * 0.5f), 11,
                         Color{ 22, 25, 38, 255 });
        }
        UiText::draw(name, static_cast<int>(memberBounds.x + u(42.0f)),
                     static_cast<int>(memberBounds.y + (memberBounds.height - 10.0f) * 0.5f),
                     11, selected ? COL_TITLE : COL_TEXT);
    }
    (void)bounds;
}

void ShopScreen::drawShopDetail() const {
    const Rectangle detail = detailRect();
    drawPanel(detail);
    if (m_stockSel < 0 || static_cast<std::size_t>(m_stockSel) >= m_info.m_stock.size()) {
        UiText::draw(u8"Chọn vật phẩm để xem chi tiết", static_cast<int>(detail.x + u(20.0f)),
                     static_cast<int>(detail.y + u(24.0f)), 14, COL_DIM);
        return;
    }
    const ShopStockInfo& item = m_info.m_stock[static_cast<std::size_t>(m_stockSel)];
    if (m_icons) m_icons->draw(item.m_id,
        Rectangle{ detail.x + u(18.0f), detail.y + u(17.0f), u(64.0f), u(64.0f) }, WHITE);
    else UiIcons::draw("item:" + item.m_id,
        Rectangle{ detail.x + u(18.0f), detail.y + u(17.0f), u(64.0f), u(64.0f) });
    UiText::draw(item.m_name, static_cast<int>(detail.x + u(96.0f)),
                 static_cast<int>(detail.y + u(18.0f)), 16, COL_TITLE);
    drawPill(Rectangle{ detail.x + u(96.0f), detail.y + u(51.0f), u(126.0f), u(24.0f) },
             categoryText(item.m_category), Color{ 36, 47, 66, 255 }, COL_LABEL);

    const int descriptionLines = drawWrapped(item.m_description, detail.x + u(18.0f),
        detail.y + u(96.0f), detail.width - u(36.0f), 12, 17, COL_TEXT, 3);
    const float noteY = detail.y + u(104.0f) + std::max(1, descriptionLines) * 17.0f;
    UiShapes::plate(Rectangle{ detail.x + u(14.0f), noteY, detail.width - u(28.0f), u(42.0f) },
                    UiTheme::panelRaised, BLANK);
    drawWrapped(usageText(item.m_category), detail.x + u(24.0f), noteY + u(7.0f),
                detail.width - u(48.0f), 10, 13, COL_LABEL, 2);

    const float ownedY = detail.y + u(235.0f);
    UiText::draw(item.m_limit > 0
        ? TextFormat(u8"Sở hữu %d/%d", item.m_owned, item.m_limit)
        : TextFormat(u8"Sở hữu %d", item.m_owned),
        static_cast<int>(detail.x + u(18.0f)), static_cast<int>(ownedY), 13,
        item.m_limit > 0 && item.m_owned >= item.m_limit ? COL_AMBER : COL_TEXT);
    const Color priceColor = m_info.m_gold >= item.m_price ? COL_GOLD : COL_ERR;
    UiText::draw(TextFormat(u8"%d vàng", item.m_price),
        static_cast<int>(detail.x + detail.width - u(116.0f)), static_cast<int>(ownedY), 14, priceColor);

    if (item.m_category == 2) {
        UiText::draw(u8"So sánh trang bị", static_cast<int>(detail.x + u(18.0f)),
                     static_cast<int>(detail.y + u(254.0f)), 11, COL_LABEL);
        drawMemberSelector(true, Rectangle{ detail.x + u(18.0f), detail.y + u(280.0f),
                                             detail.width - u(36.0f), u(42.0f) });
        const ShopMemberInfo* member = m_selectedMember < static_cast<int>(m_info.m_members.size())
            ? &m_info.m_members[static_cast<std::size_t>(m_selectedMember)] : nullptr;
        const Rectangle compare{ detail.x + u(18.0f), detail.y + u(330.0f),
                                 detail.width - u(36.0f), u(54.0f) };
        UiShapes::plate(compare, UiTheme::panelRaised, BLANK);
        if (member) {
            const auto comparison = compareEffects(!member->m_gearId.empty(),
                member->m_gearEffect, member->m_gearValue, member->m_gearValue2,
                item.m_effect, item.m_value, item.m_value2,
                member->m_gearDuration, item.m_duration);
            drawWrapped(comparison.first, compare.x + u(10.0f), compare.y + u(8.0f),
                        compare.width - u(20.0f), 10, 13, comparison.second, 2);
        } else {
            UiText::draw(u8"Chọn tướng để so sánh hiệu ứng", static_cast<int>(compare.x + u(10.0f)),
                         static_cast<int>(compare.y + u(18.0f)), 10, COL_DIM);
        }
    } else {
        const Rectangle effect{ detail.x + u(18.0f), detail.y + u(254.0f),
                                detail.width - u(36.0f), u(54.0f) };
        UiShapes::plate(effect, UiTheme::panelRaised, BLANK);
        UiText::draw(u8"Hiệu quả", static_cast<int>(effect.x + u(10.0f)),
                     static_cast<int>(effect.y + u(7.0f)), 10, COL_LABEL);
        UiText::draw(effectValue(item.m_effect, item.m_value, item.m_value2, item.m_duration),
                     static_cast<int>(effect.x + u(10.0f)), static_cast<int>(effect.y + u(27.0f)),
                     11, COL_TEXT);
    }
    if (m_goldDelta < 0 && m_goldDeltaTime > 0.0f) {
        const float progress = 1.0f - m_goldDeltaTime / m_goldDeltaDuration;
        UiText::draw(u8"+1", static_cast<int>(detail.x + u(80.0f)),
                     static_cast<int>(ownedY - u(5.0f + progress * 10.0f)), 11,
                     Color{ COL_OK.r, COL_OK.g, COL_OK.b,
                            static_cast<unsigned char>(255.0f * (1.0f - progress)) });
    }

    std::string reason;
    const bool disabled = selectedShopItemUnavailable(&reason);
    UiWidgets::Button(buyButtonRect(),
        disabled ? reason : std::string(u8"Mua · ") + std::to_string(item.m_price) + u8" vàng",
        !disabled);
    if (disabled) UiWidgets::Tooltip(buyButtonRect(), reason);
}

void ShopScreen::drawBagDetail() const {
    const Rectangle detail = detailRect();
    drawPanel(detail);
    drawMemberSelector(false, Rectangle{ detail.x + u(18.0f), detail.y + u(14.0f),
                                         detail.width - u(36.0f), u(42.0f) });
    const bool hasItem = m_bagSel >= 0 && static_cast<std::size_t>(m_bagSel) < m_info.m_bag.size();
    const ShopBagInfo* item = hasItem ? &m_info.m_bag[static_cast<std::size_t>(m_bagSel)] : nullptr;
    if (!item) {
        UiText::draw(u8"Kho đồ trống", static_cast<int>(detail.x + u(18.0f)),
                     static_cast<int>(detail.y + u(84.0f)), 16, COL_DIM);
        UiText::draw(u8"Mua vật phẩm ở cửa hàng để bắt đầu trang bị.",
                     static_cast<int>(detail.x + u(18.0f)), static_cast<int>(detail.y + u(114.0f)),
                     11, COL_LABEL);
    } else {
        if (m_icons) m_icons->draw(item->m_id,
            Rectangle{ detail.x + u(18.0f), detail.y + u(72.0f), u(58.0f), u(58.0f) }, WHITE);
        else UiIcons::draw("item:" + item->m_id,
            Rectangle{ detail.x + u(18.0f), detail.y + u(72.0f), u(58.0f), u(58.0f) });
        UiText::draw(item->m_name, static_cast<int>(detail.x + u(88.0f)),
                     static_cast<int>(detail.y + u(77.0f)), 15, COL_TITLE);
        drawPill(Rectangle{ detail.x + u(88.0f), detail.y + u(105.0f), u(126.0f), u(23.0f) },
                 categoryText(item->m_category), Color{ 36, 47, 66, 255 }, COL_LABEL);
        drawWrapped(item->m_description, detail.x + u(18.0f), detail.y + u(142.0f),
                    detail.width - u(36.0f), 11, 15, COL_TEXT, 2);
        UiText::draw(effectValue(item->m_effect, item->m_value, item->m_value2, item->m_duration),
                     static_cast<int>(detail.x + u(18.0f)), static_cast<int>(detail.y + u(190.0f)),
                     11, COL_LABEL);
    }

    const ShopMemberInfo* member = m_selectedMember < static_cast<int>(m_info.m_members.size())
        ? &m_info.m_members[static_cast<std::size_t>(m_selectedMember)] : nullptr;
    const Rectangle slot{ detail.x + u(18.0f), detail.y + u(214.0f),
                          detail.width - u(36.0f), u(78.0f) };
    UiShapes::plate(slot, UiTheme::panelRaised, BLANK);
    UiText::draw(u8"Trang bị đang mang", static_cast<int>(slot.x + u(12.0f)),
                 static_cast<int>(slot.y + u(8.0f)), 10, COL_LABEL);
    if (member && !member->m_gearId.empty()) {
        if (m_icons) m_icons->draw(member->m_gearId,
            Rectangle{ slot.x + u(10.0f), slot.y + u(31.0f), u(36.0f), u(36.0f) }, WHITE);
        UiText::draw(member->m_gearName, static_cast<int>(slot.x + u(54.0f)),
                     static_cast<int>(slot.y + u(40.0f)), 11, COL_TEXT);
    } else {
        UiText::draw(u8"Chưa trang bị", static_cast<int>(slot.x + u(12.0f)),
                     static_cast<int>(slot.y + u(43.0f)), 11, COL_DIM);
    }
    const bool canEquip = item && item->m_category == 2;
    if (item && canEquip && member) {
        const auto comparison = compareEffects(!member->m_gearId.empty(),
            member->m_gearEffect, member->m_gearValue, member->m_gearValue2,
            item->m_effect, item->m_value, item->m_value2,
            member->m_gearDuration, item->m_duration);
        const Rectangle compare{ detail.x + u(18.0f), detail.y + u(300.0f),
                                 detail.width - u(36.0f), u(42.0f) };
        UiShapes::plate(compare, UiTheme::panelRaised, BLANK);
        drawWrapped(comparison.first, compare.x + u(10.0f), compare.y + u(8.0f),
                    compare.width - u(20.0f), 10, 13, comparison.second, 2);
    } else if (item && !canEquip) {
        UiWidgets::Tooltip(Rectangle{ detail.x + u(18.0f), detail.y + u(260.0f),
                                      detail.width - u(36.0f), u(42.0f) },
                           u8"Vật phẩm này không thể trang bị.");
    }
    const Rectangle action = equipButtonRect();
    if (canEquip) {
        UiWidgets::Button(action, u8"Trang bị cho tướng đã chọn");
    } else {
        UiWidgets::Button(action, item ? u8"Không thể trang bị vật phẩm này" : u8"Chọn vật phẩm để trang bị", false);
        if (item) UiWidgets::Tooltip(action, u8"Chỉ trang bị mới đeo được cho tướng.");
    }
    if (member && !member->m_gearId.empty()) {
        const Rectangle unequip = unequipButtonRect();
        UiWidgets::Button(unequip, u8"Gỡ");
        UiWidgets::Tooltip(unequip, u8"Gỡ trang bị và chuyển lại vào kho đồ.");
    }
}
