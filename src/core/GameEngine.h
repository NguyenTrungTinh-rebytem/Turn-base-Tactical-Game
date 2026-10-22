#pragma once

#include "GameStateMachine.h"
#include "CommandStack.h"
#include "data/GameData.h"
#include "data/Skill.h"
#include "dsa/TurnManager.h"
#include "graphics/BattleMap.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "entities/Enemy.h"
#include "entities/Boss.h"
#include "raylib.h"

#include <vector>
#include <memory>
#include <string>

class GameEngine
{
    private:
    float m_aiTurnTimer = 0.0f;
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
    void setupBattle();
    void processEvents();
    void update(float deltaTime);
    void render();
    void renderBattleUI();
    void shutdown();

    void processPlayerInput();
    void executeAiTurn(Character* aiChar);
    void checkBattleCondition();

    // Core Managers & State Machine
    GameStateMachine m_stateMachine;
    CommandStack m_commandStack;
    GameData m_gameData;
    TurnManager m_turnManager;
    BattleMap m_battleMap;

    // Entity lists
    std::vector<std::unique_ptr<Character>> m_combatants;
    std::vector<Skill> m_heroSkills;

    // Battle Tracking
    Character* m_currentCharacter = nullptr;
    Character* m_selectedTarget = nullptr;

    // UI state
    int m_selectedSkillIndex = 0;
    bool m_isSelectingSkill = false;
    std::string m_battleLog = "Chao mừng den voi Battle Demo!";

    bool m_isRunning = false;
    bool m_isInitialized = false;
};