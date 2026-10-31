#pragma once
#include "raylib.h"
// Revision 3: same rectangles for hit-testing and painting, logical 1280x720.
namespace UiBattleLayout {
constexpr Rectangle TIMELINE{18,18,405,60};
constexpr Rectangle ROUND_CHIP{1092,20,164,36};
constexpr Rectangle COMMAND{1014,536,242,166};
constexpr Rectangle MENU{1217,539,33,33};
constexpr Rectangle ATTACK{1020,576,230,35};
constexpr Rectangle SKILL{1020,612,230,35};
constexpr Rectangle ITEM{1020,648,230,35};
constexpr Rectangle CANCEL{1020,646,230,45};
constexpr Rectangle CONFIRM=ATTACK;
constexpr Rectangle UNDO=SKILL;
constexpr Rectangle SKILL_POPUP{1014,219,242,311};
constexpr Rectangle ITEM_POPUP=SKILL_POPUP;
constexpr Rectangle LOG_TOGGLE{18,591,108,32};
constexpr Rectangle LOG_BODY{18,394,250,191};
constexpr Rectangle heroCard(int i) {return {18.f+175.f*i,630.f,168.f,70.f};}
constexpr Rectangle skillRow(int i){return {1021.f,252.f+37.f*i,228.f,36.f};}
constexpr Rectangle itemRow(int i){return skillRow(i);}
constexpr int VISIBLE_ROWS=7;
}
