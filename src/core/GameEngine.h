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

    // Enum quan ly Phase tran dau ro rang
    enum class BattlePhase {
        PLAYER_TURN,
        AI_THINKING,
        CHECK_END
    };

    // Core Managers
    GameStateMachine m_stateMachine;
    CommandStack m_commandStack;
    GameData m_gameData;
    TurnManager m_turnManager;
    BattleMap m_battleMap;

    // Entity lists
    std::vector<std::unique_ptr<Character>> m_combatants;
    std::vector<Skill> m_heroSkills;

    // Tracking State
    Character* m_currentCharacter = nullptr;
    Character* m_selectedTarget = nullptr;
    BattlePhase m_battlePhase = BattlePhase::PLAYER_TURN;

    // UI & Timer
    int m_selectedSkillIndex = 0;
    bool m_isSelectingSkill = false;
    float m_aiTurnTimer = 0.0f;
    std::string m_battleLog = "Chao mung den voi Battle Engine!";

    bool m_isRunning = false;
    bool m_isInitialized = false;
};