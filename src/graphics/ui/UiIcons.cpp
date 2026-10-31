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
namespace UiIcons {
void action(Action kind,Vector2 c,float radius,Color tint){
    const float r=radius;
    switch(kind){
        case Action::SWORD:
            DrawLineEx({c.x-r*.6f,c.y+r*.7f},{c.x+r*.65f,c.y-r*.7f},4.f,tint);
            DrawTriangle({c.x+r*.45f,c.y-r*.7f},{c.x+r*.85f,c.y-r*.95f},
                         {c.x+r*.7f,c.y-r*.3f},tint);
            DrawLineEx({c.x-r*.75f,c.y+r*.18f},{c.x-r*.1f,c.y+r*.78f},2.8f,tint);
            break;
        case Action::RUNE:
            DrawPolyLines(c,6,r*.85f,30,tint);
            DrawLineEx({c.x,c.y-r*.7f},{c.x+r*.62f,c.y+r*.44f},2.f,tint);
            DrawLineEx({c.x+r*.62f,c.y+r*.44f},{c.x-r*.62f,c.y+r*.44f},2.f,tint);
            DrawLineEx({c.x-r*.62f,c.y+r*.44f},{c.x,c.y-r*.7f},2.f,tint);
            break;
        case Action::POTION:
            DrawRectangleRec({c.x-r*.35f,c.y-r*.75f,r*.7f,r*.3f},tint);
            DrawPolyLines({c.x,c.y+r*.2f},6,r*.65f,30,tint);
            DrawCircleV({c.x,c.y+r*.25f},r*.25f,tint);
            break;
        case Action::GEAR:
            DrawPolyLines(c,8,r*.75f,22.5f,tint);
            DrawCircleLines(static_cast<int>(c.x),static_cast<int>(c.y),r*.3f,tint);
            break;
    }
}
}
