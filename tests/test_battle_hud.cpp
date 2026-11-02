#include "core/GameEngine.h"
#include "graphics/UiTheme.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

namespace {
bool overlaps(Rectangle left, Rectangle right) {
    return left.x < right.x + right.width && left.x + left.width > right.x &&
           left.y < right.y + right.height && left.y + left.height > right.y;
}

float area(Rectangle bounds) { return bounds.width * bounds.height; }

Vector2 center(Rectangle bounds) {
    return Vector2{ bounds.x + bounds.width * 0.5f,
                    bounds.y + bounds.height * 0.5f };
}
}

struct GameEngineTestAccess {
    static void prepareNormalWave(GameEngine& engine) {
        auto player = std::make_shared<Player>(
            "Test Hero", Stats{ 10000, 0, 100, 0, 0, 0, 30 }, Element::KIM);
        auto ally = std::make_shared<Player>(
            "Test Ally", Stats{ 10000, 0, 40, 0, 0, 0, 1 }, Element::THUY);
        auto enemy = std::make_shared<Enemy>(
            "Test Goblin", Stats{ 10000, 0, 10, 0, 0, 0, 5 }, Element::MOC);
        assert(engine.m_party.addMember(player));
        assert(engine.m_party.addMember(ally));
        ally->takeDamage(3000);
        engine.m_combatants = { player, ally, enemy };
        engine.m_views.emplace_back(std::make_unique<CombatantView>(
            player.get(), "missing-test-hero.png", 128, 128,
            BattleSlot{ Vector2{ 350.0f, 520.0f }, 1.0f, false, 0.5f }));
        engine.m_views.emplace_back(std::make_unique<CombatantView>(
            ally.get(), "missing-test-ally.png", 128, 128,
            BattleSlot{ Vector2{ 500.0f, 520.0f }, 1.0f, false, 0.5f }));
        engine.m_views.emplace_back(std::make_unique<CombatantView>(
            enemy.get(), "missing-test-enemy.png", 64, 64,
            BattleSlot{ Vector2{ 840.0f, 520.0f }, 1.5f, true, 0.5f }));
        engine.m_turnManager.addCharacter(player.get());
        engine.m_turnManager.addCharacter(ally.get());
        engine.m_turnManager.addCharacter(enemy.get());
        engine.m_lastHp[player.get()] = player->getCurrentHp();
        engine.m_lastHp[ally.get()] = ally->getCurrentHp();
        engine.m_lastHp[enemy.get()] = enemy->getCurrentHp();
        ItemData potionData;
        potionData.m_id = "TEST_POTION";
        potionData.m_name = "Test Potion";
        potionData.m_nameVi = u8"Bình thử nghiệm";
        potionData.m_descriptionVi = u8"Hồi một lượng HP cho đồng đội";
        potionData.m_category = ItemCategory::CONSUMABLE;
        potionData.m_effect = ItemEffect::HEAL_HP_PCT;
        potionData.m_value = 20;
        assert(engine.m_party.addItem(std::make_shared<Potion>(potionData)));
        engine.m_stage = 1;
        engine.m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        engine.m_currentCharacter = engine.m_turnManager.getNextTurn();
        assert(engine.m_currentCharacter == player.get());
    }

    static void click(GameEngine& engine, Vector2 position) {
        engine.handleBattleClick(position);
    }

    static int enemyHp(const GameEngine& engine) {
        return engine.m_combatants[2]->getCurrentHp();
    }

    static bool isSelectingTarget(const GameEngine& engine) {
        return engine.m_isSelectingTarget;
    }

    static bool isSelectingItem(const GameEngine& engine) {
        return engine.m_isSelectingItem;
    }

    static bool isSelectingSkill(const GameEngine& engine) {
        return engine.m_isSelectingSkill;
    }

    static bool logOpen(const GameEngine& engine) {
        return engine.m_battleLogOpen;
    }

    static Character* selectedAlly(const GameEngine& engine) {
        return engine.m_selectedAlly;
    }

    static int allyHp(const GameEngine& engine) {
        return engine.m_combatants[1]->getCurrentHp();
    }

    static Character* partyMember(const GameEngine& engine, std::size_t index) {
        return engine.m_party.getMember(index);
    }

