#include "UiLayout.h"
#include "UiTheme.h"
#include <algorithm>
#include <cmath>
namespace UiLayout {
Rectangle clamp(Rectangle r, float margin){
    r.width=std::min(r.width,UiTheme::WIDTH-2.f*margin);
    r.height=std::min(r.height,UiTheme::HEIGHT-2.f*margin);
    r.x=std::max(margin,std::min(r.x,UiTheme::WIDTH-margin-r.width));
    r.y=std::max(margin,std::min(r.y,UiTheme::HEIGHT-margin-r.height));
    return r;
}
Rectangle tooltip(Vector2 mouse,Rectangle trigger,float w,float h){
    Rectangle r{trigger.x+trigger.width+12.f,mouse.y-h*.5f,w,h};
    if(r.x+r.width>UiTheme::WIDTH-16.f)r.x=trigger.x-w-12.f;
    if(r.y<16.f)r.y=16.f;
    if(r.y+r.height>UiTheme::HEIGHT-16.f)r.y=UiTheme::HEIGHT-h-16.f;
    return clamp(r,16.f);
}
Scissor::Scissor(Rectangle r){
    const float w=static_cast<float>(GetScreenWidth());
    const float h=static_cast<float>(GetScreenHeight());
    const float scale=std::max(.01f,std::min(w/UiTheme::WIDTH,h/UiTheme::HEIGHT));
    const float ox=(w-UiTheme::WIDTH*scale)*.5f;
    const float oy=(h-UiTheme::HEIGHT*scale)*.5f;
    const int x=static_cast<int>(std::floor(ox+r.x*scale));
    const int y=static_cast<int>(std::floor(oy+r.y*scale));
    const int right=static_cast<int>(std::ceil(ox+(r.x+r.width)*scale));
    const int bottom=static_cast<int>(std::ceil(oy+(r.y+r.height)*scale));
    BeginScissorMode(x,y,std::max(1,right-x),std::max(1,bottom-y));
}
Scissor::~Scissor(){EndScissorMode();}
}
