#include "core/GameEngine.h"
#include "core/AttackCommand.h"
#include "core/UseSkillCommand.h"
#include "core/UseItemCommand.h"
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>

namespace
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;

    // ----- Sprite -----
    constexpr int FRAME_W = 64;
    constexpr int FRAME_H = 64;
    const char* const SPRITE_SHEET_PATH = "assets/images/test_sheet.png";

    // ----- Hero mac dinh (lay tu CSV: races.csv + classes.csv) -----
    const char* const HERO_NAME = "Chien Binh";
    const char* const HERO_RACE = "Human";
    const char* const HERO_CLASS = "Swordman";
    constexpr Element HERO_ELEMENT = Element::KIM;

    // ----- Gameplay -----
    constexpr float AI_TURN_DELAY = 1.2f;
    constexpr int   POTION_HEAL = 40;
    constexpr int   MANA_POTION_RESTORE = 30;
    constexpr int   INITIAL_POTIONS = 3;
    constexpr int   INITIAL_MANA_POTIONS = 2;
    constexpr int   STAGE1_ENEMY_COUNT = 3;
    constexpr float STAGE_CLEAR_HEAL_RATE = 0.3f;   // hoi 30% HP khi qua man

    // ----- He so hieu ung (StatusEffectManager chi dem luot, Core ap dung vao sat thuong) -----
    constexpr float ATK_BOOST_BONUS = 0.25f;
    constexpr float DEF_BOOST_REDUCE = 0.25f;
    constexpr float WEAKNESS_BONUS = 0.25f;
    constexpr float BURN_DAMAGE = 6.0f;
    constexpr float REGEN_AMOUNT = 8.0f;
    constexpr float SLOW_AMOUNT = 2.0f;

    const char* statusName(StatusType type)
    {
        switch (type)
        {
        case StatusType::ATK_BOOST: return "ATK+";
        case StatusType::DEF_BOOST: return "DEF+";
        case StatusType::REGEN:     return "REGEN";
        case StatusType::STUN:      return "STUN";
        case StatusType::BURN:      return "BURN";
        case StatusType::SLOW:      return "SLOW";
        case StatusType::WEAKNESS:  return "WEAK";
        }
        return "?";
    }

    Color elementColor(Element element)
    {
        switch (element)
        {
        case Element::KIM:  return LIGHTGRAY;
        case Element::MOC:  return Color{ 140, 230, 140, 255 };
        case Element::THUY: return Color{ 130, 190, 255, 255 };
        case Element::HOA:  return Color{ 255, 160, 110, 255 };
        case Element::THO:  return Color{ 210, 170, 110, 255 };
        }
        return WHITE;
    }

    const char* elementName(Element element)
    {
        switch (element)
        {
        case Element::KIM:  return "KIM";
        case Element::MOC:  return "MOC";
        case Element::THUY: return "THUY";
        case Element::HOA:  return "HOA";
        case Element::THO:  return "THO";
        }
        return "?";
    }
}

// =====================================================================
//  VONG DOI
// =====================================================================

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
    SetExitKey(KEY_NULL);   // ESC do game tu xu ly (Undo / ve Menu), khong thoat thang

    try {
        m_gameData.loadAll(resolvePath("data"));
        m_dataLoaded = true;
        m_battleLog = "Data CSV nap thanh cong!";
    }
    catch (const std::exception& e) {
        m_dataLoaded = false;
        m_battleLog = std::string("Data load notice: ") + e.what();
        std::cerr << m_battleLog << std::endl;
    }

    m_combatSubject.addObserver(this);
    m_battleMap.setStage(StageId::STAGE_1);

    m_stateMachine.changeState(GameStateMachine::State::MENU);
    m_isRunning = true;
    m_isInitialized = true;
}

void GameEngine::shutdown()
{
    m_combatSubject.clearObservers();
    // Giai phong Texture (sprite) TRUOC khi dong cua so
    m_sprites.clear();
    m_combatants.clear();

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
    // Chi hoan tac duoc khi nguoi choi chua ket thuc luot
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE) ||
        m_battlePhase != BattlePhase::PLAYER_CONFIRM)
        return false;

    const bool result = m_commandStack.cancelLastAction();
    if (result) {
        for (auto& c : m_combatants) syncHealth(c.get());
        m_battlePhase = BattlePhase::PLAYER_TURN;
        m_battleLog = "Da hoan tac (Undo) hanh dong vua roi! Chon lenh khac.";
    }
    return result;
}

// =====================================================================
//  THIET LAP TRAN DAU
// =====================================================================

