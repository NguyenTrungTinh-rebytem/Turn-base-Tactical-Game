#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/CombatantView.h
// Ghep 1 Character (chi QUAN SAT, khong so huu) <-> 1 SpriteAnimation (so huu) + 1 BattleSlot.
#include "raylib.h"
#include "BattleMap.h"
#include "SpriteAnimation.h"
#include <memory>
#include <string>

class Character;   // Forward declaration

class CombatantView {
public:
    // Neu sheetPath khong ton tai -> SpriteAnimation ve placeholder (khong crash).
    CombatantView(const Character* character, const std::string& sheetPath,
                  int frameWidth, int frameHeight, const BattleSlot& slot);
    ~CombatantView() = default;

    // Giu texture GPU => cam copy (dung qua std::unique_ptr<CombatantView>)
    CombatantView(const CombatantView&) = delete;
    CombatantView& operator=(const CombatantView&) = delete;

    // ---- API cho Core ----
    void play(AnimState state, bool restart = true);
    void update(float deltaTime);
    void draw(const BattleMap& map, Color baseTint = WHITE) const;   // bong + sprite (co lam mo khi chet)
    bool isOneShotFinished() const;

    // ---- Tien ich ----
    void triggerHitFlash();                         // nhap nhay do ~0.4s
    void setSlot(const BattleSlot& slot) { m_slot = slot; }
    const BattleSlot& getSlot() const { return m_slot; }
    Vector2 getHeadPos() const;                     // dung cho BattleUI::drawHpMpBar / spawnFloatingText
    const Character* getCharacter() const { return m_character; }
    bool isVisible() const { return m_alpha > 0.01f; }

    void unloadAll();                               // giai phong texture (Core goi truoc CloseWindow)

private:
    const Character*                 m_character = nullptr;   // non-owning (khong quan ly vong doi)
    std::unique_ptr<SpriteAnimation> m_sprite;
    BattleSlot                       m_slot;
    int                              m_frameWidth  = 64;
    int                              m_frameHeight = 64;
    float                            m_alpha = 1.0f;          // 1 song, 0 da bien mat
    float                            m_flashTimer = 0.0f;

    static constexpr float FADE_SECONDS  = 0.6f;
    static constexpr float FLASH_SECONDS = 0.4f;
};