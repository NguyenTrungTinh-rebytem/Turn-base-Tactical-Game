#include "UiIcons.h"
#include "UiTheme.h"
namespace UiIcons {
void element(Element element, Vector2 center, float radius) {
    const Color color = UiTheme::elementColor(element);
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius, color);
    switch (element) {
        case Element::KIM:
            DrawPolyLines(center, 4, radius*0.74f, 45, color);
            DrawLineEx({center.x-radius*0.45f,center.y},{center.x+radius*0.45f,center.y},1.8f,color);
            break;
        case Element::MOC:
            DrawLineEx({center.x,center.y+radius*0.7f},{center.x,center.y-radius*0.65f},2,color);
            DrawLineEx({center.x,center.y},{center.x-radius*0.6f,center.y-radius*0.3f},2,color);
            DrawLineEx({center.x,center.y-radius*0.14f},{center.x+radius*0.6f,center.y-radius*0.5f},2,color);
            break;
        case Element::THUY:
            DrawCircleV({center.x,center.y+radius*0.22f},radius*0.34f,color);
            DrawLineEx({center.x-radius*0.6f,center.y-radius*0.36f},{center.x+radius*0.6f,center.y-radius*0.36f},2,color);
            break;
        case Element::HOA:
            DrawTriangle({center.x,center.y-radius*.76f},
                         {center.x-radius*.52f,center.y+radius*.57f},
                         {center.x+radius*.52f,center.y+radius*.57f},color);
            DrawTriangle({center.x,center.y+radius*.24f},
                         {center.x-radius*.22f,center.y+radius*.52f},
                         {center.x+radius*.22f,center.y+radius*.52f},UiTheme::ASH);
            break;
        case Element::THO:
            DrawPolyLines(center, 6, radius*0.7f, 30,color);
            DrawLineEx({center.x-radius*.38f,center.y+radius*.1f},{center.x+radius*.38f,center.y+radius*.1f},2,color);
            break;
    }
}
}
