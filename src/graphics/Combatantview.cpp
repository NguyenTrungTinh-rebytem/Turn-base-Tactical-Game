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

BattleAnchor CombatantView::getAnchor() const {
    return BattleAnchors::forUnit(m_slot, m_frameWidth, m_frameHeight);
}

Vector2 CombatantView::getHeadPos() const {
    return BattleMap::getHeadPos(m_slot, m_frameHeight);
}

Rectangle CombatantView::getDrawBounds() const { return getAnchor().spriteBounds; }

bool CombatantView::containsPoint(Vector2 point) const {
    return isVisible() && CheckCollisionPointRec(point, getDrawBounds());
}

void CombatantView::draw(const BattleMap& map, Color baseTint) const {
    if (m_alpha <= 0.01f) return;   // da chet han => khong ve

    Color tint = baseTint;
    if (m_flashTimer > 0.0f && std::fmod(m_flashTimer * 12.0f, 2.0f) >= 1.0f)
        tint = Color{ 255, 80, 80, 255 };
    tint.a = static_cast<unsigned char>(tint.a * m_alpha);

    map.drawShadow(m_slot, static_cast<float>(m_frameWidth), m_alpha);
    const BattleAnchor anchor = getAnchor();
    const Rectangle bounds = anchor.spriteBounds;
    m_sprite->drawFrame(Vector2{ bounds.x, bounds.y }, m_slot.m_flipX, m_slot.m_scale, tint);

#if UI_DEV_TOOLS
    DrawRectangleLinesEx(bounds, 1.0f, Color{ 255, 59, 59, 220 });
    DrawLineEx(Vector2{ anchor.feet.x - 8.0f, anchor.feet.y },
               Vector2{ anchor.feet.x + 8.0f, anchor.feet.y }, 1.0f, YELLOW);
    DrawLineEx(Vector2{ anchor.feet.x, anchor.feet.y - 8.0f },
               Vector2{ anchor.feet.x, anchor.feet.y + 8.0f }, 1.0f, YELLOW);
#endif
}

bool CombatantView::drawPortrait(Rectangle bounds, Color tint) const {
    if (!m_sprite || !m_sprite->isLoaded()) return false;
    const Texture2D& texture = m_sprite->getTexture();
    const Rectangle source{ 0.0f, 0.0f, static_cast<float>(m_frameWidth),
                            static_cast<float>(m_frameHeight) };
    DrawTexturePro(texture, source, bounds, Vector2{ 0.0f, 0.0f }, 0.0f, tint);
    return true;
}

void CombatantView::unloadAll() { m_sprite->unload(); }