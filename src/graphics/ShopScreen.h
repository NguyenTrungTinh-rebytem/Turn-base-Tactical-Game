#pragma once
// src/graphics/ShopScreen.h
// CUA HANG + KHO DO / TRANG BI (ve chong len man hinh chuan bi truoc wave boss).
// Chi lo HIEN THI + doc input. Logic mua/trang bi do GameEngine xu ly theo ShopAction roi day du lieu moi qua setInfo().
#include "raylib.h"
#include "graphics/ItemIcons.h"
#include "entities/ItemData.h"
#include <string>
#include <vector>

struct ShopStockInfo {
    std::string m_id;
    std::string m_name;
    std::string m_description;
    int m_category = 0;      // 0 tieu hao, 1 booster, 2 trang bi
    int m_price = 0;
    int m_owned = 0;         // dang so huu (trong tui + dang deo)
    int m_limit = 0;         // gioi han mang theo, 0 = khong gioi han
    ItemEffect m_effect = ItemEffect::HEAL_HP_PCT;
    int m_value = 0;
    int m_value2 = 0;
    int m_duration = 0;
};

struct ShopBagInfo {
    std::string m_id;
    std::string m_name;
    std::string m_description;
    int m_category = 0;
    int m_count = 0;
    ItemEffect m_effect = ItemEffect::HEAL_HP_PCT;
    int m_value = 0;
    int m_value2 = 0;
    int m_duration = 0;
};

struct ShopMemberInfo {
    std::string m_name;
    std::string m_portraitPath;
    std::string m_gearId;    // rong = chua deo gi
    std::string m_gearName;
    ItemEffect m_gearEffect = ItemEffect::HELD_REGEN;
    int m_gearValue = 0;
    int m_gearValue2 = 0;
    int m_gearDuration = 0;
};

struct ShopInfo {
    int m_gold = 0;
    std::vector<ShopStockInfo>  m_stock;
    std::vector<ShopBagInfo>    m_bag;
    std::vector<ShopMemberInfo> m_members;
};

enum class ShopAction { NONE = 0, CLOSE, BUY, EQUIP, UNEQUIP };

class ShopScreen {
public:
    ShopScreen() = default;
    ~ShopScreen();

    void setIcons(const ItemIcons* icons) { m_icons = icons; }
    void setInfo(const ShopInfo& info);
    void open(int tab = 0);
    void setMessage(const std::string& text, bool ok);
    void unloadAll();

    void update(float deltaTime);
    ShopAction handleInput();
    void draw() const;

    // Du lieu di kem ShopAction vua tra ve
    int getStockIndex() const { return m_stockSel; }      // BUY
    int getBagIndex() const { return m_bagSel; }          // EQUIP
    int getMemberIndex() const { return m_actionMember; } // EQUIP / UNEQUIP

private:
    enum Tab { TAB_SHOP = 0, TAB_BAG, TAB_COUNT };
    enum Filter { FILTER_ALL = 0, FILTER_CONSUMABLE, FILTER_BOOSTER, FILTER_GEAR, FILTER_COUNT };

    static constexpr int ROW_H = 56;
    static constexpr int VISIBLE_ROWS = 7;

    Rectangle panelRect() const;
    Rectangle tabRect(int index) const;
    Rectangle filterRect(int index) const;
    Rectangle listRect() const;
    Rectangle rowRect(int visibleRow) const;
    Rectangle detailRect() const;
    Rectangle buyButtonRect() const;
    Rectangle memberRect(int index, bool shopTab) const;
    Rectangle equipButtonRect() const;
    Rectangle unequipButtonRect() const;
    Rectangle scrollbarTrackRect() const;
    Rectangle scrollbarThumbRect() const;
    Rectangle closeRect() const;

    std::vector<int> visibleIndices() const;
    int& selection();
    int  selection() const;
    int  selectionPosition(const std::vector<int>& indices) const;
    void clampSelection();
    void ensureVisible();

    void drawList() const;
    void drawShopDetail() const;
    void drawBagDetail() const;
    void drawMemberSelector(bool shopTab, const Rectangle& bounds) const;
    bool selectedShopItemUnavailable(std::string* reason = nullptr) const;

    const ItemIcons* m_icons = nullptr;
    ShopInfo m_info;
    int m_tab = TAB_SHOP;
    int m_stockSel = 0;
    int m_bagSel = 0;
    int m_filter[TAB_COUNT] = { FILTER_ALL, FILTER_ALL };
    int m_selectedMember = 0;
    int m_scroll[TAB_COUNT] = { 0, 0 };
    int m_actionMember = 0;
    float m_time = 0.0f;
    float m_displayGold = 0.0f;
    float m_goldStart = 0.0f;
    float m_goldTween = 1.0f;
    int m_goldDelta = 0;
    float m_goldDeltaTime = 0.0f;
    float m_goldDeltaDuration = 0.9f;
    bool m_hasGold = false;
    Texture2D m_memberPortraits[3]{};
    std::string m_memberPortraitPaths[3];
    bool m_draggingScrollbar = false;
    float m_scrollDragOffset = 0.0f;
    std::string m_message;
    bool  m_messageOk = true;
    float m_messageTime = 0.0f;
    mutable int m_hoverRow = -1;
};