    static void addDamageSkill(GameEngine& engine) {
        Skill skill;
        skill.m_name = "Test Strike";
        skill.m_type = SkillType::DAMAGE;
        skill.m_power = 100;
        skill.m_element = Element::KIM;
        engine.m_heroSkills.push_back(skill);
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

    static bool isPaused(const GameEngine& engine) {
        return engine.m_paused;
    }

    static bool isCurrentAlly(const GameEngine& engine) {
        return engine.m_currentCharacter == engine.m_party.getMember(1);
    }

    static void playEnemyTurnAndAdvance(GameEngine& engine) {
        engine.executeAiTurn(engine.m_currentCharacter);
        engine.advanceTurn();
    }

    static const CombatantView& enemyView(const GameEngine& engine) {
        return *engine.m_views[2];
    }
};

int main() {
    const Rectangle fixedHud[] = {
        UiTheme::turnStripHud, UiTheme::roundChipHud, UiTheme::partyCardsHud,
        UiTheme::battleHud, UiTheme::battleLogButtonHud,
        UiTheme::battleLogHud, UiTheme::battleListHud
    };
    for (std::size_t left = 0; left < sizeof(fixedHud) / sizeof(fixedHud[0]); ++left)
        for (std::size_t right = left + 1; right < sizeof(fixedHud) / sizeof(fixedHud[0]); ++right)
            assert(!overlaps(fixedHud[left], fixedHud[right]));

    const float fixedArea = area(UiTheme::turnStripHud) + area(UiTheme::roundChipHud) +
        area(UiTheme::partyCardsHud) + area(UiTheme::battleHud) +
        area(UiTheme::battleLogButtonHud);
    const float activeArea = fixedArea + area(UiTheme::battleLogHud);
    assert(std::abs(fixedArea - 323736.0f) < 0.5f);
    assert(activeArea <= 1920.0f * 1080.0f * 0.22f);

    const BattleSlot anchorSlot{ Vector2{ 640.0f, 520.0f }, 1.5f, false, 0.5f };
    const BattleAnchor anchor = BattleAnchors::forUnit(anchorSlot, 128, 128);
    assert(anchor.feet.x == anchorSlot.m_feetPos.x && anchor.feet.y == anchorSlot.m_feetPos.y);
    assert(std::abs(anchor.spriteBounds.x - 544.0f) < 0.01f);
    assert(std::abs(anchor.spriteBounds.y - 328.0f) < 0.01f);
    assert(std::abs(anchor.spriteBounds.width - 192.0f) < 0.01f);
    Player anchorPlayer("Anchor hero", Stats{ 100, 30, 10, 0, 0, 0, 10 }, Element::KIM);
    const Rectangle playerHp = BattleUI::fieldHpBounds(anchorPlayer, anchor.feet);
    assert(std::abs(playerHp.y - (anchor.feet.y + 10.0f)) < 0.01f);
    assert(playerHp.x + playerHp.width * 0.5f == anchor.feet.x);
    assert(playerHp.width >= 96.0f && playerHp.width <= 180.0f);

    const Rectangle compactList = BattleUI::listPanelBounds(1);
    assert(std::abs(compactList.height * 1.5f - 96.0f) < 0.5f);
    const Rectangle row0 = BattleUI::itemRowBounds(0, 5, 0);
    const Rectangle row1 = BattleUI::itemRowBounds(1, 5, 0);
    assert(row0.width > 0.0f && row0.height > 0.0f);
    assert(!overlaps(row0, row1));

    GameEngine engine;
    GameEngineTestAccess::prepareNormalWave(engine);
    const CombatantView& enemyView = GameEngineTestAccess::enemyView(engine);
    const BattleSlot enemySlot = enemyView.getSlot();
    const Vector2 enemyHead = enemyView.getHeadPos();
    assert(std::abs(enemyHead.x - enemySlot.m_feetPos.x) < 0.01f);
    assert(std::abs(enemyHead.y -
        (enemySlot.m_feetPos.y - 64.0f * enemySlot.m_scale - 10.0f)) < 0.01f);
    const Rectangle cancel = BattleUI::buttonBounds(BattleUIButton::CANCEL_TARGET);

    const Rectangle logButton = BattleUI::logButtonBounds();
    GameEngineTestAccess::click(engine, center(logButton));
    assert(GameEngineTestAccess::logOpen(engine));
    GameEngineTestAccess::click(engine, center(logButton));
    assert(!GameEngineTestAccess::logOpen(engine));

    const Rectangle itemButton = BattleUI::buttonBounds(BattleUIButton::ITEM);
    GameEngineTestAccess::click(engine, center(itemButton));
    assert(GameEngineTestAccess::isSelectingItem(engine));
    GameEngineTestAccess::click(engine, center(BattleUI::partyCardBounds(1)));
    assert(GameEngineTestAccess::selectedAlly(engine) == GameEngineTestAccess::partyMember(engine, 1));
    GameEngineTestAccess::click(engine, center(logButton));
    assert(GameEngineTestAccess::logOpen(engine));
    assert(!GameEngineTestAccess::isSelectingItem(engine));
    GameEngineTestAccess::click(engine, center(logButton));

    GameEngineTestAccess::click(engine, center(BattleUI::buttonBounds(BattleUIButton::SKILL)));
    GameEngineTestAccess::addDamageSkill(engine);
    assert(GameEngineTestAccess::isSelectingSkill(engine));
    GameEngineTestAccess::click(engine, center(cancel));
    assert(!GameEngineTestAccess::isSelectingSkill(engine));
    assert(!GameEngineTestAccess::isSelectingItem(engine));

    GameEngineTestAccess::click(engine, center(itemButton));
    assert(GameEngineTestAccess::isSelectingItem(engine));
    GameEngineTestAccess::click(engine, center(BattleUI::partyCardBounds(1)));
    assert(GameEngineTestAccess::selectedAlly(engine) == GameEngineTestAccess::partyMember(engine, 1));
    const int allyHpBeforeItem = GameEngineTestAccess::allyHp(engine);
    GameEngineTestAccess::click(engine, center(BattleUI::itemRowBounds(0, 1, 0)));
    assert(GameEngineTestAccess::allyHp(engine) > allyHpBeforeItem);
    assert(GameEngineTestAccess::isConfirming(engine));
    GameEngineTestAccess::click(engine, center(BattleUI::buttonBounds(BattleUIButton::UNDO)));
    assert(GameEngineTestAccess::allyHp(engine) == allyHpBeforeItem);
    assert(GameEngineTestAccess::isPlayerTurn(engine));

    assert(!GameEngineTestAccess::isSelectingItem(engine));

    const Rectangle attack = BattleUI::buttonBounds(BattleUIButton::ATTACK);
    GameEngineTestAccess::click(engine, center(attack));
    assert(GameEngineTestAccess::isSelectingTarget(engine));
    GameEngineTestAccess::click(engine, center(cancel));
    assert(!GameEngineTestAccess::isSelectingTarget(engine));

    const int initialHp = GameEngineTestAccess::enemyHp(engine);
    GameEngineTestAccess::click(engine, center(attack));
    const Rectangle enemyBounds = GameEngineTestAccess::enemyView(engine).getDrawBounds();
    GameEngineTestAccess::click(engine, center(enemyBounds));
    assert(GameEngineTestAccess::enemyHp(engine) < initialHp);
    assert(GameEngineTestAccess::isConfirming(engine));

    const Rectangle undo = BattleUI::buttonBounds(BattleUIButton::UNDO);
    GameEngineTestAccess::click(engine, center(undo));
    assert(GameEngineTestAccess::enemyHp(engine) == initialHp);
    assert(GameEngineTestAccess::isPlayerTurn(engine));

    GameEngineTestAccess::click(engine, center(attack));
    GameEngineTestAccess::click(engine, center(enemyBounds));
    assert(GameEngineTestAccess::enemyHp(engine) < initialHp);
    const Rectangle endTurn = BattleUI::buttonBounds(BattleUIButton::END_TURN);
    GameEngineTestAccess::click(engine, center(endTurn));
    assert(GameEngineTestAccess::isEnemyThinking(engine));
    GameEngineTestAccess::playEnemyTurnAndAdvance(engine);
    assert(GameEngineTestAccess::isPlayerTurn(engine));
    assert(GameEngineTestAccess::isCurrentAlly(engine));

    const Rectangle pause = BattleUI::buttonBounds(BattleUIButton::PAUSE);
    const Rectangle commandPanel = BattleUI::controlPanelBounds();
    assert(pause.x >= commandPanel.x && pause.y >= commandPanel.y);
    assert(pause.x + pause.width <= commandPanel.x + commandPanel.width);
    assert(pause.y + pause.height <= commandPanel.y + commandPanel.height);
    GameEngineTestAccess::click(engine, center(pause));
    assert(GameEngineTestAccess::isPaused(engine));

    std::cout << "NORMAL WAVE HUD MOUSE CYCLE PASSED\n";
}
