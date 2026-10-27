#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/HeroCreateScreen.h
// Man hinh TAO ANH HUNG: nhap ten + chon Chung toc / Lop / He, xem truoc chi so + ky nang + sprite.
// Chi lo HIEN THI + doc input; du lieu (chung toc, lop, chi so, ten ky nang) do GameEngine dua vao qua setOptions().
// Khong include GameData/GameEngine => de test doc lap.
#include "raylib.h"
#include "SpriteAnimation.h"
#include "LayeredSprite.h"   // ve nhan vat ghep lop (than theo Chung toc + giap theo Lop)
#include "data/Stats.h"
#include "data/Element.h"
#include <string>
#include <vector>

// 1 lua chon Chung toc hoac Lop
struct HeroOption {
    std::string              m_name;
    Stats                    m_stats;        // Race Stats hoac Class Stats
    std::vector<std::string> m_skillNames;   // ten ky nang cua lua chon nay (hien thi)
};

// Ket qua nguoi choi chon
struct HeroSelection {
    std::string m_name;
    size_t      m_raceIndex  = 0;
    size_t      m_classIndex = 0;
    Element     m_element    = Element::KIM;
};

enum class HeroCreateAction { NONE = 0, CONFIRM, BACK };

class HeroCreateScreen {
public:
    static constexpr size_t MAX_NAME_LENGTH = 12;

    HeroCreateScreen();
    ~HeroCreateScreen();

    // Giu texture GPU => cam copy
    HeroCreateScreen(const HeroCreateScreen&) = delete;
    HeroCreateScreen& operator=(const HeroCreateScreen&) = delete;

    // Tai nguyen (thieu file -> nen thu tuc / sprite placeholder, KHONG crash)
    bool loadBackground(const std::string& path);
    bool loadPreviewSprite(const std::string& path, int frameWidth, int frameHeight);
    void unloadAll();   // Core goi trong shutdown(), TRUOC CloseWindow(). Goi nhieu lan van an toan.

    // Du lieu lua chon (Race + Class tu GameData). Goi lai khi du lieu doi.
    void setOptions(const std::vector<HeroOption>& races, const std::vector<HeroOption>& classes);
    bool hasOptions() const { return !m_races.empty() && !m_classes.empty(); }

    // Dat lai lua chon ve mac dinh (goi moi khi mo man hinh)
    void reset(const std::string& defaultName = "Hero");

    void update(float deltaTime);
    HeroCreateAction handleInput();   // chuot + ban phim; CONFIRM chi tra ve khi ten hop le
    void draw() const;

    HeroSelection getSelection() const;   // ten da cat khoang trang 2 dau

private:
    enum Row { ROW_NAME = 0, ROW_RACE, ROW_CLASS, ROW_ELEMENT, ROW_COUNT };
    enum Btn { BTN_BACK = 0, BTN_RANDOM, BTN_CONFIRM, BTN_COUNT };

    Rectangle rowRect(int row) const;
    Rectangle leftArrowRect(int row) const;
    Rectangle rightArrowRect(int row) const;
    Rectangle buttonRect(int index) const;
    int  optionCount(int row) const;
    void changeValue(int row, int delta);
    void randomize();
    HeroCreateAction tryConfirm();
    void handleTextInput();
    Stats totalStats() const;

    // --- Luat Chung toc x Lop + sprite ghep lop ---
    bool currentCombo(RaceType& race, ClassType& pClass) const;   // false neu ten khong khop enum
    bool isCurrentComboValid() const;
    void enforceValidCombo(int editedRow, int direction, bool notify);   // sua hang CON LAI cho hop le
    void syncLayeredSprite();

    void drawBackground() const;
    void drawSelectors() const;
    void drawPreview() const;
    void drawButtons() const;

    std::vector<HeroOption> m_races;
    std::vector<HeroOption> m_classes;
    Stats        m_statMax;                 // gia tri lon nhat moi chi so (de chuan hoa thanh bar)

    std::string  m_name = "Hero";
    size_t       m_raceIdx = 0, m_classIdx = 0, m_elementIdx = 0;
    int          m_focus = ROW_NAME;
    int          m_hoverButton = -1;

    float        m_time = 0.0f;
    float        m_backspaceHold = 0.0f, m_backspaceRepeat = 0.0f;
    std::string  m_message;
    float        m_messageTimer = 0.0f;

    SpriteAnimation m_sprite;     // placeholder khi chua co anh ghep lop
    LayeredSprite   m_layered;    // xem truoc nhan vat that (than + giap)
    Texture2D    m_background{};
    bool         m_hasBackground = false;
};