std::string GameEngine::resolvePath(const std::string& relativePath) const
{
    // Cho phep chay tu thu muc goc repo (./build/game.exe) hoac ngay trong thu muc build
    const std::string appDir = GetApplicationDirectory();
    const std::string candidates[] = {
        relativePath,
        appDir + relativePath,
        appDir + "../" + relativePath
    };
    for (const auto& path : candidates)
    {
        if (FileExists(path.c_str()) || DirectoryExists(path.c_str()))
            return path;
    }
    return relativePath;
}

int GameEngine::finalStage() const
{
    return m_dataLoaded ? 2 : 1;
}

void GameEngine::addCombatant(std::shared_ptr<Character> character)
{
    if (!character) return;

    auto sprite = std::make_unique<SpriteAnimation>();
    sprite->load(resolvePath(SPRITE_SHEET_PATH), FRAME_W, FRAME_H);   // thieu asset -> ve placeholder
    sprite->play(AnimState::IDLE);

    m_lastHp[character.get()] = character->getCurrentHp();
    m_turnManager.addCharacter(character.get());
    m_combatants.push_back(std::move(character));
    m_sprites.push_back(std::move(sprite));
}

void GameEngine::createHero()
{
    std::shared_ptr<Player> hero;

    if (m_dataLoaded)
    {
        try {
            hero = CharacterFactory::createHero(m_gameData, HERO_NAME, HERO_RACE, HERO_CLASS, HERO_ELEMENT);
        }
        catch (const std::exception& e) {
            std::cerr << "Hero data error: " << e.what() << std::endl;
        }
    }

    if (!hero)
    {
        // Du phong khi chua co CSV
        hero = std::make_shared<Player>(HERO_NAME, Stats{ 120, 50, 25, 20, 5, 8, 15 }, HERO_ELEMENT);
        hero->addSkill(Skill{ "S01", "Chem Manh", SkillOwnerType::CLASS, "Warrior", SkillType::DAMAGE, 130, 5, EffectType::NONE, 0, Element::KIM, false });
        hero->addSkill(Skill{ "S02", "Hoa Cau", SkillOwnerType::CLASS, "Mage", SkillType::DAMAGE, 150, 12, EffectType::BURN, 2, Element::HOA, true });
        hero->addSkill(Skill{ "S03", "Hoi Mau", SkillOwnerType::CLASS, "Priest", SkillType::HEAL, 30, 10, EffectType::REGEN, 2, Element::THUY, true });
    }

    m_party.addMember(hero);
    addCombatant(hero);
}

void GameEngine::spawnEnemies(int stage)
{
    if (m_dataLoaded)
    {
        std::vector<MonsterData> monsters = m_gameData.getMonstersByStage(stage);
        if (stage == 1)
        {
            std::shuffle(monsters.begin(), monsters.end(), m_rng);
            if (monsters.size() > static_cast<size_t>(STAGE1_ENEMY_COUNT))
                monsters.resize(STAGE1_ENEMY_COUNT);
        }
        for (const auto& m : monsters)
        {
            if (m.m_isBoss)
                addCombatant(std::make_unique<Boss>(m.m_name, m.m_stats, m.m_element));
            else
                addCombatant(std::make_unique<Enemy>(m.m_name, m.m_stats, m.m_element));
        }
    }
    else
    {
        addCombatant(std::make_unique<Enemy>("Goblin", Stats{ 60, 0, 28, 0, 5, 2, 10 }, Element::MOC));
        addCombatant(std::make_unique<Boss>("Ma Vuong", Stats{ 200, 40, 30, 25, 12, 10, 8 }, Element::HOA));
    }
}

void GameEngine::loadStage(int stage)
{
    m_stage = stage;

    // Giu lai hero (m_combatants[0]), bo het quai cu
    if (m_combatants.size() > 1)
    {
        m_combatants.resize(1);
        m_sprites.resize(1);
    }

    m_turnManager.clear();
    m_commandStack.clear();
    m_statusEffects.clear();
    m_lastHp.clear();
    m_floatingTexts.clear();
    m_isSelectingSkill = false;
    m_aiTurnTimer = 0.0f;

    if (!m_combatants.empty())
    {
        Character* h = m_combatants[0].get();
        if (stage > 1)
            h->heal(static_cast<int>(static_cast<float>(h->getStats().m_hp) * STAGE_CLEAR_HEAL_RATE));
        m_turnManager.addCharacter(h);
        m_lastHp[h] = h->getCurrentHp();
    }

    spawnEnemies(stage);

    m_selectedTarget = (m_combatants.size() > 1) ? m_combatants[1].get() : nullptr;
    m_battleMap.setStage(stage == 1 ? StageId::STAGE_1 : StageId::STAGE_2);

    m_currentCharacter = nullptr;   // update() se goi advanceTurn() de lay luot dau tien
    m_battlePhase = BattlePhase::AI_THINKING;
    m_battleLog = "MAN " + std::to_string(stage) + " bat dau!";
}

