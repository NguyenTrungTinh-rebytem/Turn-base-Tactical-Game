#pragma once
#include "raylib.h"
namespace UiLayout {
Rectangle clamp(Rectangle bounds, float margin=12.f);
Rectangle tooltip(Vector2 mouse, Rectangle trigger, float width, float height);
// Raylib scissor uses screen pixel coordinates, not logical camera coordinates.
class Scissor {
public:
    explicit Scissor(Rectangle logical);
    ~Scissor();
    Scissor(const Scissor&)=delete;
    Scissor& operator=(const Scissor&)=delete;
};
}
