#include "BattleAnchors.h"
#include <cmath>
#include <limits>

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

BattleAnchor fromHead(Vector2 head, int frameWidth, int frameHeight,
                      float unitScaleMultiplier) {
    constexpr float canvasHeight = 720.0f;
    constexpr float feetFar = canvasHeight * 0.60f;
    constexpr float feetNear = canvasHeight * 0.72f;
    constexpr float scaleFar = 1.6f;
    constexpr float scaleNear = 2.0f;
    constexpr float headOffset = 10.0f;
    constexpr float candidateDepths[] = { 0.0f, 0.5f, 1.0f };
    float bestError = std::numeric_limits<float>::max();
    BattleAnchor best{};
    for (const float depth : candidateDepths) {
        const float feetY = feetFar + (feetNear - feetFar) * depth;
        const float scale = (scaleFar + (scaleNear - scaleFar) * depth) *
                            unitScaleMultiplier;
        const float spriteTop = feetY - frameHeight * scale;
        const float error = std::abs(spriteTop - headOffset - head.y);
        if (error < bestError) {
            bestError = error;
            const BattleSlot slot{ Vector2{ head.x, feetY }, scale, false, depth };
            best = forUnit(slot, frameWidth, frameHeight);
        }
    }
    return best;
}
}
