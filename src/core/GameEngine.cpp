#include "GameEngine.h"
#include <utility>
#include "raylib.h"

GameEngine::GameEngine(int screenWidth, int screenHeight, std::string title)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_title(std::move(title))
    , m_battleTime(0.0f)
{
    InitWindow(m_screenWidth, m_screenHeight, m_title.c_str());
    SetTargetFPS(60);
}

GameEngine::~GameEngine()
{
    CloseWindow();
}

// ============================================================ main loop
void GameEngine::run()
{
    while (!WindowShouldClose())
    {
        processInput();               // 1. read events
        update(GetFrameTime());       // 2. advance the game by the time of one frame
        render();                     // 3. draw the result
    }
}

// ============================================================ 1. input
void GameEngine::processInput()
{
    switch (m_stateMachine.getState())
    {
    case State::MENU:      handleMenuInput();   break;
    case State::IN_BATTLE: handleBattleInput(); break;
    case State::VICTORY:
    case State::GAME_OVER: handleResultInput(); break;
    }
}

void GameEngine::handleMenuInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        m_battleTime = 0.0f;
        m_stateMachine.changeState(State::IN_BATTLE);
    }
}

// PLACEHOLDER: the real battle input (cards, commands) is added by later tasks.
// V / G only exist so the state transitions can be tried out.
void GameEngine::handleBattleInput()
{
    if (IsKeyPressed(KEY_V))
    {
        m_stateMachine.changeState(State::VICTORY);
    }
    else if (IsKeyPressed(KEY_G))
    {
        m_stateMachine.changeState(State::GAME_OVER);
    }
}

void GameEngine::handleResultInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        m_stateMachine.changeState(State::MENU);
    }
}

// ============================================================ 2. update
void GameEngine::update(float deltaTime)
{
    switch (m_stateMachine.getState())
    {
    case State::IN_BATTLE: updateBattle(deltaTime); break;
    case State::MENU:
    case State::VICTORY:
    case State::GAME_OVER: break;   // nothing to simulate
    }
}

void GameEngine::updateBattle(float deltaTime)
{
    m_battleTime += deltaTime;
}

// ============================================================ 3. render
void GameEngine::render() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (m_stateMachine.getState())
    {
    case State::MENU:      drawMenu();     break;
    case State::IN_BATTLE: drawBattle();   break;
    case State::VICTORY:   drawVictory();  break;
    case State::GAME_OVER: drawGameOver(); break;
    }

    EndDrawing();
}

void GameEngine::drawMenu() const
{
    DrawText("MENU", m_screenWidth / 2 - 60, m_screenHeight / 3, 48, DARKBLUE);
    DrawText("Press ENTER to start battle", m_screenWidth / 2 - 170, m_screenHeight / 2, 26, DARKGRAY);
}

void GameEngine::drawBattle() const
{
    DrawText("IN BATTLE", 40, 40, 36, BLACK);
    DrawText(TextFormat("Time: %.1f s", m_battleTime), 40, 90, 24, DARKGRAY);
    DrawText("[V] victory   [G] game over   (placeholder)", 40, m_screenHeight - 60, 20, DARKGRAY);
}

void GameEngine::drawVictory() const
{
    DrawText("VICTORY!", m_screenWidth / 2 - 130, m_screenHeight / 3, 56, DARKGREEN);
    DrawText("Press ENTER to return to menu", m_screenWidth / 2 - 190, m_screenHeight / 2, 24, DARKGRAY);
}

void GameEngine::drawGameOver() const
{
    DrawText("GAME OVER", m_screenWidth / 2 - 150, m_screenHeight / 3, 56, MAROON);
    DrawText("Press ENTER to return to menu", m_screenWidth / 2 - 190, m_screenHeight / 2, 24, DARKGRAY);
}