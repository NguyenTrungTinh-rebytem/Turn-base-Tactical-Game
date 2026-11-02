#pragma once

#include "raylib.h"

namespace UiTheme {
inline constexpr int CANVAS_WIDTH = 1920;
inline constexpr int CANVAS_HEIGHT = 1080;
inline constexpr int DESIGN_WIDTH = 1280;
inline constexpr int DESIGN_HEIGHT = 720;

inline constexpr Color ink{ 15, 15, 30, 255 };
inline constexpr Color panel{ 15, 15, 30, 220 };
inline constexpr Color panelRaised{ 0, 40, 100, 240 };
inline constexpr Color accent{ 200, 170, 80, 255 };
inline constexpr Color text{ 245, 245, 245, 255 };

inline constexpr float radiusSmall = 6.0f;
inline constexpr float radiusMedium = 12.0f;
inline constexpr float alphaDisabled = 0.55f;
inline constexpr float alphaOverlay = 0.86f;
inline constexpr float spacingXs = 4.0f;
inline constexpr float spacingSm = 8.0f;
inline constexpr float spacingMd = 16.0f;
inline constexpr float spacingLg = 24.0f;
inline constexpr float fadeDuration = 0.8f;
inline constexpr float hoverDuration = 0.1f;
inline constexpr float floatingTextDuration = 1.0f;

// HUD rectangles are specified in physical 1920x1080 canvas coordinates.
inline constexpr Rectangle turnStripHud{ 24.0f, 20.0f, 420.0f, 76.0f };
inline constexpr Rectangle roundChipHud{ 1688.0f, 20.0f, 208.0f, 76.0f };
inline constexpr Rectangle partyCardsHud{ 24.0f, 112.0f, 660.0f, 112.0f };
inline constexpr Rectangle battleHud{ 24.0f, 840.0f, 720.0f, 216.0f };
inline constexpr Rectangle battleLogButtonHud{ 768.0f, 840.0f, 108.0f, 48.0f };
inline constexpr Rectangle battleLogHud{ 900.0f, 832.0f, 496.0f, 224.0f };
inline constexpr Rectangle battleListHud{ 1420.0f, 832.0f, 476.0f, 224.0f };
inline constexpr Rectangle battlefieldSafeRegion{ 24.0f, 236.0f, 1872.0f, 588.0f };
inline constexpr Rectangle turnOrderHud = turnStripHud;
}
