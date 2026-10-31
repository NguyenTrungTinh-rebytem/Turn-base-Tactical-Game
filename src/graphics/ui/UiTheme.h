#pragma once
#include "raylib.h"
#include "data/Element.h"

namespace UiTheme {
constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr Color ASH{17, 17, 25, 255};
constexpr Color IRON{35, 31, 41, 246};
constexpr Color SLATE{58, 49, 62, 255};
constexpr Color VIOLET_SHADOW{75, 50, 90, 255};
constexpr Color EMBER{242, 132, 72, 255};
constexpr Color AGED_GOLD{220, 182, 116, 255};
constexpr Color TEXT{247, 239, 222, 255};
constexpr Color MUTED{188, 177, 181, 255};
constexpr Color DANGER{224, 94, 92, 255};
constexpr float HOVER_SECONDS = 0.13f;
Color elementColor(Element element);
}
