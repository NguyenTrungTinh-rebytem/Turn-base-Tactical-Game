#include "GameEngine.h"
#include <random>
#include <utility>
#include "raylib.h"

namespace
{
    constexpr float ENEMY_TURN_DELAY = 0.9f;   // seconds before the monster acts

    void drawBar(int x, int y, int width, int height, int value, int maxValue, Color fill)
    {
        DrawRectangle(x, y, width, height, LIGHTGRAY);
        if (maxValue > 0)
        {
            DrawRectangle(x, y, width * value / maxValue, height, fill);
        }
        DrawRectangleLines(x, y, width, height, DARKGRAY);
    }

    // PLACEHOLDER sprite: a coloured body and head.
    void drawUnit(const BattleUnit& unit, int x, int y, Color bodyColor, bool showMp)
    {
        DrawText(unit.getName().c_str(), x, y, 28, BLACK);

        DrawText(TextFormat("HP %d/%d", unit.getHp(), unit.getMaxHp()), x, y + 36, 18, BLACK);
        drawBar(x, y + 58, 260, 16, unit.getHp(), unit.getMaxHp(), RED);

        if (showMp)
        {
            DrawText(TextFormat("MP %d/%d", unit.getMp(), unit.getMaxMp()), x, y + 82, 18, BLACK);
            drawBar(x, y + 104, 260, 12, unit.getMp(), unit.getMaxMp(), SKYBLUE);
        }

        DrawCircle(x + 130, y + 170, 30, bodyColor);
        DrawRectangle(x + 85, y + 205, 90, 110, bodyColor);
        DrawRectangleLines(x + 85, y + 205, 90, 110, DARKGRAY);
    }
}

// ============================================================ lifecycle
GameEngine::GameEngine(int screenWidth, int screenHeight, std::string title)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_title(std::move(title))
    , m_enemyTimer(0.0f)
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
    case State::MENU:        handleMenuInput();       break;
    case State::PLAYER_TURN: handlePlayerTurnInput(); break;
    case State::ENEMY_TURN:  break;                   // no input while the monster acts
    case State::VICTORY:
    case State::GAME_OVER:   handleResultInput();     break;
    }
}

void GameEngine::handleMenuInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        startBattle();
    }
}

void GameEngine::handlePlayerTurnInput()
{
    bool acted = false;

    if (IsKeyPressed(KEY_ONE))        acted = m_battle->playerAttack();
    else if (IsKeyPressed(KEY_TWO))   acted = m_battle->playerUseSkill();
    else if (IsKeyPressed(KEY_THREE)) acted = m_battle->playerHeal();
    else if (IsKeyPressed(KEY_FOUR))  acted = m_battle->playerDefend();

    if (acted)
    {
        finishPlayerAction();
    }
}

void GameEngine::handleResultInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        m_stateMachine.changeState(State::MENU);
    }
    else if (IsKeyPressed(KEY_R) && m_stateMachine.isState(State::GAME_OVER))
    {
        startBattle();   // retry
    }
}

// ============================================================ battle flow
void GameEngine::startBattle()
{
    m_battle = std::make_unique<BattleSystem>(std::random_device{}());
    m_enemyTimer = 0.0f;
    m_stateMachine.changeState(State::PLAYER_TURN);
}

void GameEngine::finishPlayerAction()
{
    if (m_battle->getResult() == BattleResult::PLAYER_WON)
    {
        m_stateMachine.changeState(State::VICTORY);
        return;
    }

    m_enemyTimer = 0.0f;
    m_stateMachine.changeState(State::ENEMY_TURN);
}

// ============================================================ 2. update
void GameEngine::update(float deltaTime)
{
    switch (m_stateMachine.getState())
    {
    case State::ENEMY_TURN:  updateEnemyTurn(deltaTime); break;
    case State::MENU:
    case State::PLAYER_TURN:
    case State::VICTORY:
    case State::GAME_OVER:   break;   // nothing to simulate
    }
}

