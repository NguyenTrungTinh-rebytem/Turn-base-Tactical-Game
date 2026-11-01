#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/BattleUI.h
// BattleUI CHI VE: khong chua logic game, khong include GameEngine, khong la Observer.
// Core noi su kien:  ICombatObserver::onHealthChanged -> ui.spawnFloatingText(...)
#include "raylib.h"
#include "BattleMap.h"        // BattleSlot
#include "data/Skill.h"
#include "entities/Item.h"
#include <array>
#include <string>
#include <vector>

class Character;   // Forward declaration
class ItemIcons;
class Party;

enum class BattleUIButton {
    ATTACK,
    SKILL,
    ITEM,
    END_TURN,
    UNDO,
    CANCEL_TARGET
};

class BattleUI {
public:
    BattleUI() = default;

    static Rectangle buttonBounds(BattleUIButton button);
    static Rectangle controlPanelBounds();
    static Rectangle itemMenuBounds();
    static Rectangle itemCancelBounds();
    static Rectangle partyCardBounds(int memberIndex);
    static Rectangle battleLogButtonBounds();
    static Rectangle battleLogPanelBounds();
    static Rectangle skillRowBounds(int skillIndex, int skillCount, int selectedIndex);
    static Rectangle itemRowBounds(int itemIndex);
    static int itemIndexAt(Vector2 position, int itemCount, int selectedIndex);
    static int skillScrollbarIndexAt(Vector2 position, int skillCount);
    static int itemScrollbarIndexAt(Vector2 position, int itemCount);
    static bool isVisibleHudAt(Vector2 position, bool commandVisible,
                               bool skillListVisible, bool itemListVisible,
                               bool battleLogVisible = false,
                               bool tickerVisible = false);

    void drawPartyCards(const std::array<Character*, 3>& party, int partyCount,
                        const Character* active, const Character* selectedAlly = nullptr) const;
    void drawRoundChip(int stage, int turn) const;
    void drawEnemyTurnChip(const Character* current) const;

    // ---- Marker at a unit's feet; details appear on hover ----
    void drawFieldStatus(const Character& character, Vector2 feet, bool hovered) const;

    // ---- Khung dieu khien phia duoi (thu tu ve: Panel -> Banner -> Help -> Log) ----
    void drawControlPanel() const;                                    // nen khung
    // "MAN n" + "DEN LUOT: <ten>". isPlayerTurn chi quyet dinh mau/chu, BattleUI khong biet logic game.
    void drawTurnBanner(int stage, const Character* current, bool isPlayerTurn, bool waitingConfirm) const;
    void drawCommandHelp(const std::string& helpText = "") const;     // rong = dung chu mac dinh
    void drawBattleControls(bool waitingConfirm, bool selectingTarget, bool canUndo) const;
    void observeBattleLog(const std::string& message);
    void clearBattleLog();
    void drawBattleLogButton(const std::string& ticker, bool unread) const;
    void drawBattleLogPanel(bool open);
    void scrollBattleLog(float wheelDelta);
    void dragBattleLogScrollbar(Vector2 position);
    bool battleLogUnread() const { return m_logUnread; }
    bool battleLogTickerVisible() const { return m_tickerAge < 3.0f; }
    void markBattleLogRead() { m_logUnread = false; }

    // Chọn kỹ năng trong vùng bảng điều khiển, không che khuất chiến trường.
    void drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp);
    void drawItemMenu(const std::vector<ItemPtr>& items, int selectedIndex,
                      const Character* selectedAlly, const Party& party,
                      const ItemIcons& icons) const;
    void drawTargetCursor(const BattleSlot& slot, Rectangle spriteBounds,
                          bool selected) const;

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
    std::vector<std::string> m_battleLog;
    std::string m_lastBattleLog;
    float m_tickerAge = 3.0f;
    int m_logScroll = 0;
    bool m_logUnread = false;
};