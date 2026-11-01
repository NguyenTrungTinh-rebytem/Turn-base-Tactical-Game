#include "core/GameEngine.h"
#include <cassert>
#include <iostream>
#include <memory>

struct GameEngineTestAccess {
    static void prepareBossWave(GameEngine& engine) {
        auto player = std::make_shared<Player>(
            "Test Hero", Stats{ 1000, 0, 50, 0, 0, 0, 20 }, Element::KIM);
        auto boss = std::make_shared<Boss>(
            "Test Boss", Stats{ 10000, 0, 40, 0, 0, 0, 10 }, Element::HOA);

        const bool memberAdded = engine.m_party.addMember(player);
        assert(memberAdded);
        engine.m_combatants = { player, boss };
        engine.m_views.emplace_back(std::make_unique<CombatantView>(
            player.get(), "missing-test-hero.png", 128, 128,
            BattleSlot{ Vector2{ 350.0f, 500.0f }, 1.0f, false, 0.5f }));
        engine.m_views.emplace_back(std::make_unique<CombatantView>(
            boss.get(), "missing-test-boss.png", 192, 192,
            BattleSlot{ Vector2{ 905.2f, 475.2f }, 1.92f, true, 0.5f }));

        engine.m_turnManager.addCharacter(player.get());
        engine.m_turnManager.addCharacter(boss.get());
        engine.m_lastHp[player.get()] = player->getCurrentHp();
        engine.m_lastHp[boss.get()] = boss->getCurrentHp();
        engine.m_stage = 2;
        engine.m_finalStageCache = 2;
        engine.m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        engine.m_currentCharacter = engine.m_turnManager.getNextTurn();
        assert(engine.m_currentCharacter == player.get());
    }

    static void click(GameEngine& engine, Vector2 position) {
        engine.handleBattleClick(position);
    }

    static void playEnemyTurnAndAdvance(GameEngine& engine) {
        engine.executeAiTurn(engine.m_currentCharacter);
        engine.advanceTurn();
    }

    static bool isSelectingTarget(const GameEngine& engine) {
        return engine.m_isSelectingTarget;
    }

    static int bossHp(const GameEngine& engine) {
        return engine.m_combatants[1]->getCurrentHp();
    }

    static Character* currentCharacter(const GameEngine& engine) {
        return engine.m_currentCharacter;
    }

    static bool isConfirming(const GameEngine& engine) {
        return engine.m_battlePhase == GameEngine::BattlePhase::PLAYER_CONFIRM;
    }

    static bool isEnemyThinking(const GameEngine& engine) {
        return engine.m_battlePhase == GameEngine::BattlePhase::AI_THINKING;
    }

    static bool isPlayerTurn(const GameEngine& engine) {
        return engine.m_battlePhase == GameEngine::BattlePhase::PLAYER_TURN;
    }

    static bool isCurrentHero(const GameEngine& engine) {
        return engine.m_currentCharacter == engine.m_party.getLeader();
    }

    static const CombatantView& bossView(const GameEngine& engine) {
        return *engine.m_views[1];
    }
};

int main() {
    GameEngine engine;
    GameEngineTestAccess::prepareBossWave(engine);

    const Rectangle bossBounds = GameEngineTestAccess::bossView(engine).getDrawBounds();
    assert(bossBounds.width > 350.0f);
    const int hpBefore = GameEngineTestAccess::bossHp(engine);

    const Rectangle attackButton = BattleUI::buttonBounds(BattleUIButton::ATTACK);
    GameEngineTestAccess::click(engine, Vector2{
        attackButton.x + attackButton.width / 2.0f,
        attackButton.y + attackButton.height / 2.0f
    });
    assert(GameEngineTestAccess::isSelectingTarget(engine));

    // Click near the rendered upper-right edge: outside a normal 64x64 enemy hitbox.
    GameEngineTestAccess::click(engine, Vector2{
        bossBounds.x + bossBounds.width - 10.0f,
        bossBounds.y + 10.0f
    });
    assert(GameEngineTestAccess::bossHp(engine) < hpBefore);
    assert(GameEngineTestAccess::isConfirming(engine));

    const Rectangle undoButton = BattleUI::buttonBounds(BattleUIButton::UNDO);
    GameEngineTestAccess::click(engine, Vector2{
        undoButton.x + undoButton.width / 2.0f,
        undoButton.y + undoButton.height / 2.0f
    });
    assert(GameEngineTestAccess::bossHp(engine) == hpBefore);
    assert(GameEngineTestAccess::isPlayerTurn(engine));

    GameEngineTestAccess::click(engine, Vector2{
        attackButton.x + attackButton.width / 2.0f,
        attackButton.y + attackButton.height / 2.0f
    });
    GameEngineTestAccess::click(engine, Vector2{
        bossBounds.x + bossBounds.width - 10.0f,
        bossBounds.y + 10.0f
    });
    assert(GameEngineTestAccess::bossHp(engine) < hpBefore);
    assert(GameEngineTestAccess::isConfirming(engine));

    const Rectangle endTurnButton = BattleUI::buttonBounds(BattleUIButton::END_TURN);
    GameEngineTestAccess::click(engine, Vector2{
        endTurnButton.x + endTurnButton.width / 2.0f,
        endTurnButton.y + endTurnButton.height / 2.0f
    });
    assert(GameEngineTestAccess::currentCharacter(engine) != nullptr);
    assert(GameEngineTestAccess::isEnemyThinking(engine));

    GameEngineTestAccess::playEnemyTurnAndAdvance(engine);
    assert(GameEngineTestAccess::currentCharacter(engine) != nullptr);
    assert(GameEngineTestAccess::isCurrentHero(engine));
    assert(GameEngineTestAccess::isPlayerTurn(engine));

    std::cout << "BOSS MOUSE TARGETING TEST PASSED\n";
}
