#ifndef ATTACK_COMMAND_H
#define ATTACK_COMMAND_H

#include "ICommand.h"
#include "Character.h"
#include "CharacterSnapshot.h"
#include <memory>

class AttackCommand : public ICommand {
private:
    Character* m_attacker;
    Character* m_target;
    std::unique_ptr<CharacterSnapshot> m_attackerBefore;
    std::unique_ptr<CharacterSnapshot> m_targetBefore;
    int m_damageDealt;
    float m_damageMultiplier;   // he so buff/debuff do Core tinh (ATK_BOOST, WEAKNESS...)

public:
    AttackCommand(Character* attacker, Character* target, float damageMultiplier = 1.0f);
    void execute() override;
    void undo() override;
};

#endif