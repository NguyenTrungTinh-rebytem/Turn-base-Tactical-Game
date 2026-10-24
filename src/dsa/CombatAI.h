#pragma once

#include <vector>

class Character;

class CombatAI
{
public:
    static Character* selectTarget(
        const std::vector<Character*>& targets
    );

    static std::vector<Character*> filterAliveTargets(
        const std::vector<Character*>& targets
    );
};