void GameEngine::updateEnemyTurn(float deltaTime)
{
    m_enemyTimer += deltaTime;
    if (m_enemyTimer < ENEMY_TURN_DELAY)
    {
        return;
    }

    m_battle->enemyAct();
    if (m_battle->getResult() == BattleResult::PLAYER_LOST)
    {
        m_stateMachine.changeState(State::GAME_OVER);
    }
    else
    {
        m_stateMachine.changeState(State::PLAYER_TURN);
    }
}

// ============================================================ 3. render
void GameEngine::render() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (m_stateMachine.getState())
    {
    case State::MENU:        drawMenu();                          break;
    case State::PLAYER_TURN:
    case State::ENEMY_TURN:  drawBattle();                        break;
    case State::VICTORY:     drawResult("VICTORY!", false);       break;
    case State::GAME_OVER:   drawResult("GAME OVER", true);       break;
    }

    EndDrawing();
}

void GameEngine::drawMenu() const
{
    DrawText("TURN-BASED TACTICAL RPG", m_screenWidth / 2 - 270, m_screenHeight / 3, 48, DARKBLUE);
    DrawText("Defeat all the monsters!", m_screenWidth / 2 - 140, m_screenHeight / 2 - 20, 24, DARKGRAY);
    DrawText("Press ENTER to start", m_screenWidth / 2 - 130, m_screenHeight / 2 + 30, 26, DARKGRAY);
}

void GameEngine::drawBattle() const
{
    const bool playerTurn = m_stateMachine.isState(State::PLAYER_TURN);

    DrawText(playerTurn ? "YOUR TURN" : "ENEMY TURN", m_screenWidth / 2 - 90, 24, 30,
             playerTurn ? DARKGREEN : MAROON);
    DrawText(TextFormat("Monster %d / %d", static_cast<int>(m_battle->getMonsterIndex()) + 1,
                        static_cast<int>(m_battle->getMonsterCount())),
             m_screenWidth / 2 - 80, 62, 22, DARKGRAY);

    drawUnit(m_battle->getPlayer(), 110, 100, BLUE, true);
    drawUnit(m_battle->getCurrentMonster(), m_screenWidth - 380, 100, RED, false);

    if (m_battle->isPlayerDefending())
    {
        DrawText("DEFENDING", 110, 100 + 330, 22, DARKBLUE);
    }

    // message log
    const std::vector<std::string>& log = m_battle->getLog();
    for (std::size_t i = 0; i < log.size(); ++i)
    {
        DrawText(log[i].c_str(), 60, 500 + static_cast<int>(i) * 24, 20, DARKGRAY);
    }

    if (playerTurn)
    {
        drawCommands();
    }
}

void GameEngine::drawCommands() const
{
    const int mp = m_battle->getPlayer().getMp();
    const bool canSkill = mp >= BattleSystem::SKILL_MP_COST;
    const bool canHeal = mp >= BattleSystem::HEAL_MP_COST;
    const int y = m_screenHeight - 50;

    DrawText("[1] Attack", 60, y, 22, BLACK);
    DrawText(TextFormat("[2] Power Slash (%d MP)", BattleSystem::SKILL_MP_COST), 260, y, 22, canSkill ? BLACK : GRAY);
    DrawText(TextFormat("[3] Heal +%d HP (%d MP)", BattleSystem::HEAL_AMOUNT, BattleSystem::HEAL_MP_COST), 580, y, 22,
             canHeal ? BLACK : GRAY);
    DrawText(TextFormat("[4] Defend (+%d MP)", BattleSystem::DEFEND_MP_GAIN), 960, y, 22, BLACK);
}

void GameEngine::drawResult(const char* title, bool canRetry) const
{
    DrawText(title, m_screenWidth / 2 - 140, m_screenHeight / 3, 56, canRetry ? MAROON : DARKGREEN);
    DrawText("Press ENTER to return to the menu", m_screenWidth / 2 - 210, m_screenHeight / 2, 24, DARKGRAY);
    if (canRetry)
    {
        DrawText("Press R to retry", m_screenWidth / 2 - 100, m_screenHeight / 2 + 40, 24, DARKGRAY);
    }
}