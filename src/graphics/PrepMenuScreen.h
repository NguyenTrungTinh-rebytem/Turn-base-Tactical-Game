#pragma once
// src/graphics/PrepMenuScreen.h
// MENU TRUOC TRAN (4 tab): DOI HINH / KY NANG / TUI DO / THONG TIN CONG.
// Chi lo HIEN THI + doc input; du lieu do GameEngine dua vao qua setInfo().
#include "raylib.h"
#include "data/Element.h"
#include "data/Stats.h"
#include <string>
#include <vector>

struct PrepHeroInfo {
    std::string m_name;
    std::string m_race;
    std::string m_class;
    Element     m_element = Element::KIM;
    Stats       m_stats{};
};

struct PrepSkillInfo {
    std::string m_name;
    Element     m_element = Element::KIM;
    std::string m_typeText;      // "Sat thuong", "Hoi mau", "Buff", "Debuff"
    int         m_power = 0;     // % (vd 130)
    int         m_mpCost = 0;
    std::string m_effectText;    // "Chay 2 luot", rong = khong co
    bool        m_isMagic = false; // true = sat thuong tinh theo MA (cho dong "Uoc tinh" cua tooltip)
};

struct PrepItemInfo {
    std::string m_name;
    int         m_count = 0;
};

struct PrepEnemyInfo {
    std::string m_name;
    Element     m_element = Element::KIM;
};

struct PrepGateInfo {
    std::string m_title;                 // "WAVE 1"
    std::string m_description;           // toi da 2 dong, ngan cach '\n'
    int         m_deadlineTurns = -1;    // < 0 = khong gioi han thoi gian
    bool        m_hasBoss = false;
    std::string m_bossName;
    Element     m_bossElement = Element::KIM;
    int         m_bossHp = 0;
    std::vector<PrepEnemyInfo> m_enemies;
};

struct PrepInfo {
    PrepHeroInfo               m_hero;
    std::vector<PrepHeroInfo> m_members; // nhan vat thuc su da duoc chon vao 3 o
    std::vector<PrepSkillInfo> m_skills;
    std::vector<PrepItemInfo>  m_items;
    PrepGateInfo               m_gate;
    int                        m_teammateSlots = 2;   // so o dong doi con trong
};

enum class PrepMenuAction { NONE = 0, START, BACK, EDIT_MEMBER_1, EDIT_MEMBER_2, EDIT_MEMBER_3 };

class PrepMenuScreen {
public:
    PrepMenuScreen();
    ~PrepMenuScreen();

    // Giu texture GPU => cam copy
    PrepMenuScreen(const PrepMenuScreen&) = delete;
    PrepMenuScreen& operator=(const PrepMenuScreen&) = delete;

    bool loadBackground(const std::string& path);   // thieu/loi anh -> nen thu tuc, KHONG crash
    void unloadAll();                               // Core goi trong shutdown(), TRUOC CloseWindow()

    void setInfo(const PrepInfo& info);
    void reset(int tab = 0);

    void update(float deltaTime);
    PrepMenuAction handleInput();
    void draw() const;

private:
    enum Tab { TAB_PARTY = 0, TAB_SKILLS, TAB_ITEMS, TAB_GATE, TAB_COUNT };
    enum Btn { BTN_BACK = 0, BTN_START, BTN_COUNT };

    Rectangle tabRect(int index) const;
    Rectangle contentRect() const;
    Rectangle buttonRect(int index) const;
    void selectTab(int delta);

    void drawBackground() const;
    void drawTabs() const;
    void drawPartyTab(const Rectangle& area) const;
    void drawSkillsTab(const Rectangle& area) const;
    void drawItemsTab(const Rectangle& area) const;
    void drawGateTab(const Rectangle& area) const;
    void drawButtons() const;

    PrepInfo m_info;
    int      m_tab = TAB_PARTY;
    int      m_hoverTab = -1;
    int      m_hoverButton = -1;
    mutable int m_hoverSkill = -1;   // dong skill dang tro chuot (draw() cap nhat moi frame)
    float    m_time = 0.0f;

    Texture2D m_background{};
    bool      m_hasBackground = false;
};