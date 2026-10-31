#pragma once
#include "raylib.h"
#include "data/Element.h"
namespace UiTheme {
// Existing gameplay/input geometry remains logical 1280x720; native rendering
// maps the 1920x1080 art grid at 1.5x without a blurry intermediate texture.
constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int GRID = 8;
constexpr Color ASH{14,13,22,255};
constexpr Color IRON{28,25,40,255};
constexpr Color SLATE{80,89,99,255};
constexpr Color VIOLET_SHADOW{31,35,44,255};
constexpr Color EMBER{183,132,89,255};
constexpr Color AGED_GOLD{223,193,148,255};
constexpr Color TEXT{237,235,229,255};
constexpr Color MUTED{189,194,199,255};
constexpr Color DISABLED{133,138,146,255};
constexpr Color VERDIGRIS{117,188,179,255};
constexpr Color DANGER{211,114,108,255};
constexpr Color HP{113,190,135,255};
constexpr Color MP{107,174,197,255};
constexpr float HOVER_SECONDS = 0.12f;
Color elementColor(Element element);
}
