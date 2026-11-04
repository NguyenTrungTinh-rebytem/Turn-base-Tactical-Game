#pragma once

#include "raylib.h"

namespace UiTheme {
inline constexpr int CANVAS_WIDTH = 1920;
inline constexpr int CANVAS_HEIGHT = 1080;
inline constexpr int DESIGN_WIDTH = 1280;
inline constexpr int DESIGN_HEIGHT = 720;

inline constexpr Color ink{ 7, 6, 10, 255 };
inline constexpr Color glassFillTop{ 91, 85, 120, 107 };
inline constexpr Color glassFillBottom{ 52, 46, 78, 128 };
inline constexpr Color glassButton{ 91, 85, 120, 133 };
inline constexpr Color glassModal{ 52, 46, 78, 158 };
inline constexpr Color glassTooltip{ 38, 34, 57, 199 };
inline constexpr Color glassHighlight{ 255, 255, 255, 56 };
inline constexpr Color panel = glassFillTop;
inline constexpr Color panelRaised{ 75, 69, 102, 128 };
inline constexpr Color ash{ 201, 195, 214, 140 };
inline constexpr Color silver{ 251, 247, 238, 255 };
inline constexpr Color ashText{ 224, 218, 231, 255 };
inline constexpr Color crimson{ 196, 22, 31, 255 };
inline constexpr Color crimsonGlow{ 255, 59, 59, 255 };
inline constexpr Color selectedGlassTop{ 0, 140, 180, 209 };
inline constexpr Color selectedGlassBottom{ 0, 195, 225, 209 };
inline constexpr Color gold{ 227, 182, 92, 255 };
inline constexpr Color hpHigh{ 107, 224, 124, 255 };
inline constexpr Color hpLow{ 47, 168, 90, 255 };
inline constexpr Color mpHigh{ 56, 200, 224, 255 };
inline constexpr Color mpLow{ 43, 127, 209, 255 };
inline constexpr Color danger{ 239, 75, 75, 255 };
inline constexpr Color heal{ 125, 240, 165, 255 };
inline constexpr Color navyPanel{ 16, 14, 48, 204 };     // tím navy, trong suốt 80%
inline constexpr Color cyanEdge{ 70, 225, 255, 200 };    // viền cyan mảnh
inline constexpr Color cyan{ 70, 225, 255, 255 };
inline constexpr Color hpEnemy{ 255, 150, 40, 255 };     // thanh HP kẻ địch (cam)
inline constexpr float plateChamferMax = 18.0f;
inline constexpr float bladeFadeFraction = 0.30f;
inline constexpr float bladePointRatio = 0.38f;
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

inline constexpr Rectangle turnStripHud{ 1436.0f, 12.0f, 460.0f, 68.0f };
inline constexpr Rectangle roundChipHud{ 1420.0f, 20.0f, 476.0f, 76.0f };
inline constexpr Rectangle partyCardsHud{ 16.0f, 628.0f, 372.0f, 76.0f };
inline constexpr Rectangle battleHud{ 1420.0f, 810.0f, 476.0f, 246.0f };
inline constexpr Rectangle battleLogButtonHud{ 710.0f, 900.0f, 108.0f, 48.0f };
inline constexpr Rectangle battleLogHud{ 840.0f, 840.0f, 460.0f, 216.0f };
inline constexpr Rectangle battleListHud{ 1420.0f, 300.0f, 476.0f, 510.0f };
inline constexpr Rectangle battlefieldSafeRegion{ 24.0f, 112.0f, 1872.0f, 660.0f };
inline constexpr Rectangle turnOrderHud = turnStripHud;
inline constexpr float fieldBarOffsetY = -20.0f;   
}
