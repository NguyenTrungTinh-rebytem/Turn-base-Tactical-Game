#include "UiCursor.h"
#include "UiInput.h"
#include "UiShapes.h"

void UiCursor::initialize() { HideCursor(); }
void UiCursor::shutdown() { ShowCursor(); }

void UiCursor::draw() {
    const Vector2 position = UiInput::mousePosition();
    const Vector2 points[] = {
        { position.x, position.y },
        { position.x + 5.0f, position.y + 17.0f },
        { position.x + 8.0f, position.y + 11.0f },
        { position.x + 14.0f, position.y + 14.0f },
        { position.x + 16.0f, position.y + 10.0f },
        { position.x + 10.0f, position.y + 7.0f },
        { position.x + 15.0f, position.y + 2.0f }
    };
    DrawTriangleFan(points, 7, Color{ 255, 255, 255, 255 });
    for (int i = 0; i < 7; ++i)
        DrawLineEx(points[i], points[(i + 1) % 7], 1.5f, Color{ 17, 24, 39, 255 });
}
