// Member 4 - Huynh Quang Khai | src/graphics/CombatantView.cpp
#include "CombatantView.h"
#include "entities/Character.h"
#include <algorithm>
#include <cmath>

CombatantView::CombatantView(const Character* character, const std::string& sheetPath,
                             int frameWidth, int frameHeight, const BattleSlot& slot)
    : m_character(character),
      m_sprite(std::make_unique<SpriteAnimation>()),
      m_slot(slot),
      m_frameWidth(frameWidth),
      m_frameHeight(frameHeight) {
    m_sprite->load(sheetPath, frameWidth, frameHeight);   // that bai => placeholder, khong crash
    m_sprite->play(AnimState::IDLE);
    if (m_character && !m_character->isAlive()) m_alpha = 0.0f;
}

void CombatantView::play(AnimState state, bool restart) { m_sprite->play(state, restart); }

void CombatantView::update(float deltaTime) {
    m_sprite->update(deltaTime);
    if (m_flashTimer > 0.0f) m_flashTimer = std::max(0.0f, m_flashTimer - deltaTime);

    // Chet => mo dan; hoi sinh/undo => hien lai
    const bool alive = (m_character == nullptr) || m_character->isAlive();
    const float step = deltaTime / FADE_SECONDS;
    m_alpha = alive ? std::min(1.0f, m_alpha + step) : std::max(0.0f, m_alpha - step);
}

bool CombatantView::isOneShotFinished() const { return m_sprite->isOneShotFinished(); }

void CombatantView::triggerHitFlash() { m_flashTimer = FLASH_SECONDS; }

Vector2 CombatantView::getHeadPos() const { return BattleMap::getHeadPos(m_slot, m_frameHeight); }

void CombatantView::draw(const BattleMap& map, Color baseTint) const {
    if (m_alpha <= 0.01f) return;   // da chet han => khong ve

    Color tint = baseTint;
    if (m_flashTimer > 0.0f && std::fmod(m_flashTimer * 12.0f, 2.0f) >= 1.0f)
        tint = Color{ 255, 80, 80, 255 };
    tint.a = static_cast<unsigned char>(tint.a * m_alpha);

    map.drawShadow(m_slot, static_cast<float>(m_frameWidth), m_alpha);
    m_sprite->drawFrame(BattleMap::toTopLeft(m_slot, m_frameWidth, m_frameHeight),
                        m_slot.m_flipX, m_slot.m_scale, tint);
}

void CombatantView::unloadAll() { m_sprite->unload(); }