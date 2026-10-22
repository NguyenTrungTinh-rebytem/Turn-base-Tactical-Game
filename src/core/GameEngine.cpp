#include "core/GameEngine.h"
#include "core/AttackCommand.h"
#include "core/UseSkillCommand.h"
#include <iostream>
#include <algorithm>

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

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2.5D RPG Battle Engine - Demo");
    SetTargetFPS(60);

    try {
        m_gameData.loadAll("data");
        m_battleLog = "Data CSV nap thanh cong!";
    }
    catch (const std::exception& e) {
        m_battleLog = std::string("Data load notice: ") + e.what();
    }

    m_battleMap.setStage(StageId::STAGE_1);

    m_stateMachine.changeState(GameStateMachine::State::MENU);
    m_isRunning = true;
    m_isInitialized = true;
}

void GameEngine::setupBattle()
{
    m_combatants.clear();
    m_turnManager.clear();
    m_commandStack.clear();
    m_aiTurnTimer = 0.0f;

    // 1. Khoi tao Hero (m_combatants[0])
    Stats heroStats{ 120, 50, 25, 20, 5, 8, 15 };
    auto hero = std::make_unique<Player>("Chien Binh", heroStats, Element::KIM);

    m_heroSkills = {
        Skill{ "S01", "Chem Manh", SkillOwnerType::CLASS, "Warrior", SkillType::DAMAGE, 15, 0, EffectType::NONE, 0, Element::KIM, false },
        Skill{ "S02", "Hoa Cau", SkillOwnerType::CLASS, "Mage", SkillType::DAMAGE, 30, 15, EffectType::BURN, 2, Element::HOA, true },
        Skill{ "S03", "Hoi Mau", SkillOwnerType::CLASS, "Priest", SkillType::HEAL, 25, 20, EffectType::REGEN, 2, Element::THUY, true }
    };

    // 2. Khoi tao Quai vat (m_combatants[1], m_combatants[2])
    Stats goblinStats{ 60, 0, 28, 0, 5, 2, 10 };
    auto goblin = std::make_unique<Enemy>("Goblin", goblinStats, Element::MOC);

    Stats bossStats{ 200, 40, 30, 25, 12, 10, 8 };
    auto boss = std::make_unique<Boss>("Ma Vuong", bossStats, Element::HOA);

    m_turnManager.addCharacter(hero.get());
    m_turnManager.addCharacter(goblin.get());
    m_turnManager.addCharacter(boss.get());

    m_combatants.push_back(std::move(hero));
    m_combatants.push_back(std::move(goblin));
    m_combatants.push_back(std::move(boss));

    m_currentCharacter = m_turnManager.getNextTurn();
    m_selectedTarget = m_combatants[1].get();
    m_battlePhase = BattlePhase::PLAYER_TURN;
    m_battleLog = "Tran dau bat dau! Den luot: " + (m_currentCharacter ? m_currentCharacter->getName() : "None");
}

void GameEngine::processEvents()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
        {
            if (m_isSelectingSkill) {
                m_isSelectingSkill = false;
            } else if (!cancelLastAction()) {
                m_stateMachine.changeState(GameStateMachine::State::MENU);
            }
        }
        else
        {
            m_isRunning = false;
        }
    }

    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:
        if (IsKeyPressed(KEY_ENTER))
        {
            setupBattle();
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        }
        break;

    case GameStateMachine::State::IN_BATTLE:
        // CHI XU LY INPUT KHI DANG TRONG PHASE LUOT CUA PLAYER
        if (m_battlePhase == BattlePhase::PLAYER_TURN)
        {
            processPlayerInput();
        }
        break;

    case GameStateMachine::State::VICTORY:
    case GameStateMachine::State::GAME_OVER:
        if (IsKeyPressed(KEY_ENTER))
        {
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        }
        break;
    }
}

