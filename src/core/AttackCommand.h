#ifndef ATTACK_COMMAND_H
#define ATTACK_COMMAND_H

#include "ICommand.h"
#include "Character.h"

class AttackCommand : public ICommand {
private:
    Character* m_attacker;
    Character* m_target;
    int m_damageDealt;

public:
    AttackCommand(Character* attacker, Character* target);
    void execute() override;
    void undo() override;
};

#endif