void GameEngine::startNewGame()
{
    m_combatants.clear();
    m_sprites.clear();
    m_floatingTexts.clear();

    m_party.clear();
    for (int i = 0; i < INITIAL_POTIONS; ++i)
        m_party.getInventory().addItem(std::make_shared<Potion>(POTION_HEAL));
    for (int i = 0; i < INITIAL_MANA_POTIONS; ++i)
        m_party.getInventory().addItem(std::make_shared<ManaPotion>(MANA_POTION_RESTORE));

    createHero();
    loadStage(1);
    advanceTurn();

    m_battleLog = "Tran dau bat dau! Den luot: " + (m_currentCharacter ? m_currentCharacter->getName() : std::string("None"));
}

// =====================================================================
//  SU KIEN / INPUT
// =====================================================================

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
    else if (IsKeyPressed(KEY_Z) && m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
    {
        cancelLastAction();
    }

    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:
        if (IsKeyPressed(KEY_ENTER))
        {
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
            startNewGame();
        }
        break;

    case GameStateMachine::State::IN_BATTLE:
        if (m_battlePhase == BattlePhase::PLAYER_TURN || m_battlePhase == BattlePhase::PLAYER_CONFIRM)
            processPlayerInput();
        break;

    case GameStateMachine::State::VICTORY:
    case GameStateMachine::State::GAME_OVER:
        if (IsKeyPressed(KEY_ENTER))
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        break;
    }
}

void GameEngine::ensureValidTarget()
{
    if (m_selectedTarget && m_selectedTarget->isAlive() && !isHero(m_selectedTarget))
        return;

    m_selectedTarget = nullptr;
    for (size_t i = 1; i < m_combatants.size(); ++i)
    {
        if (m_combatants[i]->isAlive()) {
            m_selectedTarget = m_combatants[i].get();
            return;
        }
    }
}

void GameEngine::cycleTarget()
{
    const size_t count = m_combatants.size();
    if (count < 2) return;

    size_t start = 1;
    const int current = indexOf(m_selectedTarget);
    if (current >= 1) start = static_cast<size_t>(current);

    for (size_t step = 1; step <= count - 1; ++step)
    {
        const size_t idx = 1 + ((start - 1 + step) % (count - 1));
        if (m_combatants[idx]->isAlive()) {
            m_selectedTarget = m_combatants[idx].get();
            return;
        }
    }
}

void GameEngine::processPlayerInput()
{
    if (!m_currentCharacter || !isHero(m_currentCharacter)) return;

    // Da hanh dong -> cho xac nhan (ENTER) hoac Undo (ESC/Z xu ly o processEvents)
    if (m_battlePhase == BattlePhase::PLAYER_CONFIRM)
    {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
            confirmTurn();
        return;
    }

    ensureValidTarget();

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
        cycleTarget();

    if (IsKeyPressed(KEY_S))
        m_isSelectingSkill = !m_isSelectingSkill;

    if (m_isSelectingSkill)
    {
        const KeyboardKey keys[] = { KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR };
        for (size_t i = 0; i < heroSkills().size() && i < 4; ++i)
        {
            if (IsKeyPressed(keys[i]))
                m_selectedSkillIndex = static_cast<int>(i);
        }

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
        {
            if (castSkill(static_cast<size_t>(m_selectedSkillIndex)))
            {
                m_isSelectingSkill = false;
                confirmTurn();   // skill khong hoan tac duoc (khong co API hoan MP) -> ket thuc luot ngay
            }
        }
        return;
    }

    if (IsKeyPressed(KEY_A))
        doBasicAttack();
    else if (IsKeyPressed(KEY_I))
        doUseItem(ItemKind::POTION);
    else if (IsKeyPressed(KEY_M))
        doUseItem(ItemKind::MANA_POTION);
}

