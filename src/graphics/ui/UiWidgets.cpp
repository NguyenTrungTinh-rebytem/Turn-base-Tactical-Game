#include "UiWidgets.h"
#include "UiFrame.h"
#include "UiText.h"
#include "UiTheme.h"
#include <algorithm>
#include <cctype>
namespace UiWidgets {
void button(Rectangle area, const std::string& text, bool hovered, bool selected, bool enabled, bool danger) {
    UiFrame::plaque(area, selected || hovered, danger, hovered?1.f:0.f);
    const int size = area.height > 56 ? 22 : 18;
    UiText::drawEllipsis(text, {area.x + 22, area.y + (area.height-size)/2, area.width - 44,static_cast<float>(size)},
                         size, enabled ? (hovered || selected ? UiTheme::AGED_GOLD : UiTheme::TEXT) : UiTheme::MUTED);
}
void tooltip(const std::string& title, const std::string& description, Vector2 mouse) {
    const float width = 330, height = 104;
    const float x = std::max(10.0f, std::min(1280.0f-width-10,mouse.x+18));
    const float y = std::max(10.0f, std::min(720.0f-height-10,mouse.y+18));
    UiFrame::panel({x,y,width,height},true);
    UiText::drawEllipsis(title,{x+18,y+12,width-36,24},20,UiTheme::AGED_GOLD);
    UiText::drawWrapped(description,{x+18,y+43,width-36,55},16,UiTheme::TEXT);
}
void itemIcon(Rectangle bounds, const std::string& id) {
    const Vector2 c = {bounds.x + bounds.width/2,bounds.y + bounds.height/2};
    std::string lower = id;
    std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    const bool mana = lower.find("mp") != std::string::npos || lower.find("mana") != std::string::npos;
    const bool revive = lower.find("rev") != std::string::npos || lower.find("phoenix") != std::string::npos;
    const bool buff = lower.find("buff") != std::string::npos || lower.find("atk") != std::string::npos || lower.find("def") != std::string::npos;
    Color liquid = mana ? Color{85,152,239,255}:revive?Color{240,170,70,255}:buff?Color{145,111,215,255}:Color{215,77,86,255};
    if (revive) {
        DrawTriangle({c.x,c.y-26},{c.x-21,c.y+21},{c.x+10,c.y+10},liquid);
        DrawTriangle({c.x,c.y-22},{c.x+21,c.y+19},{c.x-9,c.y+10},UiTheme::AGED_GOLD);
    } else if (buff) {
        DrawPoly(c,6,23,30,liquid);
        DrawPolyLines(c,6,23,30,UiTheme::AGED_GOLD);
        DrawLineEx({c.x-12,c.y},{c.x+12,c.y},3,UiTheme::TEXT);
        DrawLineEx({c.x,c.y-12},{c.x,c.y+12},3,UiTheme::TEXT);
    } else {
        DrawRectangleRec({c.x-8,c.y-28,16,10},UiTheme::AGED_GOLD);
        DrawPoly({c.x,c.y+5},6,22,30,liquid);
        DrawLineEx({c.x-11,c.y-8},{c.x+11,c.y-8},2,UiTheme::TEXT);
        if (mana) DrawCircleV({c.x,c.y+7},6,UiTheme::TEXT);
        else DrawLineEx({c.x-6,c.y+6},{c.x+6,c.y+6},3,UiTheme::TEXT);
    }
}
}
