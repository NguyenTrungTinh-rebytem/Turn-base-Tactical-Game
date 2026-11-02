#pragma once

#include "BattleMap.h"

struct BattleAnchor {
    Vector2 feet{};
    Rectangle spriteBounds{};
};

namespace BattleAnchors {
BattleAnchor forUnit(const BattleSlot& slot, int frameWidth, int frameHeight);
BattleAnchor fromHead(Vector2 head, int frameWidth, int frameHeight,
                      float unitScaleMultiplier);
}
