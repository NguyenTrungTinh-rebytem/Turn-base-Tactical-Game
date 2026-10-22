#include "StatusEffect.h"
#include "../src/entities/Character.h"
#include <algorithm>
void StatusEffectManager::addEffect(
    StatusType type,
    int durationTurns,
    float value
)
{
    if (durationTurns <= 0)
    {
        return;
    }
    for (StatusEffect& effect : m_effects)
    {
        if (effect.type == type)
        {
            effect.durationTurns = durationTurns;
            effect.value = value;
            return;
        }
    }
    m_effects.emplace_back(type, durationTurns, value);
}
void StatusEffectManager::updateEffects(Character* target)
{
    if (target == nullptr || !target->isAlive())
    {
        return;
    }

    for (StatusEffect& effect : m_effects)
    {
        switch (effect.type)
        {
        case StatusType::ATK_BOOST:
            break;

        case StatusType::DEF_BOOST:
            break;

        case StatusType::REGEN:
            target->heal(static_cast<int>(effect.value));
            break;

        case StatusType::STUN:
            target->setStunned(true);
            break;

        case StatusType::BURN:
            target->takeDamage(static_cast<int>(effect.value));
            break;

        case StatusType::SLOW:
            target->setSpeed(
                std::max(
                    0,
                    target->getStats().spd -
                    static_cast<int>(effect.value)
                )
            );
            break;

        case StatusType::WEAKNESS:
            break;
        }
        effect.durationTurns--;
    }
    m_effects.erase(
        std::remove_if(
            m_effects.begin(),
            m_effects.end(),
            [](const StatusEffect& effect)
            {
                return effect.durationTurns <= 0;
            }
        ),
        m_effects.end()
    );
    if (!hasEffect(StatusType::STUN))
    {
        target->setStunned(false);
    }
}
void StatusEffectManager::clearEffects()
{
    m_effects.clear();
}
bool StatusEffectManager::hasEffect(StatusType type) const
{
    for (const StatusEffect& effect : m_effects)
    {
        if (effect.type == type)
        {
            return true;
        }
    }

    return false;
}
const std::vector<StatusEffect>& StatusEffectManager::getEffects() const
{
    return m_effects;
}
