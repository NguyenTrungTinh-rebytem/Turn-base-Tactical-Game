#pragma once
// src/graphics/AttackLunge.h
// Hieu ung "luot toi danh" cho nhan vat can chien: lui nhe lay da -> lao toi sat muc tieu -> danh -> luot ve cho cu.
// Chi doi vi tri VE (BattleSlot::m_feetPos) cua CombatantView, KHONG dong vao logic chien dau / Undo.
// Hoat dong tren cac CombatantView do GameEngine so huu; co kiem tra con tro hop le moi frame.
#include "graphics/CombatantView.h"
#include <functional>
#include <memory>
#include <vector>

namespace AttackLunge {

using ViewList = std::vector<std::unique_ptr<CombatantView>>;

// Bat dau luot toi. onHit duoc goi DUNG LUC cham muc tieu (de phat animation ATTACK, tieng, VFX).
// Tra ve false neu khong lam duoc (view null / da cung 1 vi tri) -> caller tu choi animation thuong.
bool start(CombatantView* attacker, const CombatantView* target, std::function<void()> onHit);

void update(float deltaTime, const ViewList& views);

// Ve bong mo (afterimage) phia sau nhan vat dang lao. tintOf(i) = mau goc cua view thu i.
void drawGhosts(const BattleMap& map, const ViewList& views, const std::function<Color(size_t)>& tintOf);

bool isActive(const CombatantView* view);
void clear();   // goi khi xoa/tao lai danh sach view (loadStage, shutdown)

} // namespace AttackLunge
