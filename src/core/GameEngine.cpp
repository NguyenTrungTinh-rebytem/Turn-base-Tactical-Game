#include "core/GameEngine.h"
#include <string>
#include <unordered_map>
#include "core/AttackCommand.h"
#include "core/UseSkillCommand.h"
#include "core/UseItemCommand.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <exception>
#include <iostream>

namespace
{   
    constexpr float BOSS_SCALE_MUL = 3.2f;   // boss to gap may lan quai thuong
    constexpr int   BOSS_FRAME     = 192;    // sheet boss: moi frame 192x192
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;

    // ----- Sprite -----
    constexpr int FRAME_W = 64;
    constexpr int FRAME_H = 64;
    const char* const SPRITE_SHEET_PATH = "assets/images/test_sheet.png";
    
    // Sprite sheet rieng cho tung quai (theo id trong monsters.csv). Id la -> dung SPRITE_SHEET_PATH.
    std::string monsterSpritePath(const std::string& monsterId)
    {
    static const std::unordered_map<std::string, std::string> SPRITES = {
        { "M_GOBLIN_KIM", "assets/images/goblin_sheet.png" },
        { "M_WOLF_MOC",   "assets/images/wolf_sheet.png"   },
        { "M_SLIME_THUY", "assets/images/slime_sheet.png"  },
        { "M_IMP_HOA",    "assets/images/imp_sheet.png"    },
        { "M_GOLEM_THO",  "assets/images/golem_sheet.png"  },
        { "B_DRAGON_HOA", "assets/images/boss_dragon_sheet.png" },
    };
    const auto it = SPRITES.find(monsterId);
    return it != SPRITES.end() ? it->second : std::string(SPRITE_SHEET_PATH);
    }
    // ----- Hero mac dinh (lay tu CSV: races.csv + classes.csv) -----
    const char* const HERO_NAME = "Chien Binh";
    const char* const HERO_RACE = "Human";
    const char* const HERO_CLASS = "Swordman";
    constexpr Element HERO_ELEMENT = Element::KIM;

    // ----- Gameplay -----
    constexpr float AI_TURN_DELAY = 1.2f;
    constexpr int   POTION_HEAL = 40;
    constexpr int   INITIAL_POTIONS = 3;
    constexpr int   STAGE1_ENEMY_COUNT = 3;
    constexpr float STAGE_CLEAR_HEAL_RATE = 0.3f;   // hoi 30% HP khi qua man
    const char* const POTION_NAME = "Potion";
    const char* skillTypeText(SkillType t) {
    switch (t) {
        case SkillType::DAMAGE: return "Sat thuong";
        case SkillType::HEAL:   return "Hoi mau";
        case SkillType::BUFF:   return "Buff";
        case SkillType::DEBUFF: return "Debuff";
    }
    return "";
}

std::string effectText(EffectType e, int turns) {
    const char* name = "";
    switch (e) {
        case EffectType::ATK_BOOST: name = "Tang cong";  break;
        case EffectType::DEF_BOOST: name = "Tang thu";   break;
        case EffectType::REGEN:     name = "Hoi mau";    break;
        case EffectType::STUN:      name = "Choang";     break;
        case EffectType::BURN:      name = "Chay";       break;
        case EffectType::SLOW:      name = "Lam cham";   break;
        case EffectType::WEAKNESS:  name = "Suy yeu";    break;
        case EffectType::NONE:      return "";
    }
    return std::string(name) + " " + std::to_string(turns) + " luot";
}

int deadlineTurnsForStage(int /*stage*/) {
    return -1;   // < 0 = khong gioi han; khi co dong ho cong thi sua o day
}

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
    m_startMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));   // thieu anh -> nen thu tuc
    m_startMenu.setStatusText(m_dataLoaded ? "Data: CSV OK" : "Data: CSV loi -> dung du lieu du phong", m_dataLoaded);
    m_heroConfig = { HERO_NAME, HERO_RACE, HERO_CLASS, HERO_ELEMENT };
    m_heroCreate.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_waveSelect.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_prepMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_heroCreate.loadPreviewSprite(resolvePath(SPRITE_SHEET_PATH), FRAME_W, FRAME_H);
    setupHeroCreateScreen();
    m_combatSubject.addObserver(this);
    m_battleMap.setStage(m_stage == 1 ? StageId::STAGE_1 : StageId::STAGE_2,
    resolvePath(m_stage == 1 ? "assets/images/bg_wave1.png"
                           : "assets/images/bg_wave2.png"));
    m_stateMachine.changeState(GameStateMachine::State::MENU);
    m_isRunning = true;
    m_isInitialized = true;
}

