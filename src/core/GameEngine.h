#pragma once

#include "GameStateMachine.h"
#include "CommandStack.h"
#include "raylib.h"

class GameEngine
{
public:
    GameEngine() = default;
    ~GameEngine() = default;

    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    void run();
    void requestState(GameStateMachine::State state);
    bool queuePlayerCommand(std::unique_ptr<ICommand> command);
    bool cancelLastAction();

private:
    void initialize();
    void processEvents();
    void update(float deltaTime);
    void render();
    void shutdown();

    GameStateMachine m_stateMachine;
    CommandStack m_commandStack;
    bool m_isRunning = false;
    bool m_isInitialized = false;
};
