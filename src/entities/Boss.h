#pragma once
#include "entities/Character.h"

class Boss : public Character
{
public:
    Boss(const std::string& name, const Stats& stats, Element element);

    void takeDamage(int amount, Element atkElement, bool isMagic) override;
    void executeSkill(Skill* skill, Character* target) override;

private:
    static constexpr float DAMAGE_TAKEN_RATE = 0.8f;
    static constexpr float DAMAGE_DEALT_RATE = 1.25f;
};
