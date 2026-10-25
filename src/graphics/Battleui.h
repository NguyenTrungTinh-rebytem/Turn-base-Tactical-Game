#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/BattleUI.h
// BattleUI CHI VE: khong chua logic game, khong include GameEngine, khong la Observer.
// Core noi su kien:  ICombatObserver::onHealthChanged -> ui.spawnFloatingText(...)
#include "raylib.h"
#include "BattleMap.h"        // BattleSlot
#include "data/Skill.h"
#include <string>
#include <vector>

class Character;   // Forward declaration

class BattleUI {
public:
    BattleUI() = default;

    // ---- HUD tren dau nhan vat ----
    // anchor = vi tri tren dau nhan vat (BattleMap::getHeadPos). Chia 0 an toan.
    void drawHpMpBar(const Character& character, Vector2 anchor) const;

    // ---- Turn order (goc phai tren). upcoming = thu tu sap toi, current = dang di (co the nullptr) ----
    void drawTurnOrder(const std::vector<Character*>& upcoming, const Character* current) const;

    // ---- Khung dieu khien phia duoi (thu tu ve: Panel -> Banner -> Help -> Log) ----
    void drawControlPanel() const;                                    // nen khung
    // "MAN n" + "DEN LUOT: <ten>". isPlayerTurn chi quyet dinh mau/chu, BattleUI khong biet logic game.
    void drawTurnBanner(int stage, const Character* current, bool isPlayerTurn, bool waitingConfirm) const;
    void drawCommandHelp(const std::string& helpText = "") const;     // rong = dung chu mac dinh
    void drawBattleLog(const std::string& message) const;

    // Popup chon ky nang o giua man hinh (nam tren khung dieu khien). Skill thieu MP thi mo.
    void drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp) const;
    void drawTargetCursor(const BattleSlot& slot) const;

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
};