#include "GameEngine.h"
#include <memory>

namespace
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
}

void GameEngine::run()
{
    initialize();

    while (m_isRunning && !WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();
        processEvents();
        update(deltaTime);
        render();
    }

    shutdown();
}

void GameEngine::initialize()
{
    if (m_isInitialized)
        return;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "The First Step - Turn-based RPG");
    SetTargetFPS(60);
    m_stateMachine.changeState(GameStateMachine::State::MENU);
    m_isRunning = true;
    m_isInitialized = true;
}

void GameEngine::processEvents()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
            cancelLastAction();
        else
            m_isRunning = false;
    }

    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:
        if (IsKeyPressed(KEY_ENTER))
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        break;

    case GameStateMachine::State::IN_BATTLE:
        // Placeholder input: press V to simulate victory, G to simulate defeat.
        if (IsKeyPressed(KEY_V))
        {
            m_commandStack.clear();
            m_stateMachine.changeState(GameStateMachine::State::VICTORY);
        }
        else if (IsKeyPressed(KEY_G))
        {
            m_commandStack.clear();
            m_stateMachine.changeState(GameStateMachine::State::GAME_OVER);
        }
        break;

    case GameStateMachine::State::VICTORY:
    case GameStateMachine::State::GAME_OVER:
        if (IsKeyPressed(KEY_ENTER))
        {
            m_commandStack.clear();
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        }
        break;
    }
}

void GameEngine::update(float deltaTime)
{
    (void)deltaTime;
    // Cập nhật animation, turn manager và AI tại đây khi tích hợp các module.
}

void GameEngine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:
        DrawText("THE FIRST STEP", 440, 210, 40, DARKBLUE);
        DrawText("Press ENTER to start", 470, 300, 24, DARKGRAY);
        DrawText("ESC to exit", 530, 350, 20, GRAY);
        break;

    case GameStateMachine::State::IN_BATTLE:
        DrawText("IN BATTLE", 520, 120, 36, MAROON);
        DrawText("V: simulate victory    G: simulate game over", 330, 200, 22, DARKGRAY);
        DrawText("ESC: cancel last queued action (if any)", 365, 245, 20, GRAY);
        DrawText(TextFormat("Pending action history: %i", static_cast<int>(m_commandStack.size())),
                 420, 300, 20, DARKGRAY);
        break;

    case GameStateMachine::State::VICTORY:
        DrawText("VICTORY!", 500, 250, 48, GREEN);
        DrawText("Press ENTER to return to menu", 390, 330, 22, DARKGRAY);
        break;

    case GameStateMachine::State::GAME_OVER:
        DrawText("GAME OVER", 480, 250, 48, RED);
        DrawText("Press ENTER to return to menu", 390, 330, 22, DARKGRAY);
        break;
    }

    EndDrawing();
}

void GameEngine::shutdown()
{
    if (m_isInitialized)
    {
        CloseWindow();
        m_isInitialized = false;
    }

    m_isRunning = false;
}

void GameEngine::requestState(GameStateMachine::State state)
{
    m_stateMachine.changeState(state);
}

bool GameEngine::queuePlayerCommand(std::unique_ptr<ICommand> command)
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE) || !command)
        return false;

    m_commandStack.executeCommand(std::move(command));
    return true;
}

bool GameEngine::cancelLastAction()
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
        return false;

    return m_commandStack.cancelLastAction();
}
