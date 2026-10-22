#pragma once
#include "entities/Character.h"

// Lớp Boss: Kế thừa từ Character, trâu hơn và gây sát thương mạnh hơn Enemy thường
class Boss : public Character
{
public:
    Boss(const std::string& name, const Stats& stats, Element element);

    // Ghi đè hàm nhận sát thương (chỉ nhận 80% sát thương gốc)
    void takeDamage(int amount, Element atkElement, bool isMagic) override;

    // Ghi đè hàm dùng skill (gây 125% sát thương gốc)
    void executeSkill(Skill* skill, Character* target) override;

private:
    static constexpr float DAMAGE_TAKEN_RATE = 0.8f;   // Hệ số sát thương nhận vào (giảm 20%)
    static constexpr float DAMAGE_DEALT_RATE = 1.25f;  // Hệ số sát thương gây ra (tăng 25%)
};