void GameEngine::doBasicAttack()
{
    ensureValidTarget();
    if (!m_selectedTarget) return;

    Character* attacker = hero();
    Character* target = m_selectedTarget;
    const int hpBefore = target->getCurrentHp();

    // AttackCommand co undo(): hoan lai HP cho muc tieu
    queuePlayerCommand(std::make_unique<AttackCommand>(attacker, target, damageMultiplier(attacker, target)));

    const int dealt = hpBefore - target->getCurrentHp();
    playAnimation(attacker, AnimState::ATTACK);
    syncHealth(target);

    m_battleLog = attacker->getName() + " tan cong " + target->getName() + " gay " + std::to_string(dealt) + " sat thuong!"
                + "  [ENTER] ket thuc luot, [ESC/Z] hoan tac";
    m_battlePhase = BattlePhase::PLAYER_CONFIRM;
}

void GameEngine::doUseItem(ItemKind kind)
{
    Character* h = hero();
    if (!h) return;

    Inventory<ItemPtr>& bag = m_party.getInventory();
    const int index = bag.findIndex([kind](const ItemPtr& item) { return item->getKind() == kind; });
    if (index < 0) {
        m_battleLog = (kind == ItemKind::POTION) ? "Het Potion!" : "Het Mana Potion!";
        return;
    }

    const ItemPtr item = bag.getItem(static_cast<std::size_t>(index));
    if (!item->canUse(*h)) {
        m_battleLog = item->getName() + " khong can dung luc nay (da day).";
        return;
    }

    auto amount = std::make_shared<int>(0);

    auto executeAction = [this, h, item, amount]() {
        *amount = item->use(*h);                  // da hinh: Potion hoi HP, ManaPotion hoi MP
        m_party.getInventory().removeItem(item);
        syncHealth(h);
    };
    auto undoAction = [this, h, item, amount]() {
        item->revert(*h, *amount);                // tra lai dung luong da hoi
        m_party.getInventory().addItem(item);
    };

    queuePlayerCommand(std::make_unique<UseItemCommand>(executeAction, undoAction));

    const char* unit = (kind == ItemKind::POTION) ? " HP" : " MP";
    m_battleLog = h->getName() + " dung " + item->getName() + " hoi " + std::to_string(*amount) + unit
                + "!  [ENTER] ket thuc luot, [ESC/Z] hoan tac";
    m_battlePhase = BattlePhase::PLAYER_CONFIRM;
}

bool GameEngine::castSkill(size_t skillIndex)
{
    if (skillIndex >= heroSkills().size()) return false;

    Character* h = hero();
    const Skill skill = heroSkills()[skillIndex];   // ban sao: thao tac an toan trong lambda

    if (h->getCurrentMp() < skill.m_mpCost) {
        m_battleLog = "Khong du MP de dung " + skill.m_name + "!";
        return false;
    }

    const bool offensive = (skill.m_type == SkillType::DAMAGE) ||
                           (skill.m_type == SkillType::DEBUFF && skill.m_power > 0);
    if (offensive)
    {
        ensureValidTarget();
        if (!m_selectedTarget) return false;
    }
    Character* target = offensive ? m_selectedTarget : h;

    auto executeAction = [this, h, target, skill, offensive]() {
        const int hpBefore = target->getCurrentHp();

        if (offensive)
        {
            // CSV: power la % cua ATK. Player::executeSkill tinh (ATK + power) -> quy doi power ve dang cong them.
            const float atk = static_cast<float>(skill.m_isMagic ? h->getStats().m_ma : h->getStats().m_a);
            const float total = atk * static_cast<float>(skill.m_power) / 100.0f * damageMultiplier(h, target);
            Skill cast = skill;
            cast.m_power = static_cast<int>(total) - static_cast<int>(atk);
            h->executeSkill(&cast, target);
            playAnimation(h, AnimState::ATTACK);
        }
        else
        {
            spendMpWithoutDamage(skill);
            if (skill.m_type == SkillType::HEAL)
                h->heal(skill.m_power);
        }

        syncHealth(h);
        syncHealth(target);
        applySkillEffect(skill, target);

        if (offensive)
            m_battleLog = h->getName() + " dung " + skill.m_name + " len " + target->getName()
                        + " gay " + std::to_string(hpBefore - target->getCurrentHp()) + " sat thuong!";
        else if (skill.m_type == SkillType::HEAL)
            m_battleLog = h->getName() + " dung " + skill.m_name + " hoi " + std::to_string(target->getCurrentHp() - hpBefore) + " HP!";
        else
            m_battleLog = h->getName() + " dung " + skill.m_name + "!";
    };

    // Skill khong the hoan tac (khong hoan lai duoc MP) -> undoAction rong
    return queuePlayerCommand(std::make_unique<UseSkillCommand>(executeAction, []() {}));
}

