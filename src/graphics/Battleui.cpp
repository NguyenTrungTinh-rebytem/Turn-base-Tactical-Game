#include "graphics/ui/UiText.h"
#include "Battleui.h"
#include "graphics/ui/UiFrame.h"
#include "graphics/ui/UiTheme.h"
#include "graphics/ui/UiWidgets.h"
#include "graphics/ui/UiIcons.h"
#include "graphics/ui/UiInput.h"
#include "graphics/ui/UiBattleLayout.h"
#include "graphics/ui/UiLayout.h"
#include "graphics/ui/Strings.h"
#include "Skilltooltip.h"
#include "entities/Character.h"
#include <algorithm>
#include <cmath>
namespace {
float ratio(int value,int max){return max>0?std::max(0.f,std::min(1.f,static_cast<float>(value)/max)):0.f;}
}
void BattleUI::drawHpMpBar(const Character& c,Vector2 feet) const {
    const float w=62.f,y=std::min(565.f,feet.y+9.f),x=feet.x-w*.5f;
    UiFrame::gauge({x,y,w,6},ratio(c.getCurrentHp(),c.getStats().m_hp),UiTheme::HP);
    UiIcons::element(c.getElement(),{x-7.f,y+3.f},5.f);
}
void BattleUI::drawTurnOrder(const std::vector<Character*>& upcoming,const Character* current) const {
    // Compatibility renderer: GameEngine uses TurnOrderBar for portrait medals.
    UiFrame::panel(UiBattleLayout::TIMELINE);
    int n=0;
    for(const Character* c:upcoming){
        if(!c||n>=7)break;
        const Vector2 p{54.f+52.f*n,47.f};const float size=c==current?22.f:18.f;
        DrawPoly(p,6,size,30,UiTheme::IRON);
        DrawPolyLines(p,6,size,30,c==current?UiTheme::AGED_GOLD:UiTheme::elementColor(c->getElement()));
        UiIcons::element(c->getElement(),p,8.f);++n;
    }
}
void BattleUI::drawControlPanel() const {UiFrame::panel(UiBattleLayout::COMMAND);}
void BattleUI::drawTurnBanner(int stage,const Character* current,bool isPlayerTurn,bool waitingConfirm) const {
    UiFrame::plaque(UiBattleLayout::ROUND_CHIP);
    UiText::draw(TextFormat("Màn %d",stage),static_cast<int>(UiBattleLayout::ROUND_CHIP.x+12),29,18,UiTheme::TEXT);
    if(!isPlayerTurn){
        UiFrame::plaque({1014,536,242,47});
        UiText::draw(Strings::get(Strings::Key::ENEMY_TURN),1027,547,20,UiTheme::TEXT);
    }else if(current){
        UiIcons::element(current->getElement(),{1032,552},9.f);
        UiText::drawEllipsis(current->getName(),{1047,543,156,26},20,UiTheme::TEXT);
    }
    (void)waitingConfirm;
}
void BattleUI::drawCommandHelp(const std::string& text) const {
    UiText::drawEllipsis(text.empty()?Strings::get(Strings::Key::COMMAND_HINT):text,
                         {1023,686,227,18},16,UiTheme::MUTED);
}
void BattleUI::drawBattleLog(const std::string& message) const {
    auto& in=UiInput::instance();
    UiFrame::plaque(UiBattleLayout::LOG_TOGGLE,false,false,in.hover(UiBattleLayout::LOG_TOGGLE)?1.f:0.f);
    UiText::draw(Strings::get(Strings::Key::LOG),27,597,18,UiTheme::TEXT);
    // Collapsed state shows only the small button, not a full-width bottom band.
    (void)message;
}
void BattleUI::drawSkillMenu(const std::vector<Skill>& skills,int selectedIndex,int currentMp,int scroll) const {
    const Rectangle panel=UiBattleLayout::SKILL_POPUP;
    UiFrame::panel(panel);
    UiText::draw(Strings::get(Strings::Key::SKILLS),1024,226,20,UiTheme::TEXT);
    int hovered=-1;
    {
    UiLayout::Scissor clip({panel.x+4,panel.y+31,panel.width-8,261});
    for(int j=0;j<UiBattleLayout::VISIBLE_ROWS;++j){
        const int i=j+scroll;
        if(i<0||static_cast<size_t>(i)>=skills.size())break;
        const Skill& skill=skills[static_cast<size_t>(i)];
        const Rectangle row=UiBattleLayout::skillRow(j);
        const bool enabled=skill.m_mpCost<=currentMp;
        const bool isHover=UiInput::instance().hover(row);
        const char* kind=skill.m_type==SkillType::DAMAGE?(skill.m_isMagic?"MAGIC":"PHYSICAL"):
                         skill.m_type==SkillType::HEAL?"HEAL":"BUFF";
        UiWidgets::listRow(row,skill.m_name,TextFormat("MP %d",skill.m_mpCost),kind,isHover,i==selectedIndex,enabled,true);
        UiIcons::element(skill.m_element,{row.x+34.f,row.y+28.f},5.f);
        if(isHover)hovered=i;
    }
    }
    if(hovered>=0){
       const Skill& skill=skills[static_cast<size_t>(hovered)];
       const char* caption=skill.m_mpCost>currentMp?"Không đủ MP":
             skill.m_type==SkillType::DAMAGE?"Gây sát thương lên mục tiêu":
             skill.m_type==SkillType::HEAL?"Hồi phục cho đồng đội":"Hiệu ứng hỗ trợ";
       UiText::drawEllipsis(caption,{1024,512,225,17},16,UiTheme::MUTED);
    }
    if(static_cast<int>(skills.size())>UiBattleLayout::VISIBLE_ROWS){
        const float span=UiBattleLayout::VISIBLE_ROWS*37.f;
        const float thumb=std::max(20.f,span*UiBattleLayout::VISIBLE_ROWS/static_cast<float>(skills.size()));
        const float range=span-thumb;
        const float position=range*scroll/static_cast<float>(std::max(1,static_cast<int>(skills.size())-UiBattleLayout::VISIBLE_ROWS));
        DrawRectangleRec({1250,252+position,3,thumb},UiTheme::AGED_GOLD);
    }
}
void BattleUI::drawTargetCursor(const BattleSlot& slot)const {
    const Vector2 p=slot.m_feetPos;
    const float pulse=2.5f*std::sin(static_cast<float>(GetTime())*5.f);
    DrawEllipseLines(static_cast<int>(p.x),static_cast<int>(p.y),40.f+pulse,12.f,UiTheme::EMBER);
    DrawTriangle({p.x-9,p.y-52+pulse},{p.x+9,p.y-52+pulse},{p.x,p.y-41+pulse},UiTheme::AGED_GOLD);
}
void BattleUI::spawnFloatingText(Vector2 pos,int value,bool heal){
    FloatingText ft;ft.m_text=TextFormat(heal?"+%d":"-%d",std::abs(value));ft.m_pos=pos;
    ft.m_color=heal?UiTheme::HP:UiTheme::DANGER;ft.m_life=ft.m_maxLife=1.f;
    m_floatingTexts.push_back(std::move(ft));
}
void BattleUI::update(float dt){
    for(auto& t:m_floatingTexts){t.m_life-=dt;t.m_pos.y-=45.f*dt;}
    m_floatingTexts.erase(std::remove_if(m_floatingTexts.begin(),m_floatingTexts.end(),
       [](const FloatingText& t){return t.m_life<=0.f;}),m_floatingTexts.end());
}
void BattleUI::drawFloatingTexts()const {
    for(const auto& t:m_floatingTexts){
        Color c=t.m_color;c.a=static_cast<unsigned char>(255.f*std::max(0.f,t.m_life/t.m_maxLife));
        const int x=static_cast<int>(t.m_pos.x)-UiText::measure(t.m_text.c_str(),28)/2;
        const int y=static_cast<int>(t.m_pos.y);
        for(int dx=-2;dx<=2;dx+=2)for(int dy=-2;dy<=2;dy+=2)
            UiText::draw(t.m_text.c_str(),x+dx,y+dy,28,{0,0,0,c.a});
        UiText::draw(t.m_text.c_str(),x,y,28,c);
    }
}
void BattleUI::unloadAll(){m_floatingTexts.clear();}
