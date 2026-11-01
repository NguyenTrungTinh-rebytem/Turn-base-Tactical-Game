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

inline constexpr Rectangle battleHud{ 20.0f, 560.0f, 1240.0f, 140.0f };
inline constexpr Rectangle turnOrderHud{ 1100.0f, 10.0f, 170.0f, 270.0f };
}