void GameEngine::confirmTurn()
{
    m_commandStack.clear();    // chot hanh dong: het kha nang Undo
    m_isSelectingSkill = false;
    advanceTurn();
}

// =====================================================================
//  LUOT CHOI & AI
// =====================================================================

bool GameEngine::beginTurn(Character* c)
{
    // Quai/hero bi STUN -> mat luot. Phai kiem tra TRUOC updateEffects vi updateEffects
    // go bo hieu ung het han (STUN 1 luot) va tu tat co stun ngay trong cung lan goi.
    StatusEffectManager& effects = m_statusEffects[c];
    const bool wasStunned = effects.hasEffect(StatusType::STUN);

    const int hpBefore = c->getCurrentHp();
    effects.updateEffects(c);          // BURN tru mau, REGEN hoi mau, tru so luot
    c->setStunned(false);              // Core tu xu ly mat luot, khong de TurnManager nuot am tham
    syncHealth(c);

    if (!c->isAlive()) {
        m_battleLog = c->getName() + " guc nga do hieu ung (BURN)!";
        return false;
    }

    if (wasStunned) {
        m_battleLog = c->getName() + " bi choang, mat luot!";
        return false;
    }

    if (c->getCurrentHp() < hpBefore)
        m_battleLog = c->getName() + " chiu sat thuong tu hieu ung!";
    return true;
}

void GameEngine::advanceTurn()
{
    m_aiTurnTimer = 0.0f;
    m_isSelectingSkill = false;

    // Gioi han vong lap: phong truong hop moi nhan vat deu mat luot lien tuc
    for (int guard = 0; guard < 64; ++guard)
    {
        checkBattleCondition();
        if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
            return;

        m_currentCharacter = m_turnManager.getNextTurn();
        if (!m_currentCharacter)
            continue;

        if (!beginTurn(m_currentCharacter))
            continue;   // mat luot / chet -> sang nguoi ke tiep

        m_battlePhase = isHero(m_currentCharacter) ? BattlePhase::PLAYER_TURN : BattlePhase::AI_THINKING;
        return;
    }
    m_battlePhase = BattlePhase::PLAYER_TURN;
}

void GameEngine::executeAiTurn(Character* aiChar)
{
    Character* h = hero();
    if (!aiChar || !aiChar->isAlive() || !h || !h->isAlive()) return;

    // Don thuong dung executeSkill() cua tung lop (Enemy/Boss) -> dung da hinh:
    // Boss tu nhan he so sat thuong 125% trong Boss::executeSkill.
    const bool useMagic = aiChar->getStats().m_ma > aiChar->getStats().m_a;
    const float atk = static_cast<float>(useMagic ? aiChar->getStats().m_ma : aiChar->getStats().m_a);

    Skill basic;
    basic.m_id = "AI_BASIC";
    basic.m_name = "Don thuong";
    basic.m_type = SkillType::DAMAGE;
    basic.m_mpCost = 0;
    basic.m_element = aiChar->getElement();
    basic.m_isMagic = useMagic;
    basic.m_power = static_cast<int>(atk * damageMultiplier(aiChar, h)) - static_cast<int>(atk);

    const int hpBefore = h->getCurrentHp();
    aiChar->executeSkill(&basic, h);
    const int dealt = hpBefore - h->getCurrentHp();

    playAnimation(aiChar, AnimState::ATTACK);
    syncHealth(h);

    m_battleLog = aiChar->getName() + " tan cong " + h->getName() + " gay " + std::to_string(dealt) + " sat thuong!";
}

void GameEngine::update(float deltaTime)
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) return;

    // Cap nhat sprite + chu bay
    for (auto& sprite : m_sprites) sprite->update(deltaTime);
    for (auto& t : m_floatingTexts) {
        t.m_pos.y -= 40.0f * deltaTime;
        t.m_life -= deltaTime;
    }
    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                       [](const FloatingText& t) { return t.m_life <= 0.0f; }),
        m_floatingTexts.end());

    // Dang cho nguoi choi xac nhan/undo: khong xet ket thuc tran (muc tieu co the hoi sinh khi undo)
    if (m_battlePhase != BattlePhase::PLAYER_CONFIRM)
        checkBattleCondition();
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) return;

    if (!m_currentCharacter)
    {
        advanceTurn();
        return;
    }

    if (m_battlePhase == BattlePhase::AI_THINKING)
    {
        if (!m_currentCharacter->isAlive() || isHero(m_currentCharacter))
        {
            advanceTurn();
            return;
        }

        m_aiTurnTimer += deltaTime;
        if (m_aiTurnTimer >= AI_TURN_DELAY)
        {
            executeAiTurn(m_currentCharacter);
            advanceTurn();
        }
    }
}

