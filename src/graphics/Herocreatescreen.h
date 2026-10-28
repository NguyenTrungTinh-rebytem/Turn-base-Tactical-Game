#pragma once
// Man hinh CHON ANH HUNG: nguoi choi chon 1 trong cac tuong co san (data/heroes.csv).
// Chi lo HIEN THI + doc input; du lieu do GameEngine dua vao qua setOptions() + setPresets().
// Khong include GameData/GameEngine => de test doc lap.
#include "raylib.h"
#include "SpriteAnimation.h"
#include "data/Stats.h"
#include "data/Element.h"
#include <string>
#include <vector>

// 1 lua chon Chung toc hoac Lop (dung de tinh chi so + liet ke ky nang)
struct HeroOption {
    std::string              m_name;
    Stats                    m_stats;        // Race Stats hoac Class Stats
    std::vector<std::string> m_skillNames;   // ten ky nang cua lua chon nay (hien thi)
};

// 1 tuong co san
struct HeroPreset {
    std::string m_id;                 // dung dat ten anh: assets/images/hero_<id>_idle.png
    std::string m_name;               // ten hien thi
    size_t      m_raceIndex  = 0;     // chi so trong danh sach Race (setOptions)
    size_t      m_classIndex = 0;     // chi so trong danh sach Class (setOptions)
    Element     m_element    = Element::KIM;
    std::string m_idleSpritePath;     // dai frame IDLE (o vuong, vd 4 x 128px); thieu file -> placeholder
};

// Ket qua nguoi choi chon (giu nguyen cau truc cu de GameEngine khong phai doi)
struct HeroSelection {
    std::string m_name;
    size_t      m_raceIndex  = 0;
    size_t      m_classIndex = 0;
    Element     m_element    = Element::KIM;
};

enum class HeroCreateAction { NONE = 0, CONFIRM, BACK };

class HeroCreateScreen {
public:
    HeroCreateScreen();
    ~HeroCreateScreen();

    // Giu texture GPU => cam copy
    HeroCreateScreen(const HeroCreateScreen&) = delete;
    HeroCreateScreen& operator=(const HeroCreateScreen&) = delete;

    // Tai nguyen (thieu file -> nen thu tuc / sprite placeholder, KHONG crash)
    bool loadBackground(const std::string& path);
    bool loadPreviewSprite(const std::string& path, int frameWidth, int frameHeight);   // placeholder cho tuong chua co anh
    void unloadAll();   // Core goi trong shutdown(), TRUOC CloseWindow(). Goi nhieu lan van an toan.

    // Du lieu: Race + Class (tinh chi so, ky nang) va danh sach tuong co san. Goi lai khi du lieu doi.
    void setOptions(const std::vector<HeroOption>& races, const std::vector<HeroOption>& classes);
    void setPresets(const std::vector<HeroPreset>& presets);   // can cua so da mo (load texture)
    bool hasOptions() const { return !m_races.empty() && !m_classes.empty() && !m_presets.empty(); }

    // Dat lai ve tuong dau tien (goi moi khi mo man hinh). Tham so giu cho tuong thich API cu.
    void reset(const std::string& defaultName = "Hero");

    void update(float deltaTime);
    HeroCreateAction handleInput();   // chuot + ban phim
    void draw() const;

    HeroSelection getSelection() const;
    void setSlotLabel(const std::string& label) { m_slotLabel = label; }

private:
    enum Btn { BTN_BACK = 0, BTN_CONFIRM, BTN_COUNT };

    Rectangle cardRect(size_t index) const;
    Rectangle buttonRect(int index) const;
    Stats     totalStats(const HeroPreset& preset) const;
    void      moveSelection(int delta);          // trai/phai
    void      moveSelectionRow(int rowDelta);    // len/xuong
    size_t    columnCount() const;

    void drawBackground() const;
    void drawCards() const;
    void drawDetail() const;
    void drawButtons() const;
    void unloadPresetTextures();

    std::string m_slotLabel = "CHON ANH HUNG";
    std::vector<HeroOption> m_races;
    std::vector<HeroOption> m_classes;
    Stats        m_statMax{};                // gia tri lon nhat moi chi so (chuan hoa thanh bar)

    std::vector<HeroPreset> m_presets;
    std::vector<Texture2D>  m_presetTextures;   // song song voi m_presets (id == 0 -> khong co anh)
    size_t       m_selected = 0;
    int          m_hoverCard = -1;
    int          m_hoverButton = -1;

    float        m_time = 0.0f;

    SpriteAnimation m_sprite;     // placeholder khi tuong chua co anh
    Texture2D    m_background{};
    bool         m_hasBackground = false;
};
