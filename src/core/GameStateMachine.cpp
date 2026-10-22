#include "GameStateMachine.h"

GameStateMachine::State GameStateMachine::getState() const
{
    return m_state;
}

void GameStateMachine::changeState(State newState)
{
    m_state = newState;
}

bool GameStateMachine::is(State state) const
{
    return m_state == state;
}