void GameEngine::shutdown()
{
    m_combatSubject.clearObservers();
    // Giai phong Texture (sprite) TRUOC khi dong cua so
    m_views.clear();
    m_ui.unloadAll();
    m_turnOrderBar.reset();
    m_startMenu.unloadAll();
    m_heroCreate.unloadAll();
    m_waveSelect.unloadAll();
    m_prepMenu.unloadAll();
    m_battleMap.unloadAll();
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

Skill GameEngine::toBattleSkill(const SkillData& data, Element heroElement)
{
    // TODO(Data): khi GameData co buildSkill(id) thi thay ham nay.
    // Nguyen to: lay tu data/skill_elements.csv (GameData::setSkillElement de sua).
    // Skill chua duoc gan nguyen to -> Core suy ra tam tu hieu ung (du phong nhu cu).
    // isMagic: CSV chua co cot nay nen van suy ra tu owner.
    Skill skill;
    skill.m_id = data.m_id;
    skill.m_name = data.m_name;
    skill.m_ownerType = data.m_ownerType;
    skill.m_owner = data.m_owner;
    skill.m_type = data.m_type;
    skill.m_power = data.m_power;     // DAMAGE/DEBUFF: % cua ATK; HEAL: luong HP hoi co dinh
    skill.m_mpCost = data.m_mpCost;
    skill.m_effect = data.m_effect;
    skill.m_effectTurns = data.m_effectTurns;

    if (data.m_hasElement)
    {
        skill.m_element = data.m_element;
    }
    else
    {
        switch (data.m_effect)
        {
        case EffectType::BURN:     skill.m_element = Element::HOA;  break;
        case EffectType::SLOW:     skill.m_element = Element::THUY; break;
        case EffectType::STUN:     skill.m_element = Element::THO;  break;
        case EffectType::WEAKNESS: skill.m_element = Element::MOC;  break;
        default:                   skill.m_element = heroElement;   break;
        }
    }

    skill.m_isMagic = (data.m_ownerType == SkillOwnerType::CLASS)
        ? (data.m_owner == "Mage" || data.m_owner == "Enchanter")
        : (data.m_owner == "Elf");
    return skill;
}

void GameEngine::addCombatant(std::unique_ptr<Character> character, const std::string& spritePath)
{
    if (!character) return;

    // Thieu file anh rieng -> dung sheet mac dinh de game khong bi mat hinh
    int frameW = FRAME_W, frameH = FRAME_H;
    std::string sheet = resolvePath(spritePath.empty() ? SPRITE_SHEET_PATH : spritePath);
    if (!FileExists(sheet.c_str()))
        sheet = resolvePath(SPRITE_SHEET_PATH);
    else if (dynamic_cast<const Boss*>(character.get()))
        frameW = frameH = BOSS_FRAME;            // boss dung sheet khung lon (192x192)

    auto view = std::make_unique<CombatantView>(character.get(), sheet,
                                            frameW, frameH, BattleSlot{});

    m_lastHp[character.get()] = character->getCurrentHp();
    m_turnManager.addCharacter(character.get());
    m_combatants.push_back(std::move(character));
    m_views.push_back(std::move(view));
}
void GameEngine::layoutViews()
{
    if (m_views.empty()) return;

    // Hero: luôn ở m_combatants[0]
    m_views[0]->setSlot(m_battleMap.layoutPlayers({ 1.0f })[0]);

    // Quái: Boss to 1.3, quái thường 1.0
    std::vector<float> muls;
    for (size_t i = 1; i < m_combatants.size(); ++i)
        muls.push_back(dynamic_cast<const Boss*>(m_combatants[i].get())
                   ? BOSS_SCALE_MUL * static_cast<float>(FRAME_W) / BOSS_FRAME : 1.0f);

    const auto slots = m_battleMap.layoutEnemies(muls);
    for (size_t i = 0; i < slots.size() && i + 1 < m_views.size(); ++i)
        m_views[i + 1]->setSlot(slots[i]);
}

std::vector<Character*> GameEngine::upcomingOrder() const
{
    std::vector<Character*> order;
    for (Character* c : m_turnManager.getCharacters())
        if (c && c->isAlive()) order.push_back(c);

    // Cung quy tac voi priority_queue: toc do cao di truoc
    std::stable_sort(order.begin(), order.end(), [](const Character* a, const Character* b) {
        return a->getStats().m_spd > b->getStats().m_spd;
    });
    return order;
}
void GameEngine::markWaveCleared(int stage)
{
    if (stage < 1) return;
    if (m_wavesCleared.size() <= static_cast<size_t>(stage))
        m_wavesCleared.resize(static_cast<size_t>(stage) + 1, false);
    m_wavesCleared[static_cast<size_t>(stage)] = true;
    m_unlockedWave = std::max(m_unlockedWave, std::min(stage + 1, finalStage()));   // xong wave n -> mo wave n+1
}

void GameEngine::openWaveSelect()
{
    std::vector<WaveInfo> waves;
    for (int stage = 1; stage <= finalStage(); ++stage)
    {
        WaveInfo w;
        w.m_number = stage;
        w.m_title = "WAVE " + std::to_string(stage);

        const std::vector<MonsterData> monsters = m_gameData.getMonstersByStage(stage);
        for (const MonsterData& m : monsters)
        {
            w.m_enemies.push_back({ m.m_name, m.m_element });
            if (m.m_isBoss) w.m_isBoss = true;
        }

        // Wave 1 chi xuat hien ngau nhien STAGE1_ENEMY_COUNT quai trong so quai co the gap (xem spawnEnemies)
        const int appear = (stage == 1) ? std::min(static_cast<int>(monsters.size()), STAGE1_ENEMY_COUNT)
                                        : static_cast<int>(monsters.size());
        w.m_description = w.m_isBoss ? std::string("Tran chien voi Boss")
                                     : "Xuat hien ngau nhien " + std::to_string(appear) + " quai";
        w.m_locked = stage > m_unlockedWave;
        w.m_cleared = static_cast<size_t>(stage) < m_wavesCleared.size() && m_wavesCleared[static_cast<size_t>(stage)];
        waves.push_back(std::move(w));
    }

    m_waveSelect.setWaves(waves);
    m_waveSelect.setHeroSummary(m_heroConfig.m_name + " - " + m_heroConfig.m_race + " " + m_heroConfig.m_class);
    m_waveSelect.reset(0);
    m_stateMachine.changeState(GameStateMachine::State::WAVE_SELECT);
}
void GameEngine::leavePrepMenu()
{
    if (m_dataLoaded && finalStage() > 1)
        m_stateMachine.changeState(GameStateMachine::State::WAVE_SELECT);
    else
        m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
}

void GameEngine::openPrepMenu(int wave)
{
    m_pendingWave = wave;
    PrepInfo info;

    // Tab 1: Doi hinh
    info.m_hero.m_name = m_heroConfig.m_name;
    info.m_hero.m_race = m_heroConfig.m_race;
    info.m_hero.m_class = m_heroConfig.m_class;
    info.m_hero.m_element = m_heroConfig.m_element;
    info.m_hero.m_stats = Stats{ 120, 50, 25, 20, 5, 8, 15 };   // du phong
    info.m_teammateSlots = 2;

    // Tab 2: Ky nang
    try {
        info.m_hero.m_stats = m_gameData.getBaseStats(m_heroConfig.m_race, m_heroConfig.m_class);
        for (const auto& id : m_gameData.getHeroSkillIds(m_heroConfig.m_race, m_heroConfig.m_class))
        {
            const Skill s = toBattleSkill(m_gameData.getSkill(id), m_heroConfig.m_element);
            info.m_skills.push_back({ s.m_name, s.m_element, skillTypeText(s.m_type), s.m_power,
                                      s.m_mpCost, effectText(s.m_effect, s.m_effectTurns), s.m_isMagic });
        }
    }                                  
    catch (const std::exception& e) {
        std::cerr << "Prep menu data error: " << e.what() << std::endl;
    }
    // Tab 3: Tui do
    info.m_items.push_back({ POTION_NAME, INITIAL_POTIONS });

    // Tab 4: Thong tin cong
    info.m_gate.m_title = "WAVE " + std::to_string(wave);
    info.m_gate.m_deadlineTurns = deadlineTurnsForStage(wave);
    for (const MonsterData& m : m_gameData.getMonstersByStage(wave))
    {
        if (m.m_isBoss) {
            info.m_gate.m_hasBoss = true;
            info.m_gate.m_bossName = m.m_name;
            info.m_gate.m_bossElement = m.m_element;
            info.m_gate.m_bossHp = m.m_stats.m_hp;
        } else {
            info.m_gate.m_enemies.push_back({ m.m_name, m.m_element });
        }
    }
    info.m_gate.m_description = info.m_gate.m_hasBoss
        ? std::string("Tran chien voi Boss\nHa gac cong de tien len")
        : std::string("Quet sach quai canh cong\nXuat hien ngau nhien");

    m_prepMenu.setInfo(info);
    m_prepMenu.reset(0);
    m_stateMachine.changeState(GameStateMachine::State::PREP_MENU);
}
void GameEngine::setupHeroCreateScreen()
{
    if (!m_dataLoaded) return;   // khong co du lieu -> bo qua man Tao Anh Hung

    auto skillNames = [this](const std::vector<std::string>& ids) {
        std::vector<std::string> names;
        for (const auto& id : ids)
        {
            try { names.push_back(m_gameData.getSkill(id).m_name); }
            catch (const std::exception&) { names.push_back(id); }
        }
        return names;
    };

    std::vector<HeroOption> races, classes;
    for (const RaceData& r : m_gameData.getRaces())
        races.push_back({ r.m_name, r.m_stats, skillNames(r.m_skillIds) });
    for (const ClassData& c : m_gameData.getClasses())
        classes.push_back({ c.m_name, c.m_stats, skillNames(c.m_skillIds) });

    m_heroCreate.setOptions(races, classes);
}

void GameEngine::applyHeroSelection(const HeroSelection& selection)
{
    const auto& races = m_gameData.getRaces();
    const auto& classes = m_gameData.getClasses();
    if (selection.m_raceIndex < races.size())    m_heroConfig.m_race = races[selection.m_raceIndex].m_name;
    if (selection.m_classIndex < classes.size()) m_heroConfig.m_class = classes[selection.m_classIndex].m_name;
    m_heroConfig.m_name = selection.m_name;
    m_heroConfig.m_element = selection.m_element;
}
void GameEngine::createHero()
{
    Stats heroStats{ 120, 50, 25, 20, 5, 8, 15 };
    m_heroSkills.clear();

    bool fromData = false;
    if (m_dataLoaded)
    {
        try {
            heroStats = m_gameData.getBaseStats(m_heroConfig.m_race, m_heroConfig.m_class);
            for (const auto& id : m_gameData.getHeroSkillIds(m_heroConfig.m_race, m_heroConfig.m_class))
    m_heroSkills.push_back(toBattleSkill(m_gameData.getSkill(id), m_heroConfig.m_element));
            fromData = true;
        }
        catch (const std::exception& e) {
            std::cerr << "Hero data error: " << e.what() << std::endl;
            m_heroSkills.clear();
        }
    }

    if (!fromData)
    {
        // Du phong khi chua co CSV
        m_heroSkills = {
            Skill{ "S01", "Chem Manh", SkillOwnerType::CLASS, "Warrior", SkillType::DAMAGE, 130, 5, EffectType::NONE, 0, Element::KIM, false },
            Skill{ "S02", "Hoa Cau", SkillOwnerType::CLASS, "Mage", SkillType::DAMAGE, 150, 12, EffectType::BURN, 2, Element::HOA, true },
            Skill{ "S03", "Hoi Mau", SkillOwnerType::CLASS, "Priest", SkillType::HEAL, 30, 10, EffectType::REGEN, 2, Element::THUY, true }
        };
    }

    addCombatant(std::make_unique<Player>(m_heroConfig.m_name, heroStats, m_heroConfig.m_element));
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
                addCombatant(std::make_unique<Boss>(m.m_name, m.m_stats, m.m_element),
                             monsterSpritePath(m.m_id));      // <-- THEM tham so thu 2
            else
                addCombatant(std::make_unique<Enemy>(m.m_name, m.m_stats, m.m_element),
                             monsterSpritePath(m.m_id));      // <-- THEM tham so thu 2
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
        m_views.resize(1);
    }

    m_turnManager.clear();
    m_commandStack.clear();
    m_statusEffects.clear();
    m_lastHp.clear();
    m_ui.unloadAll();
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
    layoutViews();

    m_selectedTarget = (m_combatants.size() > 1) ? m_combatants[1].get() : nullptr;
    m_battleMap.setStage(m_stage == 1 ? StageId::STAGE_1 : StageId::STAGE_2,
    resolvePath(m_stage == 1 ? "assets/images/bg_wave1.png"
                           : "assets/images/bg_wave2.png"));

    m_currentCharacter = nullptr;   // update() se goi advanceTurn() de lay luot dau tien
    m_battlePhase = BattlePhase::AI_THINKING;
    m_battleLog = "MAN " + std::to_string(stage) + " bat dau!";
}

