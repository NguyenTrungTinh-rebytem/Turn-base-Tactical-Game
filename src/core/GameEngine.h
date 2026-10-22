#pragma once

#include <memory>
#include <string>
#include "GameStateMachine.h"

// Owns the window and runs the main game loop:
//   while (window is open) { processInput(); update(dt); render(); }
// Each step dispatches on the current State of the GameStateMachine.
class GameEngine
{
public:
    GameEngine(int screenWidth, int screenHeight, std::string title);
    ~GameEngine();

    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    void run();

private:
    // ---- the three steps of one frame
    void processInput();
    void update(float deltaTime);
    void render() const;

    // ---- input per state
    void handleMenuInput();
    void handlePlayerTurnInput();
    void handleResultInput();

    // ---- battle flow
    void startBattle();
    void finishPlayerAction();   // called after the player used up the turn
    void updateEnemyTurn(float deltaTime);

    // ---- drawing
    void drawMenu() const;
    void drawBattle() const;
    void drawCommands() const;
    void drawResult(const char* title, bool canRetry) const;

private:
    int m_screenWidth;
    int m_screenHeight;
    std::string m_title;

    GameStateMachine m_stateMachine;
    std::unique_ptr<BattleSystem> m_battle;   // null while in the menu
    float m_enemyTimer;                       // short pause so the player can read the log
};