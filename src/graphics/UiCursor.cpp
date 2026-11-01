#include "UiCursor.h"
#include "UiInput.h"
#include "UiShapes.h"
#include "UiTheme.h"
#include <algorithm>
#include <array>
#include <cmath>

void UiCursor::initialize() { HideCursor(); }
void UiCursor::shutdown() { ShowCursor(); }

namespace {
struct Ripple {
    Vector2 position{};
    float age = 1.0f;
};
std::array<Ripple, 4> ripples{};
std::size_t nextRipple = 0;
}

void UiCursor::update(float deltaTime) {
    for (Ripple& ripple : ripples)
        ripple.age = std::min(1.0f, ripple.age + std::max(0.0f, deltaTime) / 0.34f);
    if (UiInput::pressed(MOUSE_BUTTON_LEFT)) {
        Ripple& ripple = ripples[nextRipple];
        ripple.position = UiInput::mousePosition();
        ripple.age = 0.0f;
        nextRipple = (nextRipple + 1) % ripples.size();
    }
}

void UiCursor::draw() {
    const Vector2 position = UiInput::mousePosition();
    for (const Ripple& ripple : ripples) {
        if (ripple.age >= 1.0f) continue;
        const float radius = 4.0f + ripple.age * 20.0f;
        const unsigned char alpha = static_cast<unsigned char>(180.0f * (1.0f - ripple.age));
        DrawCircleLines(static_cast<int>(ripple.position.x), static_cast<int>(ripple.position.y),
                        radius, Color{ UiTheme::ember.r, UiTheme::ember.g, UiTheme::ember.b, alpha });
    }
    if (UiInput::cursorType() == UiInput::CursorType::DISABLED) {
        DrawCircleV(position, 8.0f, Color{ 108, 101, 119, 220 });
        DrawLineEx({ position.x - 4.0f, position.y - 4.0f },
                   { position.x + 4.0f, position.y + 4.0f }, 2.0f, WHITE);
        return;
    }
    if (UiInput::cursorType() == UiInput::CursorType::TARGET) {
        DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), 10.0f,
                        UiTheme::danger);
        DrawLineEx({ position.x - 14.0f, position.y }, { position.x - 5.0f, position.y },
                   2.0f, UiTheme::danger);
        DrawLineEx({ position.x + 5.0f, position.y }, { position.x + 14.0f, position.y },
                   2.0f, UiTheme::danger);
        DrawLineEx({ position.x, position.y - 14.0f }, { position.x, position.y - 5.0f },
                   2.0f, UiTheme::danger);
        DrawLineEx({ position.x, position.y + 5.0f }, { position.x, position.y + 14.0f },
                   2.0f, UiTheme::danger);
        return;
    }
    const Vector2 points[] = {
        { position.x, position.y },
        { position.x + 5.0f, position.y + 17.0f },
        { position.x + 8.0f, position.y + 11.0f },
        { position.x + 14.0f, position.y + 14.0f },
        { position.x + 16.0f, position.y + 10.0f },
        { position.x + 10.0f, position.y + 7.0f },
        { position.x + 15.0f, position.y + 2.0f }
    };
    const Color fill = UiInput::cursorType() == UiInput::CursorType::POINTER
        ? UiTheme::accent : UiTheme::text;
    DrawTriangleFan(points, 7, fill);
    for (int i = 0; i < 7; ++i)
        DrawLineEx(points[i], points[(i + 1) % 7], 1.5f, Color{ 17, 24, 39, 255 });
}
