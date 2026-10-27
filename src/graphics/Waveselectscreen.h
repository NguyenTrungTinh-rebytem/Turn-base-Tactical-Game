#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/WaveSelectScreen.h
// Man hinh CHON WAVE: moi wave la 1 the (card) co danh sach quai, trang thai khoa / da hoan thanh.
// Chi lo HIEN THI + doc input; danh sach wave do GameEngine dua vao qua setWaves().
#include "raylib.h"
#include "data/Element.h"
#include <string>
#include <vector>

struct WaveEnemy {
    std::string m_name;
    Element     m_element = Element::KIM;
};

struct WaveInfo {
    int         m_number = 1;            // so wave (= so Stage trong game)
    std::string m_title;                 // vd "WAVE 1"
    std::string m_description;           // toi da 2 dong, ngan cach bang '\n'
    std::string m_lockHint;              // dong huong dan khi bi khoa, vd "Hoan thanh WAVE 1.2\nde mo khoa" (rong = tu sinh)
    bool        m_isBoss = false;
    bool        m_locked = false;
    bool        m_cleared = false;
    std::vector<WaveEnemy> m_enemies;    // quai co the gap
};

enum class WaveSelectAction { NONE = 0, START, BACK };

class WaveSelectScreen {
public:
    WaveSelectScreen();
    ~WaveSelectScreen();

    // Giu texture GPU => cam copy
    WaveSelectScreen(const WaveSelectScreen&) = delete;
    WaveSelectScreen& operator=(const WaveSelectScreen&) = delete;

    bool loadBackground(const std::string& path);   // thieu/loi anh -> nen thu tuc, KHONG crash
    void unloadAll();                               // Core goi trong shutdown(), TRUOC CloseWindow()

    void setWaves(const std::vector<WaveInfo>& waves);
    void setHeroSummary(const std::string& text) { m_heroSummary = text; }   // vd "Khai - Human Swordman"
    bool hasWaves() const { return !m_waves.empty(); }
    void reset(size_t selectedIndex = 0);

    void update(float deltaTime);
    WaveSelectAction handleInput();   // START chi tra ve khi wave dang chon KHONG bi khoa
    void draw() const;

    int getSelectedWave() const;      // so wave dang chon (0 neu khong co du lieu)

private:
    enum Btn { BTN_BACK = 0, BTN_START, BTN_COUNT };

    Rectangle cardRect(size_t index) const;
    Rectangle buttonRect(int index) const;
    bool selectedLocked() const;
    WaveSelectAction tryStart();
    void select(int delta);

    void drawBackground() const;
    void drawCard(size_t index) const;
    void drawButtons() const;

    std::vector<WaveInfo> m_waves;
    size_t       m_selected = 0;
    int          m_hoverButton = -1;
    float        m_time = 0.0f;
    std::string  m_heroSummary;
    std::string  m_message;
    float        m_messageTimer = 0.0f;

    Texture2D    m_background{};
    bool         m_hasBackground = false;
};