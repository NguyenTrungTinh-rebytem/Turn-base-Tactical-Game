#pragma once

class GameStateMachine
{
public:
    enum class State
    {
        MENU,
        IN_BATTLE,
        VICTORY,
        GAME_OVER
    };

    GameStateMachine() = default;

    State getState() const;
    void changeState(State newState);
    bool is(State state) const;

private:
    State m_state = State::MENU;
};