void GameEngine::checkBattleCondition()
{
    if (m_combatants.empty()) return;

    if (!m_combatants[0]->isAlive())
    {
        m_stateMachine.changeState(GameStateMachine::State::GAME_OVER);
        return;
    }

    for (size_t i = 1; i < m_combatants.size(); ++i)
    {
        if (m_combatants[i]->isAlive())
            return;
    }

    // Tat ca quai da chet
    if (m_stage < finalStage())
        loadStage(m_stage + 1);
    else
        m_stateMachine.changeState(GameStateMachine::State::VICTORY);
}

// =====================================================================
//  CHIEN DAU
// =====================================================================

float GameEngine::damageMultiplier(const Character* attacker, const Character* target) const
{
    float multiplier = 1.0f;

    auto a = m_statusEffects.find(attacker);
    if (a != m_statusEffects.end() && a->second.hasEffect(StatusType::ATK_BOOST))
        multiplier *= 1.0f + ATK_BOOST_BONUS;

    auto t = m_statusEffects.find(target);
    if (t != m_statusEffects.end())
    {
        if (t->second.hasEffect(StatusType::DEF_BOOST)) multiplier *= 1.0f - DEF_BOOST_REDUCE;
        if (t->second.hasEffect(StatusType::WEAKNESS))  multiplier *= 1.0f + WEAKNESS_BONUS;
    }
    return multiplier;
}

void GameEngine::spendMpWithoutDamage(const Skill& skill)
{
    if (Character* h = hero())
        h->consumeMp(skill.m_mpCost);
}

void GameEngine::applySkillEffect(const Skill& skill, Character* target)
{
    if (skill.m_effect == EffectType::NONE || skill.m_effectTurns <= 0) return;
    if (!target || !target->isAlive()) return;

    StatusType type = StatusType::ATK_BOOST;
    float value = 0.0f;
    switch (skill.m_effect)
    {
    case EffectType::ATK_BOOST: type = StatusType::ATK_BOOST; value = ATK_BOOST_BONUS; break;
    case EffectType::DEF_BOOST: type = StatusType::DEF_BOOST; value = DEF_BOOST_REDUCE; break;
    case EffectType::WEAKNESS:  type = StatusType::WEAKNESS;  value = WEAKNESS_BONUS; break;
    case EffectType::REGEN:     type = StatusType::REGEN;     value = REGEN_AMOUNT; break;
    case EffectType::BURN:      type = StatusType::BURN;      value = BURN_DAMAGE; break;
    case EffectType::SLOW:      type = StatusType::SLOW;      value = SLOW_AMOUNT; break;
    case EffectType::STUN:      type = StatusType::STUN;      value = 0.0f; break;
    case EffectType::NONE:      return;
    }
    m_statusEffects[target].addEffect(type, skill.m_effectTurns, value);
}

void GameEngine::playAnimation(Character* character, AnimState state)
{
    const int idx = indexOf(character);
    if (idx >= 0 && static_cast<size_t>(idx) < m_sprites.size())
        m_sprites[static_cast<size_t>(idx)]->play(state, true);
}

// =====================================================================
//  OBSERVER
// =====================================================================

void GameEngine::syncHealth(Character* character)
{
    if (!character) return;
    auto it = m_lastHp.find(character);
    if (it == m_lastHp.end()) {
        m_lastHp[character] = character->getCurrentHp();
        return;
    }

    const int oldHp = it->second;
    const int newHp = character->getCurrentHp();
    if (oldHp == newHp) return;

    m_combatSubject.notifyHealthChanged(character, newHp);   // onHealthChanged cap nhat m_lastHp
    if (oldHp > 0 && newHp <= 0)
        m_combatSubject.notifyCharacterDied(character);
}

void GameEngine::onHealthChanged(Character* character, int newHealth)
{
    auto it = m_lastHp.find(character);
    const int oldHp = (it != m_lastHp.end()) ? it->second : newHealth;
    m_lastHp[character] = newHealth;

    const int delta = newHealth - oldHp;
    if (delta < 0)
    {
        spawnFloatingText(character, std::to_string(delta), RED);
        if (newHealth > 0)
            playAnimation(character, AnimState::TAKE_DAMAGE);
    }
    else if (delta > 0)
    {
        spawnFloatingText(character, "+" + std::to_string(delta), GREEN);
    }
}

