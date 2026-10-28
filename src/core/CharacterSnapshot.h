#pragma once

#include "entities/Character.h"
#include "entities/Item.h"
#include <memory>

// Snapshot combat HP/MP/buff/gear before a Command executes; used for Undo.
// Deep-copy equipped gear so one-battle effects (Survival Sash) are reversible.
struct CharacterSnapshot
{
    int m_hp = 0;
    int m_mp = 0;
    CombatModifiers m_modifiers;

    explicit CharacterSnapshot(const Character& character)
        : m_hp(character.getCurrentHp()), m_mp(character.getCurrentMp()),
          m_modifiers(character.modifiers())
    {
        if (auto gear = m_modifiers.getGear())
            m_modifiers.equipGear(std::make_shared<HeldGear>(*gear));
    }

    void restore(Character& character) const
    {
        const int hpDiff = m_hp - character.getCurrentHp();
        if (hpDiff > 0) character.heal(hpDiff);
        else if (hpDiff < 0) character.takeDamage(-hpDiff);

        const int mpDiff = m_mp - character.getCurrentMp();
        if (mpDiff > 0) character.restoreMp(mpDiff);
        else if (mpDiff < 0) character.consumeMp(-mpDiff);
        character.modifiers() = m_modifiers;
    }
};