void GameEngine::processPlayerInput()
{
    if (!m_currentCharacter) return;

    // Doi muc tieu (TAB hoac Mui ten)
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
    {
        for (size_t i = 1; i < m_combatants.size(); ++i) {
            if (m_combatants[i]->isAlive() && m_combatants[i].get() != m_selectedTarget) {
                m_selectedTarget = m_combatants[i].get();
                break;
            }
        }
    }

    // Phim A: Tan cong thuong
    if (IsKeyPressed(KEY_A) && m_selectedTarget)
    {
        Character* attacker = m_combatants[0].get(); // Chien Binh
        Character* target = m_selectedTarget;

        int hpBefore = target->getCurrentHp();

        // Truyen 2 con tro attacker, target theo dung AttackCommand(Character*, Character*)
        auto command = std::make_unique<AttackCommand>(attacker, target);
        queuePlayerCommand(std::move(command));

        int dealt = hpBefore - target->getCurrentHp();
        m_battleLog = attacker->getName() + " tan cong " + target->getName() + " gay " + std::to_string(dealt) + " sat thuong!";

        // Lay luot tiep theo
        m_currentCharacter = m_turnManager.getNextTurn();
        m_aiTurnTimer = 0.0f;

        // Kiem tra luot tiep theo thuoc ve ai
        if (m_currentCharacter == m_combatants[0].get()) {
            m_battlePhase = BattlePhase::PLAYER_TURN;
        } else {
            m_battlePhase = BattlePhase::AI_THINKING;
        }
    }

    // Phim S: Biet/Tat Menu Skill
    if (IsKeyPressed(KEY_S))
    {
        m_isSelectingSkill = !m_isSelectingSkill;
    }

    if (m_isSelectingSkill)
    {
        if (IsKeyPressed(KEY_ONE) && m_heroSkills.size() >= 1) m_selectedSkillIndex = 0;
        if (IsKeyPressed(KEY_TWO) && m_heroSkills.size() >= 2) m_selectedSkillIndex = 1;
        if (IsKeyPressed(KEY_THREE) && m_heroSkills.size() >= 3) m_selectedSkillIndex = 2;

        if (IsKeyPressed(KEY_SPACE) && m_selectedTarget)
        {
            Skill* skill = &m_heroSkills[m_selectedSkillIndex];
            Character* player = m_combatants[0].get();

            if (player->getCurrentMp() >= skill->m_mpCost)
            {
                Character* target = (skill->m_type == SkillType::HEAL) ? player : m_selectedTarget;
                int hpBefore = target->getCurrentHp();

                player->executeSkill(skill, target);

                if (skill->m_type == SkillType::HEAL) {
                    m_battleLog = player->getName() + " dung " + skill->m_name + " hoi HP!";
                } else {
                    int dealt = hpBefore - target->getCurrentHp();
                    m_battleLog = player->getName() + " dung " + skill->m_name + " gay " + std::to_string(dealt) + " sat thuong!";
                }

                m_isSelectingSkill = false;
                m_currentCharacter = m_turnManager.getNextTurn();
                m_aiTurnTimer = 0.0f;

                if (m_currentCharacter == m_combatants[0].get()) {
                    m_battlePhase = BattlePhase::PLAYER_TURN;
                } else {
                    m_battlePhase = BattlePhase::AI_THINKING;
                }
            }
            else
            {
                m_battleLog = "Khong du MP de dung " + skill->m_name + "!";
            }
        }
    }
}

void GameEngine::executeAiTurn(Character* aiChar)
{
    if (!aiChar || !aiChar->isAlive()) return;

    Character* hero = m_combatants[0].get();
    if (!hero || !hero->isAlive()) return;

    int hpBefore = hero->getCurrentHp();

    int rawDmg = aiChar->getStats().m_a;
    hero->takeDamage(rawDmg, aiChar->getElement(), false);

    int dealt = hpBefore - hero->getCurrentHp();
    m_battleLog = aiChar->getName() + " tan cong " + hero->getName() + " gay " + std::to_string(dealt) + " sat thuong!";
}

void GameEngine::update(float deltaTime)
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) return;

    checkBattleCondition();

    if (!m_currentCharacter)
    {
        m_currentCharacter = m_turnManager.getNextTurn();
        return;
    }

    // TIEP TUC XU LY PHASE AI
    if (m_battlePhase == BattlePhase::AI_THINKING)
    {
        if (m_currentCharacter && m_currentCharacter->isAlive() && m_currentCharacter != m_combatants[0].get())
        {
            m_aiTurnTimer += deltaTime;

            // Cho dung 1.2 giay cho moi luot cua Quai vat
            if (m_aiTurnTimer >= 1.2f)
            {
                executeAiTurn(m_currentCharacter);

                // Lay luot ke tiếp
                m_currentCharacter = m_turnManager.getNextTurn();
                m_aiTurnTimer = 0.0f;

                // Neu luot tiep theo quay ve Player -> Doi Phase
                if (m_currentCharacter == m_combatants[0].get())
                {
                    m_battlePhase = BattlePhase::PLAYER_TURN;
                }
            }
        }
        else
        {
            m_battlePhase = BattlePhase::PLAYER_TURN;
        }
    }
}

void GameEngine::checkBattleCondition()
{
    if (m_combatants.empty() || !m_combatants[0]->isAlive())
    {
        m_stateMachine.changeState(GameStateMachine::State::GAME_OVER);
        return;
    }

    bool allEnemiesDead = true;
    for (size_t i = 1; i < m_combatants.size(); ++i) {
        if (m_combatants[i]->isAlive()) {
            allEnemiesDead = false;
            break;
        }
    }

    if (allEnemiesDead)
    {
        m_stateMachine.changeState(GameStateMachine::State::VICTORY);
    }
}

