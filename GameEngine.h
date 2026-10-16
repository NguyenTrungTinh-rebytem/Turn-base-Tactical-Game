#pragma once

#include <string>
#include "GameStateMachine.h"

// Owns the window and runs the main game loop:
//   while (window is open) { processInput(); update(dt); render(); }
// Each of the three steps dispatches on the current State.
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
    void handleBattleInput();
    void handleResultInput();

    // ---- update per state
    void updateBattle(float deltaTime);

    // ---- render per state
    void drawMenu() const;
    void drawBattle() const;
    void drawVictory() const;
    void drawGameOver() const;

private:
    int m_screenWidth;
    int m_screenHeight;
    std::string m_title;

    GameStateMachine m_stateMachine;
    float m_battleTime;   // seconds spent in the current battle (placeholder state)
};