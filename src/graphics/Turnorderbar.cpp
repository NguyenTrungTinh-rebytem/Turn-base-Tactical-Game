#include "Turnorderbar.h"
#include "Combatantview.h"
#include "entities/Character.h"
#include "graphics/ui/UiFrame.h"
#include "graphics/ui/UiTheme.h"
#include "graphics/ui/UiInput.h"
#include "graphics/ui/UiIcons.h"
#include "graphics/ui/UiBattleLayout.h"
#include <algorithm>
#include <cmath>
namespace {
constexpr int MAX_SHOWN=7;
float smooth(float from,float to,float dt){return from+(to-from)*(1.f-std::exp(-14.f*dt));}
}
void TurnOrderBar::reset(){m_rows.clear();m_lastCurrent=nullptr;m_hovered=nullptr;m_flash=0;}
void TurnOrderBar::draw(const std::vector<Character*>& upcoming,const Character* current,
                        const std::vector<std::shared_ptr<Character>>& all,
                        const std::vector<std::unique_ptr<CombatantView>>& views,size_t partyCount){
 const float dt=std::min(.05f,GetFrameTime());m_hovered=nullptr;
 for(Row& r:m_rows)r.m_present=false;
 int rank=0;
 for(const Character* c:upcoming){
   if(!c||rank>=MAX_SHOWN)break;
   auto it=std::find_if(m_rows.begin(),m_rows.end(),[c](const Row& r){return r.m_character==c;});
   const float dest=54.f+rank*52.f;
   if(it==m_rows.end()){
      Row fresh{};fresh.m_character=c;fresh.m_y=dest;fresh.m_alpha=0.f;
      m_rows.push_back(fresh);it=m_rows.end()-1;
   }
   it->m_present=true;it->m_targetY=dest;it->m_isCurrent=c==current;
   it->m_rank=rank;it->m_element=c->getElement();it->m_name=c->getName();
   it->m_hp=c->getCurrentHp();it->m_maxHp=c->getStats().m_hp;it->m_spd=c->getSpeed();
   ++rank;
 }
 for(Row& r:m_rows){
    if(r.m_present){r.m_y=smooth(r.m_y,r.m_targetY,dt);r.m_alpha=std::min(1.f,r.m_alpha+dt*10.f);}
    else r.m_alpha-=dt*10.f;
 }
 m_rows.erase(std::remove_if(m_rows.begin(),m_rows.end(),[](const Row& r){return r.m_alpha<=0.f;}),m_rows.end());
 UiFrame::panel(UiBattleLayout::TIMELINE);
 for(const Row& row:m_rows){
    const float cx=row.m_y,cy=48.f;
    const float size=row.m_isCurrent?23.f:18.f;
    const Vector2 c{cx,cy};
    const auto it=std::find_if(all.begin(),all.end(),[&row](const std::shared_ptr<Character>& h){return h.get()==row.m_character;});
    const size_t idx=static_cast<size_t>(std::distance(all.begin(),it));
    const Color edge=row.m_isCurrent?UiTheme::AGED_GOLD:idx<partyCount?UiTheme::VERDIGRIS:UiTheme::DANGER;
    DrawPoly(c,6,size,30,{32,28,42,255});
    if(it!=all.end()&&idx<views.size()&&views[idx]){
        const float d=row.m_isCurrent?27.f:22.f;
        views[idx]->drawPortrait({cx-d/2,cy-d/2,d,d},WHITE);
    }
    DrawPolyLines(c,6,size,30,edge);
    UiIcons::element(row.m_element,{cx+size*.67f,cy+size*.70f},5.f);
    if(UiInput::instance().hover({cx-size-3.f,cy-size-3.f,2*size+6.f,2*size+6.f}))m_hovered=row.m_character;
 }
}
