#pragma once
#include <vector>
class Character;
enum class StatusType
{
    ATK_BOOST,
    DEF_BOOST,
    REGEN,
    STUN,
    BURN,
    SLOW,
    WEAKNESS
};
struct StatusEffect
{
    StatusType type;
    int durationTurns;
    float value;

    StatusEffect(
        StatusType effectType,
        int duration,
        float effectValue
    )
        : type(effectType),
          durationTurns(duration),
          value(effectValue)
    {
    }
};
class StatusEffectManager
{
private:
    std::vector<StatusEffect> m_effects;

public:
    void addEffect(
        StatusType type,
        int durationTurns,
        float value
    );
    void updateEffects(Character* target);
    void clearEffects();
    bool hasEffect(StatusType type) const;
    const std::vector<StatusEffect>& getEffects() const;
};
