#pragma once

// The states of a turn-based battle game.
//   MENU        - title screen
//   PLAYER_TURN - the player picks an action
//   ENEMY_TURN  - the monster acts
//   VICTORY     - every monster has been defeated
//   GAME_OVER   - the hero has fallen
enum class State { MENU, PLAYER_TURN, ENEMY_TURN, VICTORY, GAME_OVER };

class GameStateMachine
{
public:
    GameStateMachine();

    State getState() const;
    bool isState(State state) const;

    // True while a battle is running (either side's turn).
    bool isInBattle() const;

    // True if the rules allow going from `from` to `to`.
    bool canTransition(State from, State to) const;

    // Changes state only if the transition is allowed.
    // Returns false (and changes nothing) otherwise.
    bool changeState(State next);

private:
    State m_currentState;
};