#pragma once

#include "raylib.h"

namespace UiTheme {
inline constexpr int CANVAS_WIDTH = 1920;
inline constexpr int CANVAS_HEIGHT = 1080;
inline constexpr int DESIGN_WIDTH = 1280;
inline constexpr int DESIGN_HEIGHT = 720;

inline constexpr Color ink{ 7, 6, 10, 255 };
inline constexpr Color panel{ 14, 12, 20, 224 };
inline constexpr Color panelRaised{ 24, 20, 33, 236 };
inline constexpr Color ash{ 141, 135, 148, 255 };
inline constexpr Color silver{ 230, 226, 218, 255 };
inline constexpr Color ashText{ 169, 163, 174, 255 };
inline constexpr Color crimson{ 196, 22, 31, 255 };
inline constexpr Color crimsonGlow{ 255, 59, 59, 255 };
inline constexpr Color gold{ 227, 182, 92, 255 };
inline constexpr Color hpHigh{ 107, 224, 124, 255 };
inline constexpr Color hpLow{ 47, 168, 90, 255 };
inline constexpr Color mpHigh{ 56, 200, 224, 255 };
inline constexpr Color mpLow{ 43, 127, 209, 255 };
inline constexpr Color danger{ 239, 75, 75, 255 };
inline constexpr Color heal{ 125, 240, 165, 255 };
inline constexpr float plateChamferMax = 16.0f;
inline constexpr float bladeFadeFraction = 0.35f;
inline constexpr float bladePointRatio = 0.32f;
inline constexpr float runeBarSkew = 0.12f;
inline constexpr float displayTracking = 0.10f;
inline constexpr float hudTracking = 0.02f;

inline constexpr Color accent = crimson;
inline constexpr Color text = silver;
inline constexpr float radiusSmall = 6.0f;
inline constexpr float radiusMedium = 12.0f;
inline constexpr float alphaDisabled = 0.4f;
inline constexpr float alphaOverlay = 0.86f;
inline constexpr float spacingXs = 4.0f;
inline constexpr float spacingSm = 8.0f;
inline constexpr float spacingMd = 16.0f;
inline constexpr float spacingLg = 24.0f;
inline constexpr float fadeDuration = 0.8f;
inline constexpr float hoverDuration = 0.1f;
inline constexpr float floatingTextDuration = 1.0f;

inline constexpr Rectangle turnStripHud{ 0.0f, 20.0f, 820.0f, 76.0f };
inline constexpr Rectangle roundChipHud{ 1420.0f, 20.0f, 476.0f, 76.0f };
inline constexpr Rectangle partyCardsHud{ 24.0f, 900.0f, 660.0f, 156.0f };
inline constexpr Rectangle battleHud{ 1420.0f, 810.0f, 476.0f, 246.0f };
inline constexpr Rectangle battleLogButtonHud{ 710.0f, 900.0f, 108.0f, 48.0f };
inline constexpr Rectangle battleLogHud{ 840.0f, 840.0f, 460.0f, 216.0f };
inline constexpr Rectangle battleListHud{ 1420.0f, 300.0f, 476.0f, 510.0f };
inline constexpr Rectangle battlefieldSafeRegion{ 24.0f, 112.0f, 1872.0f, 660.0f };
inline constexpr Rectangle turnOrderHud = turnStripHud;
}
