#include "UiFrame.h"
#include "UiTheme.h"
#include <algorithm>
namespace {
Color shade(Color c,int amount){
 auto clamp=[](int n){return static_cast<unsigned char>(std::max(0,std::min(255,n)));};
 return {clamp(c.r+amount),clamp(c.g+amount),clamp(c.b+amount),255};
}
void shard(Rectangle r,Color fill,Color edge,bool emphasized){
 if(r.width<8||r.height<8)return;
 const float cut=std::min(9.f,r.height*.22f);
 // Single solid chamfered silhouette, no lines crossing content.
 DrawRectangleRec({r.x+cut,r.y,r.width-2*cut,r.height},fill);
 DrawRectangleRec({r.x,r.y+cut,r.width,r.height-2*cut},fill);
 DrawTriangle({r.x+cut,r.y},{r.x,r.y+cut},{r.x+cut,r.y+cut},fill);
 DrawTriangle({r.x+r.width-cut,r.y},{r.x+r.width,r.y+cut},{r.x+r.width-cut,r.y+cut},fill);
 DrawTriangle({r.x,r.y+r.height-cut},{r.x+cut,r.y+r.height},{r.x+cut,r.y+r.height-cut},fill);
 DrawTriangle({r.x+r.width,r.y+r.height-cut},{r.x+r.width-cut,r.y+r.height},{r.x+r.width-cut,r.y+r.height-cut},fill);
 DrawLineEx({r.x+cut,r.y+.5f},{r.x+r.width-cut,r.y+.5f},emphasized?2.f:1.f,edge);
 if(emphasized)DrawRectangleRec({r.x+1,r.y+cut,3,r.height-cut*2},edge);
}
}
namespace UiFrame {
void panel(Rectangle r,bool highlighted){
 shard(r,{21,18,31,255},highlighted?UiTheme::AGED_GOLD:Color{75,69,86,255},highlighted);
}
void plaque(Rectangle r,bool active,bool danger,float hover){
 const bool focus=active||hover>.01f;
 const Color base=danger?Color{74,37,46,255}:active?Color{66,49,39,255}:Color{32,29,43,255};
 const Color fill=shade(base,hover>0.f?static_cast<int>(10*std::min(1.f,hover)):0);
 shard(r,fill,danger?UiTheme::DANGER:focus?UiTheme::AGED_GOLD:Color{74,68,84,255},focus);
}
void gauge(Rectangle r,float value,Color tint){
 const float fraction=std::max(0.f,std::min(1.f,value));
 DrawRectangleRec(r,{13,12,19,255});
 if(fraction>.001f)DrawRectangleRec({r.x+1,r.y+1,(r.width-2)*fraction,std::max(1.f,r.height-2)},tint);
}
}
