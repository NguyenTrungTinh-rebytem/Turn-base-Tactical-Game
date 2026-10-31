#include "UiFrame.h"
#include "UiTheme.h"
#include <algorithm>
namespace {
void notched(Rectangle r, Color color, float notch) {
    const Vector2 pts[8] = {
        {r.x+notch,r.y},{r.x+r.width-notch,r.y},{r.x+r.width,r.y+notch},
        {r.x+r.width,r.y+r.height-notch},{r.x+r.width-notch,r.y+r.height},
        {r.x+notch,r.y+r.height},{r.x,r.y+r.height-notch},{r.x,r.y+notch}
    };
    for (int i = 0; i < 8; ++i) DrawTriangle({r.x+r.width/2, r.y+r.height/2}, pts[i], pts[(i+1)%8], color);
}
}
namespace UiFrame {
void panel(Rectangle r, bool highlighted) {
    DrawRectangleRec({r.x+3,r.y+5,r.width,r.height}, Color{0,0,0,115});
    notched(r, UiTheme::IRON, 13);
    DrawLineEx({r.x+18,r.y+4}, {r.x+r.width-18,r.y+4}, 1.5f, highlighted?UiTheme::EMBER:UiTheme::SLATE);
    DrawLineEx({r.x+18,r.y+r.height-5}, {r.x+r.width-18,r.y+r.height-5}, 1.5f, UiTheme::SLATE);
    for (Vector2 p : {Vector2{r.x+13,r.y+13},Vector2{r.x+r.width-13,r.y+13},
                      Vector2{r.x+13,r.y+r.height-13},Vector2{r.x+r.width-13,r.y+r.height-13}}) {
        DrawCircleV(p, 3.0f, UiTheme::AGED_GOLD);
        DrawCircleV(p, 1.1f, UiTheme::ASH);
    }
}
void plaque(Rectangle r, bool active, bool danger, float hover) {
    notched({r.x+2,r.y+4,r.width,r.height}, {0,0,0,110}, 11);
    Color base = active ? Color{90,48,39,248}: UiTheme::IRON;
    if (hover > 0.01f) {
        base.r = static_cast<unsigned char>(std::min(255.0f, base.r + 24.0f * hover));
        base.g = static_cast<unsigned char>(std::min(255.0f, base.g + 11.0f * hover));
    }
    notched(r, base, 10);
    Color edge = danger ? UiTheme::DANGER : active ? UiTheme::EMBER : UiTheme::SLATE;
    DrawLineEx({r.x+12,r.y+3},{r.x+r.width-12,r.y+3},active ? 3.0f:1.6f,edge);
    DrawLineEx({r.x+12,r.y+r.height-3},{r.x+r.width-12,r.y+r.height-3},1.6f,edge);
    if (active || hover > 0.3f) {
        DrawCircleV({r.x+9,r.y+r.height/2},2.5f,edge);
        DrawCircleV({r.x+r.width-9,r.y+r.height/2},2.5f,edge);
    }
}
void gauge(Rectangle bounds, float ratio, Color tint) {
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    DrawRectangleRec(bounds, {7,8,12,240});
    DrawRectangleRec({bounds.x+2,bounds.y+2,(bounds.width-4)*ratio,bounds.height-4},tint);
    DrawLineEx({bounds.x,bounds.y},{bounds.x+bounds.width,bounds.y},1,UiTheme::AGED_GOLD);
    DrawLineEx({bounds.x,bounds.y+bounds.height},{bounds.x+bounds.width,bounds.y+bounds.height},1,UiTheme::SLATE);
}
}
