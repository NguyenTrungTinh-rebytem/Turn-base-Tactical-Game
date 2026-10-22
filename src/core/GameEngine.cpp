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

    // 1. Nạp dữ liệu CSV
    try {
        m_gameData.loadAll("data");
        m_battleLog = "Data CSV nap thanh cong!";
    }
    catch (const std::exception& e) {
        m_battleLog = std::string("Data load notice: ") + e.what();
    }

    // 2. Thiết lập sân đấu mặc định
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

    // Hero & Quái vật (Không dấu)
    Stats heroStats{ 120, 50, 25, 20, 5, 8, 15 };
    auto hero = std::make_unique<Player>("Chien Binh", heroStats, Element::KIM);

    m_heroSkills = {
        Skill{ "S01", "Chem Manh", SkillOwnerType::CLASS, "Warrior", SkillType::DAMAGE, 15, 0, EffectType::NONE, 0, Element::KIM, false },
        Skill{ "S02", "Hoa Cau", SkillOwnerType::CLASS, "Mage", SkillType::DAMAGE, 30, 15, EffectType::BURN, 2, Element::HOA, true },
        Skill{ "S03", "Hoi Mau", SkillOwnerType::CLASS, "Priest", SkillType::HEAL, 25, 20, EffectType::REGEN, 2, Element::THUY, true }
    };

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
    m_battleLog = "Tran dau bat dau! Luot cua: " + (m_currentCharacter ? m_currentCharacter->getName() : "None");
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
        processPlayerInput();
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

    // Chỉ nhận Input khi đến lượt của Player
    Player* player = dynamic_cast<Player*>(m_currentCharacter);
    if (!player) return;

    // Chuyển mục tiêu nhắm đánh (Tab hoặc Phím Mũi Tên)
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
    {
        for (size_t i = 1; i < m_combatants.size(); ++i) {
            if (m_combatants[i]->isAlive() && m_combatants[i].get() != m_selectedTarget) {
                m_selectedTarget = m_combatants[i].get();
                break;
            }
        }
    }

    // Phím A: Tấn công thường
if (IsKeyPressed(KEY_A) && m_selectedTarget)
{
    Character* attacker = player;
    Character* target = m_selectedTarget;

    // Khởi tạo AttackCommand với 2 con trỏ Character* đúng theo AttackCommand.h
    auto command = std::make_unique<AttackCommand>(attacker, target);

    int hpBefore = target->getCurrentHp();
    queuePlayerCommand(std::move(command)); // Khởi chạy lệnh tấn công
    int dealt = hpBefore - target->getCurrentHp();

    m_battleLog = attacker->getName() + " tấn công " + target->getName() + " gây " + std::to_string(dealt) + " sát thương!";
    
    // Chuyển lượt
    m_currentCharacter = m_turnManager.getNextTurn();
}

    // Phím S: Mở danh sách Skill
    if (IsKeyPressed(KEY_S))
    {
        m_isSelectingSkill = !m_isSelectingSkill;
    }

    // Chọn Kỹ năng khi menu Skill đang mở
    if (m_isSelectingSkill)
    {
        if (IsKeyPressed(KEY_ONE) && m_heroSkills.size() >= 1) m_selectedSkillIndex = 0;
        if (IsKeyPressed(KEY_TWO) && m_heroSkills.size() >= 2) m_selectedSkillIndex = 1;
        if (IsKeyPressed(KEY_THREE) && m_heroSkills.size() >= 3) m_selectedSkillIndex = 2;

        if (IsKeyPressed(KEY_SPACE) && m_selectedTarget)
        {
            Skill* skill = &m_heroSkills[m_selectedSkillIndex];
            if (player->getCurrentMp() >= skill->m_mpCost)
            {
                Character* target = (skill->m_type == SkillType::HEAL) ? player : m_selectedTarget;
                
                auto command = std::make_unique<UseSkillCommand>(
                    [player, skill, target, this]() {
                        int hpBefore = target->getCurrentHp();
                        if (skill->m_type == SkillType::HEAL) {
                            player->heal(skill->m_power);
                            m_battleLog = player->getName() + " dùng " + skill->m_name + " hồi " + std::to_string(skill->m_power) + " HP!";
                        } else {
                            player->executeSkill(skill, target);
                            int dealt = hpBefore - target->getCurrentHp();
                            m_battleLog = player->getName() + " dùng " + skill->m_name + " gây " + std::to_string(dealt) + " sát thương!";
                        }
                    },
                    []() {}
                );

                queuePlayerCommand(std::move(command));
                m_isSelectingSkill = false;
                m_currentCharacter = m_turnManager.getNextTurn();
            }
            else
            {
                m_battleLog = "Không đủ MP để dùng " + skill->m_name + "!";
            }
        }
    }
}

