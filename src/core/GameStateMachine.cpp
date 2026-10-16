#include "GameStateMachine.h"

GameStateMachine::GameStateMachine()
    : m_currentState(State::MENU)
{
}

State GameStateMachine::getState() const
{
    return m_currentState;
}

bool GameStateMachine::isState(State state) const
{
    return m_currentState == state;
}

// MENU      -> IN_BATTLE
// IN_BATTLE -> VICTORY | GAME_OVER
// VICTORY   -> MENU
// GAME_OVER -> MENU | IN_BATTLE (retry)
bool GameStateMachine::canTransition(State from, State to) const
{
    switch (from)
    {
    case State::MENU:
        return to == State::IN_BATTLE;
    case State::IN_BATTLE:
        return to == State::VICTORY || to == State::GAME_OVER;
    case State::VICTORY:
        return to == State::MENU;
    case State::GAME_OVER:
        return to == State::MENU || to == State::IN_BATTLE;
    }
    return false;
}

bool GameStateMachine::changeState(State next)
{
    if (!canTransition(m_currentState, next))
    {
        return false;
    }

    m_currentState = next;
    return true;
}