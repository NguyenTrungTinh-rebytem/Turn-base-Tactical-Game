#pragma once

#include "raylib.h"

namespace UiTheme {
inline constexpr int CANVAS_WIDTH = 1920;
inline constexpr int CANVAS_HEIGHT = 1080;
inline constexpr int DESIGN_WIDTH = 1280;
inline constexpr int DESIGN_HEIGHT = 720;

inline constexpr Color obsidian{ 12, 10, 18, 255 };
inline constexpr Color ink = obsidian;
inline constexpr Color panel{ 20, 17, 31, 216 };
inline constexpr Color panelBottom{ 29, 24, 48, 224 };
inline constexpr Color panelRaised{ 29, 24, 48, 224 };
inline constexpr Color buttonGlassTop{ 20, 17, 31, 255 };
inline constexpr Color buttonGlassBottom{ 29, 24, 48, 255 };
inline constexpr Color border{ 58, 49, 80, 255 };
inline constexpr Color secondaryBorder{ 74, 63, 99, 255 };
inline constexpr Color accent{ 242, 190, 92, 255 };
inline constexpr Color ember{ 232, 131, 43, 255 };
inline constexpr Color verdigris{ 63, 214, 192, 255 };
inline constexpr Color text{ 244, 236, 221, 255 };
inline constexpr Color ash{ 185, 176, 162, 255 };
inline constexpr Color disabled{ 108, 101, 119, 255 };
inline constexpr Color hpTop{ 107, 224, 124, 255 };
inline constexpr Color hpBottom{ 47, 168, 90, 255 };
inline constexpr Color mpTop{ 56, 200, 224, 255 };
inline constexpr Color mpBottom{ 43, 127, 209, 255 };
inline constexpr Color danger{ 239, 75, 75, 255 };
inline constexpr Color heal{ 125, 240, 165, 255 };
inline constexpr Color currency{ 255, 211, 107, 255 };
inline constexpr Color kim{ 233, 236, 245, 255 };
inline constexpr Color moc{ 98, 210, 111, 255 };
inline constexpr Color thuy{ 76, 157, 255, 255 };
inline constexpr Color hoa{ 255, 90, 74, 255 };
inline constexpr Color tho{ 214, 164, 92, 255 };
inline constexpr Color primaryTop{ 240, 178, 74, 255 };
inline constexpr Color primaryBottom{ 200, 106, 34, 255 };
inline constexpr Color primaryText{ 26, 16, 8, 255 };
inline constexpr Color dangerTop{ 122, 36, 52, 255 };
inline constexpr Color dangerBottom{ 86, 23, 38, 255 };

inline constexpr float panelAlpha = 0.84f;
inline constexpr float modalAlpha = 0.92f;
inline constexpr float tooltipAlpha = 0.94f;
inline constexpr float buttonAlpha = 0.90f;
inline constexpr float primaryButtonAlpha = 0.96f;
inline constexpr float chipAlpha = 0.80f;
inline constexpr float disabledAlpha = 0.60f;

// Canvas coordinates are 1280x720 and are rendered at 1.5x.
inline constexpr float radiusLarge = 9.33f;
inline constexpr float radiusCard = 8.0f;
inline constexpr float radiusSmall = 6.67f;
inline constexpr float radiusMedium = 9.33f;
inline constexpr float radiusChip = 5.33f;
inline constexpr float radiusBar = 2.67f;
inline constexpr float radiusTooltip = 6.67f;
inline constexpr float radiusHex = 2.67f;
inline constexpr float alphaDisabled = disabledAlpha;
inline constexpr float alphaOverlay = 0.86f;
inline constexpr float spacingXs = 4.0f;
inline constexpr float spacingSm = 8.0f;
inline constexpr float spacingMd = 16.0f;
inline constexpr float spacingLg = 24.0f;
inline constexpr float fadeDuration = 0.2f;
inline constexpr float hoverDuration = 0.12f;
inline constexpr float floatingTextDuration = 1.0f;
inline constexpr float floatingShadowSize = 8.0f;

inline constexpr float battlefieldMargin = 24.0f;
inline constexpr Rectangle battleTurnStrip{ 24.0f, 24.0f, 360.0f, 52.0f };
inline constexpr Rectangle battleRoundChip{ 1100.0f, 24.0f, 156.0f, 32.0f };
inline constexpr Rectangle battlePartyCards[3]{
    Rectangle{ 24.0f, 620.0f, 170.0f, 72.0f },
    Rectangle{ 210.0f, 620.0f, 170.0f, 72.0f },
    Rectangle{ 396.0f, 620.0f, 170.0f, 72.0f }
};
inline constexpr Rectangle battleCommandCluster{ 1016.0f, 571.0f, 240.0f, 125.0f };
inline constexpr Rectangle battleSkillList{ 1016.0f, 265.0f, 240.0f, 300.0f };
inline constexpr Rectangle battleItemList{ 1016.0f, 265.0f, 240.0f, 300.0f };
inline constexpr Rectangle battleCommandListMax{ 1016.0f, 265.0f, 240.0f, 300.0f };
inline constexpr Rectangle battleLogPanel{ 24.0f, 438.0f, 250.0f, 120.0f };
inline constexpr Rectangle battleLogButton{ 24.0f, 576.0f, 100.0f, 30.0f };
inline constexpr Rectangle battleLogTicker{ 134.0f, 576.0f, 240.0f, 30.0f };
inline constexpr Rectangle battleEnemyTurnChip{ 1040.0f, 638.0f, 192.0f, 38.0f };
}
