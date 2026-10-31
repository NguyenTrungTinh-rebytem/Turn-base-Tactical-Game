#include "UiWidgets.h"
#include "UiFrame.h"
#include "UiText.h"
#include "UiTheme.h"
#include "UiIcons.h"
#include "UiLayout.h"
#include <algorithm>
#include <cctype>
namespace {
Color fg(bool enabled,bool highlighted){return !enabled?UiTheme::DISABLED:highlighted?UiTheme::AGED_GOLD:UiTheme::TEXT;}
bool has(const std::string& id,const char* s){return id.find(s)!=std::string::npos;}
}
namespace UiWidgets {
void button(Rectangle r,const std::string& text,bool hovered,bool selected,bool enabled,bool danger){
    const bool focus=enabled&&(hovered||selected);
    UiFrame::plaque(r,focus,danger,hovered?1.f:0.f);
    const int size=r.height>=44?22:18;
    UiLayout::Scissor clip({r.x+9,r.y+3,r.width-18,r.height-6});
    const int y=static_cast<int>(r.y+(r.height-size)/2.f-2);
    UiText::drawEllipsis(text,{r.x+14,static_cast<float>(y),r.width-28,static_cast<float>(size+5)},size,fg(enabled,focus));
}
void commandButton(Rectangle r,const std::string& text,int kind,bool hovered,bool enabled){
    UiFrame::plaque(r,false,false,hovered&&enabled?1.f:0.f);
    if(hovered&&enabled)DrawRectangleRec({r.x+3,r.y+6,3,r.height-12},UiTheme::AGED_GOLD);
    UiIcons::action(static_cast<UiIcons::Action>(kind),{r.x+21,r.y+r.height/2},11.f,
                    fg(enabled,hovered));
    UiLayout::Scissor clip({r.x+40,r.y+2,r.width-47,r.height-4});
    UiText::drawEllipsis(text,{r.x+42,r.y+(r.height-22)/2-1,r.width-49,27},20,fg(enabled,hovered));
}
void listRow(Rectangle r,const std::string& title,const std::string& cost,
             const std::string& iconId,bool hover,bool selected,bool enabled,bool skill){
    UiFrame::plaque(r,selected,false,hover&&enabled?1.f:0.f);
    if(hover||selected)DrawRectangleRec({r.x+2,r.y+5,3,r.height-10},UiTheme::AGED_GOLD);
    UiLayout::Scissor clip({r.x+6,r.y+1,r.width-12,r.height-2});
    const Rectangle iconBox={r.x+8,r.y+3,29,29};
    if(skill)skillIcon(iconBox,iconId);else itemIcon(iconBox,iconId);
    const int fontSize=18;
    const float pillW=std::max(43.f,static_cast<float>(UiText::measure(cost.c_str(),16)+18));
    const Rectangle pill={r.x+r.width-pillW-8,r.y+5,pillW,26};
    DrawRectangleRec(pill,{45,43,56,255});
    UiText::draw(cost.c_str(),static_cast<int>(pill.x+(pill.width-UiText::measure(cost.c_str(),16))/2),
                 static_cast<int>(pill.y+3),16,enabled?UiTheme::VERDIGRIS:UiTheme::DISABLED);
    UiText::drawEllipsis(title,{r.x+42,r.y+8,
                std::max(0.f,pill.x-r.x-48),24},fontSize,fg(enabled,hover||selected));
}
void tooltip(const std::string& title,const std::string& description,Vector2 mouse){
    tooltip(title,description,mouse,{mouse.x-4,mouse.y-4,8,8});
}
void tooltip(const std::string& title,const std::string& description,Vector2 mouse,Rectangle trigger){
    const Rectangle r=UiLayout::tooltip(mouse,trigger,245,98);
    UiFrame::panel(r);
    UiLayout::Scissor clip({r.x+9,r.y+7,r.width-18,r.height-14});
    UiText::drawEllipsis(title,{r.x+12,r.y+10,r.width-24,29},20,UiTheme::AGED_GOLD);
    UiText::drawWrapped(description,{r.x+12,r.y+44,r.width-24,52},16,UiTheme::TEXT);
}
void itemIcon(Rectangle r,const std::string& id){
    const Vector2 c={r.x+r.width*.5f,r.y+r.height*.5f};
    const float u=std::min(r.width,r.height)/32.f;
    // IDs during battle and English names in Prep both map to the same icons.
    std::string key=id;
    std::transform(key.begin(),key.end(),key.begin(),[](unsigned char ch){return static_cast<char>(std::toupper(ch));});
    if(has(key,"PHOENIX")||has(key,"REV")){
        DrawTriangle({c.x-11*u,c.y+11*u},{c.x+8*u,c.y-13*u},{c.x+4*u,c.y+12*u},UiTheme::AGED_GOLD);
        DrawLineEx({c.x-7*u,c.y+11*u},{c.x+8*u,c.y-9*u},2*u,UiTheme::TEXT);
    }else if(has(key,"IRON")||has(key,"SHIELD")){
        DrawPoly(c,5,13*u,18,{105,141,180,255});DrawPolyLines(c,5,13*u,18,UiTheme::TEXT);
    }else if(has(key,"HASTE")){
        DrawLineEx({c.x-10*u,c.y+8*u},{c.x+8*u,c.y-9*u},4*u,UiTheme::VERDIGRIS);
        DrawTriangle({c.x-12*u,c.y-4*u},{c.x+9*u,c.y-9*u},{c.x-2*u,c.y+5*u},UiTheme::TEXT);
    }else if(has(key,"PRECISION")){
        DrawCircleLines(static_cast<int>(c.x),static_cast<int>(c.y),10*u,UiTheme::VERDIGRIS);
        DrawCircleV(c,4*u,UiTheme::TEXT);
    }else if(has(key,"POWER")||has(key,"SERUM")){
        DrawPoly(c,6,11*u,30,UiTheme::AGED_GOLD);
        DrawLineEx({c.x,c.y+7*u},{c.x,c.y-8*u},3*u,UiTheme::TEXT);
        DrawTriangle({c.x-6*u,c.y-3*u},{c.x+6*u,c.y-3*u},{c.x,c.y-11*u},UiTheme::TEXT);
    }else if(has(key,"ENERGY")||has(key,"MANA")||has(key,"MP")){
        DrawRectangleRec({c.x-11*u,c.y-6*u,22*u,12*u},{45,154,182,255});
        DrawRectangleLinesEx({c.x-11*u,c.y-6*u,22*u,12*u},2*u,UiTheme::TEXT);
        DrawLineEx({c.x,c.y-4*u},{c.x,c.y+4*u},2*u,UiTheme::TEXT);
    }else{
        const bool large=has(key,"MAJOR")||has(key,"LARGE");
        const float rad=(large?12.f:9.f)*u;
        DrawRectangleRec({c.x-5*u,c.y-13*u,10*u,6*u},UiTheme::AGED_GOLD);
        DrawPoly({c.x,c.y+2*u},6,rad,30,{191,68,77,255});
        DrawPolyLines({c.x,c.y+2*u},6,rad,30,UiTheme::TEXT);
    }
}
void skillIcon(Rectangle r,const std::string& id){
    const Vector2 c={r.x+r.width*.5f,r.y+r.height*.5f};
    // Five shapes are visually distinct by category and element in source ids.
    if(has(id,"HEAL")||has(id,"RESTORE")){
        DrawCircleV(c,12,UiTheme::HP);
        DrawLineEx({c.x-8,c.y},{c.x+8,c.y},3,UiTheme::TEXT);
        DrawLineEx({c.x,c.y-8},{c.x,c.y+8},3,UiTheme::TEXT);
    }else if(has(id,"BUFF")||has(id,"AURA")){
        DrawPolyLines(c,6,12,30,UiTheme::AGED_GOLD);
        DrawTriangle({c.x-5,c.y+5},{c.x+5,c.y+5},{c.x,c.y-9},UiTheme::AGED_GOLD);
    }else if(has(id,"DEBUFF")||has(id,"CURSE")){
        DrawPoly(c,4,10,45,UiTheme::DANGER);
        DrawLineEx({c.x-6,c.y-7},{c.x+6,c.y+7},2,UiTheme::TEXT);
    }else if(has(id,"MAGIC")||has(id,"SPELL")){
        UiIcons::action(UiIcons::Action::RUNE,c,12,UiTheme::VERDIGRIS);
    }else UiIcons::action(UiIcons::Action::SWORD,c,12,UiTheme::AGED_GOLD);
}
}
