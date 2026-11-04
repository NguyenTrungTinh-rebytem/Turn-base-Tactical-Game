#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/BattleUI.h
// BattleUI CHI VE: khong chua logic game, khong include GameEngine, khong la Observer.
// Core noi su kien:  ICombatObserver::onHealthChanged -> ui.spawnFloatingText(...)
#include "raylib.h"
#include "BattleMap.h"        // BattleSlot
#include "BattleAnchors.h"
#include "data/Skill.h"
#include <string>
#include <vector>

class Character;   // Forward declaration

enum class BattleUIButton {
    ATTACK,
    SKILL,
    ITEM,
    END_TURN,
    UNDO,
    CANCEL_TARGET,
    PAUSE,
    LOG_BUTTON
};

class BattleUI {
public:
    BattleUI() = default;

    static Rectangle turnStripBounds();
    static Rectangle roundChipBounds();
    static Rectangle partyCardBounds(int index);
    static Rectangle fieldHpBounds(const Character& character, Vector2 feet);
    static Rectangle fieldHpBounds(const Character& character, const BattleAnchor& anchor);
    static Rectangle buttonBounds(BattleUIButton button);
    static Rectangle controlPanelBounds();
    static Rectangle itemMenuBounds();
    static Rectangle listPanelBounds(int rowCount);
    static Rectangle logButtonBounds();
    static Rectangle logPanelBounds();
    static Rectangle skillRowBounds(int skillIndex, int skillCount, int selectedIndex);
    static Rectangle itemRowBounds(int itemIndex, int itemCount, int selectedIndex);

    // ---- HUD tren dau nhan vat ----
    // anchor is the existing core-facing head point; bars use the sprite anchor reconstructed by BattleAnchors.
    void drawHpMpBar(const Character& character, Vector2 anchor, int enemyIndex = -1) const;
    void drawRoundChip(int stage, const Character* current, int gold) const;
    void drawPartyCards(const std::vector<Character*>& party, const Character* current,
                        const Character* selectedAlly) const;

    // ---- Khung dieu khien phia duoi (thu tu ve: Panel -> Banner -> Help -> Log) ----
    void drawControlPanel() const;                                    // nen khung
    // "MAN n" + "DEN LUOT: <ten>". isPlayerTurn chi quyet dinh mau/chu, BattleUI khong biet logic game.
    void drawTurnBanner(int stage, const Character* current, bool isPlayerTurn, bool waitingConfirm) const;
    void drawCommandHelp(const std::string& helpText = "") const;     // rong = dung chu mac dinh
    void drawBattleControls(bool waitingConfirm, bool selectingTarget, bool selectingList,
                            bool canUndo) const;
    void drawLogButton(bool open) const;
    void drawListPanel(const std::string& title, int rowCount, int selectedIndex) const;
    void drawBattleLog(const std::string& message) const;

    // Popup chon ky nang o giua man hinh (nam tren khung dieu khien). Skill thieu MP thi mo.
    void drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp) const;
    void drawItemMenu(const std::vector<std::string>& labels, int selectedIndex) const;
    void drawTargetCursor(Rectangle targetBounds) const;

    // ---- Chu bay (damage / heal) ----
    // value: so luong (dau am duoc bo qua). isHeal=true -> "+N" xanh, false -> "-N" do.
    void spawnFloatingText(Vector2 pos, int value, bool isHeal);
    void update(float deltaTime);
    void drawFloatingTexts() const;

    // Xoa hieu ung dang chay (khi vao man moi). Khong giu texture nao => an toan truoc/sau CloseWindow().
    void unloadAll();

private:
    struct FloatingText {
        std::string m_text;
        Vector2     m_pos{};
        Color       m_color{};
        float       m_life = 0.0f;
        float       m_maxLife = 1.0f;
    };
    std::vector<FloatingText> m_floatingTexts;
    mutable const Character* m_lastRoundCharacter = nullptr;
    mutable int m_lastRoundStage = -1;
    mutable int m_turnNumber = 0;
    mutable float m_highlightY = -1.0f;   // vị trí thanh chọn đang trượt (-1 = chưa có)
};