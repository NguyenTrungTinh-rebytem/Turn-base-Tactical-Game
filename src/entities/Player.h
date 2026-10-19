#pragma once
#include "entities/Character.h"

class Player : public Character
{
public:
    Player(const std::string& name, const Stats& stats, Element element);

    void executeSkill(Skill* skill, Character* target) override;
};
