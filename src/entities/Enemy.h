#pragma once
#include "entities/Character.h"

class Enemy : public Character
{
public:
    Enemy(const std::string& name, const Stats& stats, Element element);

    void executeSkill(Skill* skill, Character* target) override;
};
