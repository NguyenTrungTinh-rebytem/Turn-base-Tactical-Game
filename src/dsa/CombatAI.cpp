#include "CombatAI.h"
#include "entities/Character.h"

#include <algorithm>

std::vector<Character*> CombatAI::filterAliveTargets(
    const std::vector<Character*>& targets
)
{
    std::vector<Character*> aliveTargets;

    for (Character* target : targets)
    {
        if (target != nullptr && target->isAlive())
        {
            aliveTargets.push_back(target);
        }
    }

    return aliveTargets;
}

Character* CombatAI::selectTarget(
    const std::vector<Character*>& targets
)
{
    std::vector<Character*> aliveTargets =
        filterAliveTargets(targets);

    if (aliveTargets.empty())
    {
        return nullptr;
    }

    return *std::min_element(
        aliveTargets.begin(),
        aliveTargets.end(),
        [](const Character* lhs, const Character* rhs)
        {
            const int lhsMaxHp = lhs->getStats().m_hp;
            const int rhsMaxHp = rhs->getStats().m_hp;

            const float lhsHpRatio =
                lhsMaxHp > 0
                    ? static_cast<float>(lhs->getCurrentHp()) / lhsMaxHp
                    : 0.0f;

            const float rhsHpRatio =
                rhsMaxHp > 0
                    ? static_cast<float>(rhs->getCurrentHp()) / rhsMaxHp
                    : 0.0f;

            if (lhsHpRatio != rhsHpRatio)
            {
                return lhsHpRatio < rhsHpRatio;
            }

            return lhs->getSpeed() > rhs->getSpeed();
        }
    );
}