void GameEngine::onCharacterDied(Character* character)
{
    if (character)
        m_battleLog += "  >> " + character->getName() + " guc nga!";
}

void GameEngine::spawnFloatingText(const Character* character, const std::string& text, Color color)
{
    const int idx = indexOf(character);
    if (idx < 0) return;

    const BattleSlot slot = slotOf(idx);
    FloatingText t;
    t.m_pos = BattleMap::getHeadPos(slot, FRAME_H);
    t.m_pos.x += static_cast<float>(GetRandomValue(-20, 20));
    t.m_text = text;
    t.m_color = color;
    t.m_life = 1.0f;
    m_floatingTexts.push_back(t);
}

// =====================================================================
//  TIEN ICH
// =====================================================================

int GameEngine::indexOf(const Character* character) const
{
    for (size_t i = 0; i < m_combatants.size(); ++i)
        if (m_combatants[i].get() == character) return static_cast<int>(i);
    return -1;
}

Character* GameEngine::hero() const
{
    return m_combatants.empty() ? nullptr : m_combatants[0].get();
}

const std::vector<Skill>& GameEngine::heroSkills() const
{
    static const std::vector<Skill> none;
    const Player* leader = m_party.getLeader();
    return leader ? leader->getSkills() : none;
}

bool GameEngine::isHero(const Character* character) const
{
    return character != nullptr && character == hero();
}

BattleSlot GameEngine::slotOf(int index) const
{
    if (index <= 0)
        return m_battleMap.getPlayerSlot(0, 1);

    const bool isBossUnit = dynamic_cast<const Boss*>(m_combatants[static_cast<size_t>(index)].get()) != nullptr;
    return m_battleMap.getEnemySlot(index - 1, static_cast<int>(m_combatants.size()) - 1, isBossUnit ? 1.3f : 1.0f);
}

// =====================================================================
//  RENDER
// =====================================================================

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
        DrawText(m_dataLoaded ? "Data: CSV OK" : "Data: CSV loi -> dung du lieu du phong", 440, 420, 18,
                 m_dataLoaded ? DARKGREEN : MAROON);
        break;

    case GameStateMachine::State::IN_BATTLE:
        m_battleMap.draw();
        renderCombatants();
        renderBattleUI();
        renderFloatingTexts();
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

void GameEngine::renderCombatants()
{
    for (size_t i = 0; i < m_combatants.size(); ++i)
    {
        const Character* c = m_combatants[i].get();
        if (!c->isAlive()) continue;

        const BattleSlot slot = slotOf(static_cast<int>(i));
        m_battleMap.drawShadow(slot, static_cast<float>(FRAME_W));

        const Vector2 topLeft = BattleMap::toTopLeft(slot, FRAME_W, FRAME_H);
        const Color tint = (i == 0) ? WHITE : elementColor(c->getElement());
        m_sprites[i]->drawFrame(topLeft, slot.m_flipX, slot.m_scale, tint);
    }
}

void GameEngine::renderFloatingTexts()
{
    for (const auto& t : m_floatingTexts)
    {
        const unsigned char alpha = static_cast<unsigned char>(255.0f * std::min(1.0f, t.m_life));
        Color color = t.m_color;
        color.a = alpha;
        DrawText(t.m_text.c_str(), static_cast<int>(t.m_pos.x), static_cast<int>(t.m_pos.y), 28, color);
    }
}

