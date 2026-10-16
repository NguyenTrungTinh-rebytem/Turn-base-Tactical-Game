#pragma once

// The four top-level states of the game.
enum class State { MENU, IN_BATTLE, VICTORY, GAME_OVER };

class GameStateMachine
{
public:
    GameStateMachine();

    State getState() const;
    bool isState(State state) const;

    // True if the rules allow going from `from` to `to`.
    bool canTransition(State from, State to) const;

    // Changes state only if the transition is allowed.
    // Returns false (and changes nothing) otherwise.
    bool changeState(State next);

private:
    State m_currentState;
};