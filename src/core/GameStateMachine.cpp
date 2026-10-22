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

bool GameStateMachine::isInBattle() const
{
    return m_currentState == State::PLAYER_TURN || m_currentState == State::ENEMY_TURN;
}

// MENU        -> PLAYER_TURN
// PLAYER_TURN -> ENEMY_TURN | VICTORY
// ENEMY_TURN  -> PLAYER_TURN | GAME_OVER
// VICTORY     -> MENU
// GAME_OVER   -> MENU | PLAYER_TURN (retry)
bool GameStateMachine::canTransition(State from, State to) const
{
    switch (from)
    {
    case State::MENU:
        return to == State::PLAYER_TURN;
    case State::PLAYER_TURN:
        return to == State::ENEMY_TURN || to == State::VICTORY;
    case State::ENEMY_TURN:
        return to == State::PLAYER_TURN || to == State::GAME_OVER;
    case State::VICTORY:
        return to == State::MENU;
    case State::GAME_OVER:
        return to == State::MENU || to == State::PLAYER_TURN;
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