void GameEngine::startNewGame(int startStage)
{
    m_combatants.clear();
    m_views.clear();
    m_ui.unloadAll();

    m_inventory = Inventory<std::string>();
    for (int i = 0; i < INITIAL_POTIONS; ++i)
        m_inventory.addItem(POTION_NAME);

    createHero();
    loadStage(startStage); 
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
        if (m_stateMachine.is(GameStateMachine::State::CREATE_HERO))
    {
    m_stateMachine.changeState(GameStateMachine::State::MENU);
    }
    else if (m_stateMachine.is(GameStateMachine::State::WAVE_SELECT))
    {
    m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);   // ESC o man Chon Wave = ve Tao Hero
    }
    else if (m_stateMachine.is(GameStateMachine::State::PREP_MENU))
    {
    leavePrepMenu();
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
    switch (m_startMenu.handleInput())
    {
    case MenuAction::START_GAME:
    if (m_heroCreate.hasOptions())
    {
        m_heroCreate.reset(m_heroConfig.m_name);
        m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
    }
    else   // chua co data CSV: vao tran luon voi hero mac dinh
    {
        m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        startNewGame();
    }
    break;
    case MenuAction::EXIT:
        m_isRunning = false;
        break;
    case MenuAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::CREATE_HERO:
    switch (m_heroCreate.handleInput())
    {
    case HeroCreateAction::CONFIRM:
    applyHeroSelection(m_heroCreate.getSelection());
    if (m_dataLoaded && finalStage() > 1)
        openWaveSelect();   // co nhieu wave -> cho nguoi choi chon
        else if (m_dataLoaded)
    openPrepMenu(1); 
    else
    {
        m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        startNewGame();
    }
    break;
    case HeroCreateAction::BACK:
        m_stateMachine.changeState(GameStateMachine::State::MENU);
        break;
    case HeroCreateAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::WAVE_SELECT:
    switch (m_waveSelect.handleInput())
    {
    case WaveSelectAction::START:
        openPrepMenu(m_waveSelect.getSelectedWave());
        break;
    case WaveSelectAction::BACK:
        m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
        break;
    case WaveSelectAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::PREP_MENU:
    switch (m_prepMenu.handleInput())
    {
    case PrepMenuAction::START:
        m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        startNewGame(m_pendingWave);
        break;
    case PrepMenuAction::BACK:
        leavePrepMenu();
        break;
    case PrepMenuAction::NONE:
        break;
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
        for (size_t i = 0; i < m_heroSkills.size() && i < 4; ++i)
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
        doUseItem();
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

void GameEngine::doUseItem()
{
    Character* h = hero();
    if (m_inventory.getItems().empty()) {
        m_battleLog = "Het binh mau!";
        return;
    }
    if (h->getCurrentHp() >= h->getStats().m_hp) {
        m_battleLog = "HP da day, khong can dung Potion.";
        return;
    }

    auto healed = std::make_shared<int>(0);

    auto executeAction = [this, h, healed]() {
        const int before = h->getCurrentHp();
        h->heal(POTION_HEAL);
        *healed = h->getCurrentHp() - before;
        m_inventory.removeItem(POTION_NAME);
        syncHealth(h);
    };
    auto undoAction = [this, h, healed]() {
        h->takeDamage(*healed);     // tru lai dung luong da hoi
        m_inventory.addItem(POTION_NAME);
    };

    queuePlayerCommand(std::make_unique<UseItemCommand>(executeAction, undoAction));

    m_battleLog = h->getName() + " dung Potion hoi " + std::to_string(*healed) + " HP!  [ENTER] ket thuc luot, [ESC/Z] hoan tac";
    m_battlePhase = BattlePhase::PLAYER_CONFIRM;
}

bool GameEngine::castSkill(size_t skillIndex)
{
    if (skillIndex >= m_heroSkills.size()) return false;

    Character* h = hero();
    const Skill skill = m_heroSkills[skillIndex];   // ban sao: thao tac an toan trong lambda

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
    if (m_stateMachine.is(GameStateMachine::State::MENU))
    m_startMenu.update(deltaTime);
    if (m_stateMachine.is(GameStateMachine::State::CREATE_HERO))
    m_heroCreate.update(deltaTime);
    if (m_stateMachine.is(GameStateMachine::State::WAVE_SELECT))
    m_waveSelect.update(deltaTime);
    if (m_stateMachine.is(GameStateMachine::State::PREP_MENU))
    m_prepMenu.update(deltaTime);
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) return;

    // Cap nhat sprite + hieu ung UI (chu bay)
    for (auto& v : m_views) v->update(deltaTime);
    m_ui.update(deltaTime);

    // Dang cho nguoi choi xac nhan/undo: khong xet ket thuc tran (muc tieu co the hoi sinh khi undo)
    if (m_battlePhase != BattlePhase::PLAYER_CONFIRM)
        checkBattleCondition();
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) 
    return;

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
    markWaveCleared(m_stage);
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
    // Character khong co API tru MP rieng; cach duy nhat la executeSkill().
    // Goi len chinh hero voi power = 0 roi hoi lai dung luong HP vua mat.
    // TODO(Entities): them Character::spendMp(int) de bo workaround nay.
    Character* h = hero();
    Skill free = skill;
    free.m_power = 0;
    const int hpBefore = h->getCurrentHp();
    h->executeSkill(&free, h);
    const int lost = hpBefore - h->getCurrentHp();
    if (lost > 0) h->heal(lost);
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
    if (idx >= 0 && static_cast<size_t>(idx) < m_views.size())
        m_views[static_cast<size_t>(idx)]->play(state, true);
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
    const int idx = indexOf(character);
    if (delta == 0 || idx < 0 || static_cast<size_t>(idx) >= m_views.size()) return;

    CombatantView& view = *m_views[static_cast<size_t>(idx)];
    Vector2 pos = view.getHeadPos();
    pos.x += static_cast<float>(GetRandomValue(-20, 20));
    m_ui.spawnFloatingText(pos, delta, delta > 0);

    if (delta < 0)
    {
        view.triggerHitFlash();
        if (newHealth > 0)
            playAnimation(character, AnimState::TAKE_DAMAGE);
    }
}

void GameEngine::onCharacterDied(Character* character)
{
    if (character)
        m_battleLog += "  >> " + character->getName() + " guc nga!";
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

bool GameEngine::isHero(const Character* character) const
{
    return character != nullptr && character == hero();
}

BattleSlot GameEngine::slotOf(int index) const
{
    return m_views[static_cast<size_t>(index)]->getSlot();
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
        m_startMenu.draw();
        break;
    case GameStateMachine::State::CREATE_HERO:
    m_heroCreate.draw();
    break;
    case GameStateMachine::State::WAVE_SELECT:
    m_waveSelect.draw();
    break;
    case GameStateMachine::State::PREP_MENU:
    m_prepMenu.draw();
    break;
    case GameStateMachine::State::IN_BATTLE:
        m_battleMap.draw();
        renderCombatants();
        renderBattleUI();
        m_ui.drawFloatingTexts();
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
    // 2.5D: ve hang xa truoc, hang gan sau (sap theo y cua chan)
    std::vector<size_t> order(m_views.size());
    std::iota(order.begin(), order.end(), size_t{ 0 });
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return m_views[a]->getSlot().m_feetPos.y < m_views[b]->getSlot().m_feetPos.y;
    });

    for (size_t i : order)
    {
        const Color tint = (i == 0) ? WHITE : elementColor(m_combatants[i]->getElement());
        m_views[i]->draw(m_battleMap, tint);   // da gom bong + mo dan khi chet
    }
}

void GameEngine::renderBattleUI()
{
    // ----- HP/MP + con tro muc tieu + hieu ung tung nhan vat -----
    for (size_t i = 0; i < m_combatants.size(); ++i)
    {
        const Character* c = m_combatants[i].get();
        if (!c->isAlive()) continue;

        const Vector2 head = m_views[i]->getHeadPos();
        m_ui.drawHpMpBar(*c, head);

        if (c == m_selectedTarget)
            m_ui.drawTargetCursor(m_views[i]->getSlot());

        // Hieu ung dang co (giu logic cu cua Core; BattleUI chua ve phan nay)
        auto fx = m_statusEffects.find(c);
        if (fx != m_statusEffects.end())
        {
            int ex = static_cast<int>(head.x) - 60;
            const int ey = static_cast<int>(head.y) - 12;
            for (const StatusEffect& effect : fx->second.getEffects())
            {
                const char* label = TextFormat("%s(%d)", statusName(effect.type), effect.durationTurns);
                DrawText(label, ex, ey, 12, ORANGE);
                ex += MeasureText(label, 12) + 6;
            }
        }
    }

    // ----- Danh sach luot (goc phai tren) -----
       m_turnOrderBar.draw(upcomingOrder(), m_currentCharacter);

    // ----- Bang dieu khien -----
    m_ui.drawControlPanel();
    m_ui.drawTurnBanner(m_stage, m_currentCharacter,
                        m_currentCharacter && isHero(m_currentCharacter),
                        m_battlePhase == BattlePhase::PLAYER_CONFIRM);

    if (m_battlePhase == BattlePhase::PLAYER_CONFIRM)
        m_ui.drawCommandHelp("[ENTER] Ket thuc luot  |  [ESC/Z] Hoan tac (Undo)");
    else
        m_ui.drawCommandHelp(TextFormat("[A] Tan Cong | [S] Ky Nang | [I] Potion x%d | [TAB] Doi Muc Tieu | [ESC] Menu",
                                        static_cast<int>(m_inventory.getItems().size())));
    m_ui.drawBattleLog("BATTLE LOG: " + m_battleLog);

    // ----- Menu ky nang (popup giua man hinh, ve cuoi cung de nam tren cung) -----
    if (m_isSelectingSkill)
        m_ui.drawSkillMenu(m_heroSkills, m_selectedSkillIndex, hero() ? hero()->getCurrentMp() : 0);
}