void GameEngine::executeAiTurn(Character* aiChar)
{
    if (!aiChar || !aiChar->isAlive()) return;

    // Tìm mục tiêu tấn công là Chiến Binh (Player)
    Character* hero = m_combatants[0].get();
    if (!hero || !hero->isAlive()) return;

    int hpBefore = hero->getCurrentHp();

    // Thực hiện đòn đánh dựa trên chỉ số công của AI
    int rawDmg = aiChar->getStats().m_a;
    hero->takeDamage(rawDmg, aiChar->getElement(), false);

    int dealt = hpBefore - hero->getCurrentHp();

    // Cập nhật Battle Log cho đòn đánh của AI
    m_battleLog = aiChar->getName() + " tan cong " + hero->getName() + " gay " + std::to_string(dealt) + " sat thuong!";
}

void GameEngine::update(float deltaTime)
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) return;

    checkBattleCondition();

    // Xử lý lượt đánh của AI (Bất kỳ nhân vật nào KHÔNG PHẢI là Player)
    if (m_currentCharacter && m_currentCharacter->isAlive())
    {
        Player* isPlayer = dynamic_cast<Player*>(m_currentCharacter);

        if (!isPlayer) // Nếu là AI (Goblin, Ma Vương, ...)
        {
            m_aiTurnTimer += deltaTime;

            // Chờ 0.8 giây để người chơi nhìn rõ lượt và Battle Log
            if (m_aiTurnTimer >= 0.8f)
            {
                executeAiTurn(m_currentCharacter);
                m_currentCharacter = m_turnManager.getNextTurn();
                m_aiTurnTimer = 0.0f; // Reset timer cho lượt sau
            }
        }
        else
        {
            m_aiTurnTimer = 0.0f; // Reset timer khi đến lượt Người chơi
        }
    }
}
void GameEngine::checkBattleCondition()
{
    // 1. Kiểm tra Hero chết -> GAME_OVER
    if (m_combatants.empty() || !m_combatants[0]->isAlive())
    {
        m_stateMachine.changeState(GameStateMachine::State::GAME_OVER);
        return;
    }

    // 2. Kiểm tra toàn bộ Enemy/Boss chết -> VICTORY
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
    // 1. Ve thong tin Nhan vat & Thanh HP/MP tren San dau
    for (size_t i = 0; i < m_combatants.size(); ++i)
    {
        auto& c = m_combatants[i];
        if (!c->isAlive()) continue;

        BattleSlot slot = (i == 0) ? m_battleMap.getPlayerSlot(0, 1) 
                                   : m_battleMap.getEnemySlot((int)i - 1, (int)m_combatants.size() - 1, (i == 2 ? 1.3f : 1.0f));

        Vector2 pos = slot.m_feetPos;
        int x = (int)pos.x - 60;
        int y = (int)pos.y - 120;

        // Khung mau chon Target
        if (c.get() == m_selectedTarget) {
            DrawRectangleLines(x - 5, y - 5, 130, 90, GOLD);
        }

        // Ten & Luot
        Color titleColor = (c.get() == m_currentCharacter) ? YELLOW : WHITE;
        DrawText(c->getName().c_str(), x, y, 18, titleColor);

        // HP Bar
        float hpRatio = (float)c->getCurrentHp() / (float)c->getStats().m_hp;
        DrawRectangle(x, y + 25, 120, 12, RED);
        DrawRectangle(x, y + 25, (int)(120 * hpRatio), 12, GREEN);
        DrawRectangleLines(x, y + 25, 120, 12, BLACK);
        DrawText(TextFormat("%d/%d", c->getCurrentHp(), c->getStats().m_hp), x + 25, y + 26, 10, WHITE);

        // MP Bar
        if (c->getStats().m_mp > 0) {
            float mpRatio = (float)c->getCurrentMp() / (float)c->getStats().m_mp;
            DrawRectangle(x, y + 42, 120, 8, DARKGRAY);
            DrawRectangle(x, y + 42, (int)(120 * mpRatio), 8, BLUE);
            DrawRectangleLines(x, y + 42, 120, 8, BLACK);
        }
    }

    // 2. Bang Huong dan & Battle Log phia duoi
    DrawRectangle(20, 560, 1240, 140, Fade(BLACK, 0.8f));
    DrawRectangleLines(20, 560, 1240, 140, RAYWHITE);

    DrawText("DEN LUOT:", 40, 575, 20, YELLOW);
    DrawText(m_currentCharacter ? m_currentCharacter->getName().c_str() : "None", 160, 575, 20, GREEN);

    DrawText("DIEU KHIEN: [A] Tan Cong  |  [S] Ky Nang  |  [TAB] Doi Muc Tieu  |  [ESC/Z] Undo", 40, 610, 18, LIGHTGRAY);
    DrawText(TextFormat("BATTLE LOG: %s", m_battleLog.c_str()), 40, 650, 20, ORANGE);

    // 3. Menu chon Skill khi nhan S
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
        m_battleLog = "Đã hoàn tác (Undo) hành động vừa rồi!";
    }
    return result;
}