void GameEngine::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:
        DrawText("2.5D RPG BATTLE ENGINE", 380, 220, 40, DARKBLUE);
        DrawText("Press ENTER to Start Battle", 440, 320, 24, DARKGRAY);
        DrawText("ESC to Exit", 560, 370, 20, GRAY);
        break;

    case GameStateMachine::State::IN_BATTLE:
        m_battleMap.draw();
        renderBattleUI();
        break;

    case GameStateMachine::State::VICTORY:
        DrawText("VICTORY!", 500, 250, 50, GREEN);
        DrawText("Press ENTER to return to Menu", 420, 340, 22, DARKGRAY);
        break;

    case GameStateMachine::State::GAME_OVER:
        DrawText("GAME OVER", 480, 250, 50, RED);
        DrawText("Press ENTER to return to Menu", 420, 340, 22, DARKGRAY);
        break;
    }

    EndDrawing();
}

void GameEngine::renderBattleUI()
{
    for (size_t i = 0; i < m_combatants.size(); ++i)
    {
        auto& c = m_combatants[i];
        if (!c->isAlive()) continue;

        BattleSlot slot = (i == 0) ? m_battleMap.getPlayerSlot(0, 1) 
                                   : m_battleMap.getEnemySlot((int)i - 1, (int)m_combatants.size() - 1, (i == 2 ? 1.3f : 1.0f));

        Vector2 pos = slot.m_feetPos;
        int x = (int)pos.x - 60;
        int y = (int)pos.y - 120;

        if (c.get() == m_selectedTarget) {
            DrawRectangleLines(x - 5, y - 5, 130, 90, GOLD);
        }

        Color titleColor = (c.get() == m_currentCharacter) ? YELLOW : WHITE;
        DrawText(c->getName().c_str(), x, y, 18, titleColor);

        float hpRatio = (float)c->getCurrentHp() / (float)c->getStats().m_hp;
        DrawRectangle(x, y + 25, 120, 12, RED);
        DrawRectangle(x, y + 25, (int)(120 * hpRatio), 12, GREEN);
        DrawRectangleLines(x, y + 25, 120, 12, BLACK);
        DrawText(TextFormat("%d/%d", c->getCurrentHp(), c->getStats().m_hp), x + 25, y + 26, 10, WHITE);

        if (c->getStats().m_mp > 0) {
            float mpRatio = (float)c->getCurrentMp() / (float)c->getStats().m_mp;
            DrawRectangle(x, y + 42, 120, 8, DARKGRAY);
            DrawRectangle(x, y + 42, (int)(120 * mpRatio), 8, BLUE);
            DrawRectangleLines(x, y + 42, 120, 8, BLACK);
        }
    }

    DrawRectangle(20, 560, 1240, 140, Fade(BLACK, 0.8f));
    DrawRectangleLines(20, 560, 1240, 140, RAYWHITE);

    DrawText("DEN LUOT:", 40, 575, 20, YELLOW);
    if (m_currentCharacter)
    {
        if (m_currentCharacter == m_combatants[0].get()) {
            DrawText(m_currentCharacter->getName().c_str(), 160, 575, 20, GREEN);
        } else {
            std::string aiTurnText = m_currentCharacter->getName() + " (Dang suy nghi...)";
            DrawText(aiTurnText.c_str(), 160, 575, 20, RED);
        }
    }

    DrawText("DIEU KHIEN: [A] Tan Cong  |  [S] Ky Nang  |  [TAB] Doi Muc Tieu  |  [ESC/Z] Undo", 40, 610, 18, LIGHTGRAY);
    DrawText(TextFormat("BATTLE LOG: %s", m_battleLog.c_str()), 40, 650, 20, ORANGE);

    if (m_isSelectingSkill)
    {
        DrawRectangle(400, 300, 480, 220, Fade(DARKBLUE, 0.95f));
        DrawRectangleLines(400, 300, 480, 220, SKYBLUE);
        DrawText("DANH SACH KY NANG (Bam 1-3 de chon, SPACE de dung)", 420, 315, 16, YELLOW);

        for (size_t i = 0; i < m_heroSkills.size(); ++i)
        {
            Color txtColor = (m_selectedSkillIndex == (int)i) ? GREEN : WHITE;
            std::string skText = TextFormat("[%d] %s (MP: %d, Power: %d)", 
                (int)i + 1, m_heroSkills[i].m_name.c_str(), m_heroSkills[i].m_mpCost, m_heroSkills[i].m_power);
            DrawText(skText.c_str(), 430, 350 + (int)i * 35, 18, txtColor);
        }
    }
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

    bool result = m_commandStack.cancelLastAction();
    if (result) {
        m_battleLog = "Da hoan tac (Undo) hanh dong vua roi!";
    }
    return result;
}