void GameEngine::renderBattleUI()
{
    // ----- HUD tung nhan vat (neo tren dau sprite) -----
    for (size_t i = 0; i < m_combatants.size(); ++i)
    {
        const Character* c = m_combatants[i].get();
        if (!c->isAlive()) continue;

        const BattleSlot slot = slotOf(static_cast<int>(i));
        const Vector2 head = BattleMap::getHeadPos(slot, FRAME_H);
        const int x = static_cast<int>(head.x) - 60;
        const int y = static_cast<int>(head.y) - 70;

        if (c == m_selectedTarget)
            DrawRectangleLines(x - 5, y - 5, 130, 82, GOLD);

        const Color titleColor = (c == m_currentCharacter) ? YELLOW : WHITE;
        DrawText(TextFormat("%s [%s]", c->getName().c_str(), elementName(c->getElement())), x, y, 16, titleColor);

        const int maxHp = std::max(1, c->getStats().m_hp);
        const float hpRatio = std::clamp(static_cast<float>(c->getCurrentHp()) / static_cast<float>(maxHp), 0.0f, 1.0f);
        DrawRectangle(x, y + 22, 120, 12, RED);
        DrawRectangle(x, y + 22, static_cast<int>(120 * hpRatio), 12, GREEN);
        DrawRectangleLines(x, y + 22, 120, 12, BLACK);
        DrawText(TextFormat("%d/%d", c->getCurrentHp(), maxHp), x + 25, y + 23, 10, WHITE);

        if (c->getStats().m_mp > 0) {
            const float mpRatio = std::clamp(static_cast<float>(c->getCurrentMp()) / static_cast<float>(c->getStats().m_mp), 0.0f, 1.0f);
            DrawRectangle(x, y + 38, 120, 8, DARKGRAY);
            DrawRectangle(x, y + 38, static_cast<int>(120 * mpRatio), 8, BLUE);
            DrawRectangleLines(x, y + 38, 120, 8, BLACK);
        }

        // Hieu ung dang co (StatusEffectManager)
        auto fx = m_statusEffects.find(c);
        if (fx != m_statusEffects.end())
        {
            int ex = x;
            for (const StatusEffect& effect : fx->second.getEffects())
            {
                const char* label = TextFormat("%s(%d)", statusName(effect.type), effect.durationTurns);
                DrawText(label, ex, y + 50, 12, ORANGE);
                ex += MeasureText(label, 12) + 6;
            }
        }
    }

    // ----- Bang dieu khien -----
    DrawRectangle(20, 560, 1240, 140, Fade(BLACK, 0.8f));
    DrawRectangleLines(20, 560, 1240, 140, RAYWHITE);

    DrawText(TextFormat("MAN %d", m_stage), 1160, 575, 20, SKYBLUE);
    DrawText("DEN LUOT:", 40, 575, 20, YELLOW);
    if (m_currentCharacter)
    {
        if (isHero(m_currentCharacter)) {
            const char* suffix = (m_battlePhase == BattlePhase::PLAYER_CONFIRM) ? " (Cho xac nhan)" : "";
            DrawText(TextFormat("%s%s", m_currentCharacter->getName().c_str(), suffix), 160, 575, 20, GREEN);
        } else {
            DrawText(TextFormat("%s (Dang suy nghi...)", m_currentCharacter->getName().c_str()), 160, 575, 20, RED);
        }
    }

    if (m_battlePhase == BattlePhase::PLAYER_CONFIRM)
        DrawText("[ENTER] Ket thuc luot  |  [ESC/Z] Hoan tac (Undo)", 40, 610, 18, LIGHTGRAY);
    else
    {
        const auto& bag = m_party.getInventory();
        const int potions = static_cast<int>(bag.countIf([](const ItemPtr& i) { return i->getKind() == ItemKind::POTION; }));
        const int manas = static_cast<int>(bag.countIf([](const ItemPtr& i) { return i->getKind() == ItemKind::MANA_POTION; }));
        DrawText(TextFormat("[A] Tan Cong | [S] Ky Nang | [I] Potion x%d | [M] Mana x%d | [TAB] Doi Muc Tieu | [ESC] Menu",
                            potions, manas), 40, 610, 18, LIGHTGRAY);
    }
    DrawText(TextFormat("BATTLE LOG: %s", m_battleLog.c_str()), 40, 650, 20, ORANGE);

    // ----- Menu ky nang -----
    if (m_isSelectingSkill)
    {
        const int boxH = 90 + static_cast<int>(heroSkills().size()) * 35;
        DrawRectangle(360, 290, 560, boxH, Fade(DARKBLUE, 0.95f));
        DrawRectangleLines(360, 290, 560, boxH, SKYBLUE);
        DrawText("KY NANG (1-4 chon, SPACE de dung, ESC dong)", 380, 305, 16, YELLOW);

        const int mp = hero() ? hero()->getCurrentMp() : 0;
        for (size_t i = 0; i < heroSkills().size(); ++i)
        {
            const Skill& s = heroSkills()[i];
            Color txtColor = WHITE;
            if (mp < s.m_mpCost) txtColor = GRAY;
            else if (m_selectedSkillIndex == static_cast<int>(i)) txtColor = GREEN;

            const char* suffix = (s.m_type == SkillType::HEAL) ? " HP" : "%";
            DrawText(TextFormat("[%d] %s  (MP %d, %d%s)", static_cast<int>(i) + 1, s.m_name.c_str(), s.m_mpCost, s.m_power, suffix),
                     390, 340 + static_cast<int>(i) * 35, 18, txtColor);
        }
    }
}