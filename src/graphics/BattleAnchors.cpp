#include "BattleAnchors.h"

namespace BattleAnchors {
BattleAnchor forUnit(const BattleSlot& slot, int frameWidth, int frameHeight) {
    const Vector2 topLeft = BattleMap::toTopLeft(slot, frameWidth, frameHeight);
    return {
        slot.m_feetPos,
        Rectangle{ topLeft.x, topLeft.y,
                   static_cast<float>(frameWidth) * slot.m_scale,
                   static_cast<float>(frameHeight) * slot.m_scale }
    };
}
}
