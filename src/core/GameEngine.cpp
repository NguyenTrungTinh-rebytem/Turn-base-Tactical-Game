#include "core/GameEngine.h"
#include <string>
#include <unordered_map>
#include "core/AttackCommand.h"
#include "core/UseSkillCommand.h"
#include "core/UseItemCommand.h"
#include "core/CharacterSnapshot.h"
#include "entities/CharacterFactory.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <exception>
#include <iostream>
#include "data/HeroPresetConfig.h"
#include "data/CharacterRules.h"
#include <cctype>

namespace
{   
    constexpr float BOSS_SCALE_MUL = 3.2f;   // boss to gap may lan quai thuong
    constexpr float BOSS_OFFSET_X = 150.0f;   // đẩy Boss ra xa phe hero (px, dương = sang phải)
    constexpr float BOSS_OFFSET_Y = 0.0f;     // 0 = giữ nguyên hàng; số âm = lùi lên cao hơn (trông xa hơn)
    constexpr float ENEMY_SCALE_MUL = 1.3f;
    constexpr int   BOSS_FRAME     = 192;    // sheet boss: moi frame 192x192
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;

    // ----- Sprite -----
    constexpr int FRAME_W = 64;
    constexpr int FRAME_H = 64;
    const char* const SPRITE_SHEET_PATH = "assets/images/test_sheet.png";
    // Hero dung sheet chien dau rieng: assets/images/hero_<id>_battle.png, o 128x128 (3 hang: idle 4 / attack 4 / hurt 2).
    constexpr int   HERO_FRAME = 128;
    constexpr float HERO_SCALE_MUL = 0.6f;   // o 128 ve nho lai de gan bang quai (o 64)
    // id hero = ten viet thuong, bo hau to " 2"/" 3" (do applyHeroSelection them khi trung ten)
    std::string heroSpritePath(const std::string& heroName)
    {
        std::string id;
        for (char ch : heroName) id += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        while (!id.empty() && (std::isdigit(static_cast<unsigned char>(id.back())) || id.back() == ' ')) id.pop_back();
        if (id.empty()) return "";
        return "assets/images/hero_" + id + "_battle.png";
    }
    
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
    constexpr int   INITIAL_POTIONS = 3;
    constexpr int   STAGE1_ENEMY_COUNT = 3;
    constexpr float STAGE_CLEAR_HEAL_RATE = 0.3f;   // hoi 30% HP khi qua man
    const char* const POTION_NAME = "ITM_HP_MINOR";
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
    {
        std::string itemError;
        m_itemsLoaded = m_itemDatabase.tryLoad(resolvePath("data"), &itemError);
        if (!m_itemsLoaded) std::cerr << "Item data: " << itemError << std::endl;
        else {
            for (const ItemData& item : m_itemDatabase.getItems())
                if (item.m_category == ItemCategory::HELD) m_gearIds.push_back(item.m_id);
        }
    }
    m_startMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));   // thieu anh -> nen thu tuc
    m_startMenu.setStatusText(m_dataLoaded ? "Data: CSV OK" : "Data: CSV loi -> dung du lieu du phong", m_dataLoaded);
    m_heroConfig = { HERO_NAME, HERO_RACE, HERO_CLASS, HERO_ELEMENT };
    m_partyConfigs.clear();
    m_heroCreate.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_waveSelect.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_prepMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_heroCreate.loadPreviewSprite(resolvePath(SPRITE_SHEET_PATH), FRAME_W, FRAME_H);
    setupHeroCreateScreen();
    m_combatSubject.addObserver(this);
    m_battleMap.setStage(m_stage == 1 ? StageId::STAGE_1 : StageId::STAGE_2,
    resolvePath(m_stage == 1 ? "assets/images/bg_wave1.png"
                           : "assets/images/bg_wave2.png"));
    m_waveManager.reset();
    m_waveManager.unlockWave(1);
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
    m_party.clear();
    m_baseSpeed.clear();
    m_scaleMul.clear();

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
    float scaleMul = 1.0f;
    std::string sheet = resolvePath(spritePath.empty() ? SPRITE_SHEET_PATH : spritePath);
    if (!FileExists(sheet.c_str()))
        sheet = resolvePath(SPRITE_SHEET_PATH);
    else if (dynamic_cast<const Boss*>(character.get()))
        frameW = frameH = BOSS_FRAME;            // boss dung sheet khung lon (192x192)
    else if (dynamic_cast<const Player*>(character.get()))
    {
        frameW = frameH = HERO_FRAME;            // hero co sheet rieng khung 128x128
        scaleMul = HERO_SCALE_MUL;
    }
    m_scaleMul[character.get()] = scaleMul;

    auto view = std::make_unique<CombatantView>(character.get(), sheet,
                                            frameW, frameH, BattleSlot{});

    m_baseSpeed[character.get()] = character->getSpeed();
    m_lastHp[character.get()] = character->getCurrentHp();
    m_turnManager.addCharacter(character.get());
    m_combatants.push_back(std::shared_ptr<Character>(std::move(character)));
    m_views.push_back(std::move(view));
}
void GameEngine::layoutViews()
{
    const std::size_t members = m_party.size();
    if (m_views.empty() || members == 0) return;

    std::vector<float> playerScales(members, 1.0f);
    for (std::size_t i = 0; i < members && i < m_combatants.size(); ++i)
    {
        const auto it = m_scaleMul.find(m_combatants[i].get());
        if (it != m_scaleMul.end()) playerScales[i] = it->second;
    }
    const auto playerSlots = m_battleMap.layoutPlayers(playerScales);
    for (std::size_t i = 0; i < members && i < m_views.size(); ++i)
        m_views[i]->setSlot(playerSlots[i]);

    std::vector<float> enemyScales;
    for (std::size_t i = members; i < m_combatants.size(); ++i)
        enemyScales.push_back(dynamic_cast<const Boss*>(m_combatants[i].get())
            ? BOSS_SCALE_MUL * static_cast<float>(FRAME_W) / BOSS_FRAME : ENEMY_SCALE_MUL);
    const auto enemySlots = m_battleMap.layoutEnemies(enemyScales);
    for (std::size_t i = 0; i < enemySlots.size() && i + members < m_views.size(); ++i)
    {
        BattleSlot slot = enemySlots[i];
        if (dynamic_cast<const Boss*>(m_combatants[i + members].get()))
        {
            slot.m_feetPos.x += BOSS_OFFSET_X;   // đẩy Boss ra xa phe hero
            slot.m_feetPos.y += BOSS_OFFSET_Y;
        }
        m_views[i + members]->setSlot(slot);
    }
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
    m_waveManager.completeCurrentWave();
    m_waveManager.unlockWave(std::min(stage + 1, finalStage()));
    m_unlockedWave = std::max(m_unlockedWave, std::min(stage + 1, finalStage()));
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
        w.m_locked = !m_waveManager.isWaveUnlocked(stage);
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
    info.m_teammateSlots = static_cast<int>(Party::MAX_SIZE) - static_cast<int>(m_partyConfigs.size());
    for (const HeroConfig& config : m_partyConfigs) {
        PrepHeroInfo member;
        member.m_name = config.m_name;
        member.m_race = config.m_race;
        member.m_class = config.m_class;
        member.m_element = config.m_element;
        try { member.m_stats = m_gameData.getBaseStats(config.m_race, config.m_class); }
        catch (const std::exception&) { member.m_stats = Stats{120,50,25,20,5,8,15}; }
        info.m_members.push_back(member);
    }

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
    if (m_itemsLoaded) {
        const auto& all = m_itemDatabase.getItems();
        for (const ItemData& item : all)
            info.m_items.push_back({ item.m_name, item.m_id == POTION_NAME ? INITIAL_POTIONS :
                                                 item.m_category == ItemCategory::HELD ? 1 : 1 });
    }

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
    
        // ----- Tuong co san: data/heroes.csv (thieu file -> danh sach mac dinh) -----
    std::vector<HeroPresetData> presetData;
    if (loadHeroPresets(resolvePath("data/heroes.csv"), presetData) == 0)
        presetData = defaultHeroPresets();

    auto lower = [](std::string text) {
        for (char& ch : text) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        return text;
    };
    auto findOption = [&lower](const std::vector<HeroOption>& list, const std::string& name) -> int {
        for (size_t i = 0; i < list.size(); ++i)
            if (lower(list[i].m_name) == lower(name)) return static_cast<int>(i);
        return -1;
    };

    std::vector<HeroPreset> presets;
    for (const HeroPresetData& d : presetData)
    {
        const int raceIndex = findOption(races, d.m_race);
        const int classIndex = findOption(classes, d.m_class);
        if (raceIndex < 0 || classIndex < 0)
        {
            std::cerr << "heroes.csv: bo qua '" << d.m_id << "' (khong tim thay " << d.m_race << " / " << d.m_class << ")" << std::endl;
            continue;
        }

        HeroPreset p;
        p.m_id = d.m_id;
        p.m_name = d.m_name;
        p.m_raceIndex = static_cast<size_t>(raceIndex);
        p.m_classIndex = static_cast<size_t>(classIndex);
        p.m_element = d.m_element;
        p.m_idleSpritePath = resolvePath("assets/images/hero_" + d.m_id + "_idle.png");
        presets.push_back(p);
    }
    m_heroCreate.setPresets(presets);
}

void GameEngine::applyHeroSelection(const HeroSelection& selection)
{
    const auto& races = m_gameData.getRaces();
    const auto& classes = m_gameData.getClasses();
    HeroConfig config;
    if (selection.m_raceIndex < races.size()) config.m_race = races[selection.m_raceIndex].m_name;
    if (selection.m_classIndex < classes.size()) config.m_class = classes[selection.m_classIndex].m_name;
    config.m_name = selection.m_name;
    config.m_element = selection.m_element;
    if (config.m_race.empty() || config.m_class.empty() || config.m_name.empty()) return;

    // Ten phai rieng biet de hien thi ro trong log, HP bar va thu tu luot.
    for (std::size_t i = 0; i < m_partyConfigs.size(); ++i) {
        if (static_cast<int>(i) != m_editingPartySlot && m_partyConfigs[i].m_name == config.m_name) {
            config.m_name += " " + std::to_string(m_editingPartySlot + 1);
            break;
        }
    }
    if (m_editingPartySlot == 0) m_heroConfig = config;
    if (m_editingPartySlot < static_cast<int>(m_partyConfigs.size()))
        m_partyConfigs[static_cast<std::size_t>(m_editingPartySlot)] = config;
    else if (m_partyConfigs.size() < Party::MAX_SIZE)
        m_partyConfigs.push_back(config);
}

void GameEngine::openPartyMemberSelection(int slot)
{
    // Chon lan luot de khong co o trong chen giua doi hinh.
    const int maxFilled = static_cast<int>(m_partyConfigs.size());
    m_editingPartySlot = std::max(0, std::min(slot, maxFilled));
    m_selectingFromPrep = true;
    m_heroCreate.reset();
    m_heroCreate.setSlotLabel(m_editingPartySlot == 0
        ? "CHON THU LINH (1/3)"
        : "CHON DONG DOI " + std::to_string(m_editingPartySlot + 1) + "/3");
    m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
}

void GameEngine::createHero()
{
    m_heroSkills.clear();
    m_party.clear();
    auto addPartyHero = [this](std::unique_ptr<Player> player) {
        if (!player) return;
        const std::string sprite = heroSpritePath(player->getName());   // hero_<ten>_battle.png (thieu -> sheet mac dinh)
        addCombatant(std::move(player), sprite);
        m_party.addMember(std::dynamic_pointer_cast<Player>(m_combatants.back()));
    };

    if (m_dataLoaded) {
        const std::vector<HeroConfig> configs = m_partyConfigs.empty()
            ? std::vector<HeroConfig>{m_heroConfig} : m_partyConfigs;
        for (const HeroConfig& config : configs) {
            try {
                addPartyHero(CharacterFactory::createHero(m_gameData, config.m_name,
                    config.m_race, config.m_class, config.m_element));
            } catch (const std::exception& e) {
                std::cerr << "Hero factory error (" << config.m_name << "): " << e.what() << std::endl;
            }
        }
    }
    if (m_party.isEmpty()) {
        auto player = std::make_unique<Player>(m_heroConfig.m_name,
            Stats{120, 50, 25, 20, 5, 8, 15}, m_heroConfig.m_element);
        const std::vector<Skill> defaultSkills = {
            Skill{"S01", "Chem Manh", SkillOwnerType::CLASS, "Warrior", SkillType::DAMAGE, 130, 5, EffectType::NONE, 0, Element::KIM, false},
            Skill{"S02", "Hoa Cau", SkillOwnerType::CLASS, "Mage", SkillType::DAMAGE, 150, 12, EffectType::BURN, 2, Element::HOA, true},
            Skill{"S03", "Hoi Mau", SkillOwnerType::CLASS, "Priest", SkillType::HEAL, 30, 10, EffectType::REGEN, 2, Element::THUY, true}
        };
        for (const Skill& skill : defaultSkills) player->addSkill(skill);
        addPartyHero(std::move(player));
    }
    refreshCurrentSkills();
}

void GameEngine::refreshCurrentSkills()
{
    const auto* player = dynamic_cast<const Player*>(hero());
    m_heroSkills = player ? player->getSkills() : std::vector<Skill>{};
    m_selectedSkillIndex = 0;
}

void GameEngine::seedItems()
{
    if (!m_itemsLoaded) return;
    auto seed = [this](const char* id, int count) {
        if (!m_itemDatabase.findItem(id)) return;
        for (int i = 0; i < count; ++i)
            m_party.addItem(ItemFactory::createItem(m_itemDatabase, id));
    };
    seed(POTION_NAME, INITIAL_POTIONS);
    seed("ITM_HP_MAJOR", 1);
    seed("ITM_ENERGY", 2);
    seed("ITM_PHOENIX", 2);
    seed("ITM_POWER_SERUM", 1);
    seed("ITM_IRON_SHIELD", 1);
    seed("ITM_HASTE", 1);
    seed("ITM_PRECISION", 1);
    for (const std::string& id : m_gearIds) seed(id.c_str(), 1);
    if (!m_gearIds.empty() && m_prepGearIndex > 0) {
        const std::string& id = m_gearIds[static_cast<std::size_t>(m_prepGearIndex - 1)];
        const auto& items = m_party.getInventory().getItems();
        auto it = std::find_if(items.begin(), items.end(), [&](const ItemPtr& p) { return p->getId() == id; });
        if (it != items.end()) m_party.equipGear(0, *it);
    }
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
    m_waveManager.addWave(stage);
    m_waveManager.startNextWave();

    // Giu lai moi thanh vien Party, bo quai cu
    for (std::size_t i = m_party.size(); i < m_combatants.size(); ++i)
        m_baseSpeed.erase(m_combatants[i].get());
    m_combatants.resize(m_party.size());
    m_views.resize(m_party.size());

    m_turnManager.clear();
    m_commandStack.clear();
    m_statusEffects.clear();
    m_lastHp.clear();
    m_ui.unloadAll();
    m_isSelectingSkill = false;
    m_isSelectingItem = false;
    m_aiTurnTimer = 0.0f;

    for (const auto& member : m_party.getMembers()) {
        Character* h = member.get();
        if (stage > 1 && h->isAlive())
            h->heal(static_cast<int>(static_cast<float>(h->getMaxHp()) * STAGE_CLEAR_HEAL_RATE));
        h->resetBattleState();
        auto speed = m_baseSpeed.find(h);
        if (speed != m_baseSpeed.end()) h->setSpeed(speed->second);
        if (h->isAlive()) m_turnManager.addCharacter(h);
        m_lastHp[h] = h->getCurrentHp();
    }

    spawnEnemies(stage);
    layoutViews();

    m_selectedAlly = m_party.getLeader();
    m_selectedTarget = (m_combatants.size() > m_party.size()) ? m_combatants[m_party.size()].get() : nullptr;
    m_battleMap.setStage(m_stage == 1 ? StageId::STAGE_1 : StageId::STAGE_2,
    resolvePath(m_stage == 1 ? "assets/images/bg_wave1.png"
                           : "assets/images/bg_wave2.png"));

    m_currentCharacter = nullptr;   // update() se goi advanceTurn() de lay luot dau tien
    m_battlePhase = BattlePhase::AI_THINKING;
    m_battleLog = "MAN " + std::to_string(stage) + " bat dau!";
}

void GameEngine::startNewGame(int startStage)
{
    m_currentCharacter = nullptr;
    m_selectedTarget = nullptr;
    m_selectedAlly = nullptr;
    m_turnManager.clear();
    m_statusEffects.clear();
    m_commandStack.clear();
    m_combatants.clear();
    m_baseSpeed.clear();
    m_scaleMul.clear();
    m_views.clear();
    m_ui.unloadAll();

    createHero();
    seedItems();
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
            if (m_isSelectingSkill || m_isSelectingItem) {
                m_isSelectingSkill = false;
                m_isSelectingItem = false;
            } else if (!cancelLastAction()) {
                m_stateMachine.changeState(GameStateMachine::State::MENU);
            }
        }
        else 
        if (m_stateMachine.is(GameStateMachine::State::CREATE_HERO))
    {
        if (m_selectingFromPrep) {
            m_selectingFromPrep = false;
            openPrepMenu(m_pendingWave);
        } else {
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        }
    }
    else if (m_stateMachine.is(GameStateMachine::State::WAVE_SELECT))
    {
    m_editingPartySlot = 0;
    m_selectingFromPrep = false;
    m_heroCreate.setSlotLabel("CHON THU LINH (1/3)");
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
        m_partyConfigs.clear();
        m_editingPartySlot = 0;
        m_selectingFromPrep = false;
        m_heroCreate.reset(m_heroConfig.m_name);
        m_heroCreate.setSlotLabel("CHON THU LINH (1/3)");
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
    if (m_selectingFromPrep) {
        m_selectingFromPrep = false;
        openPrepMenu(m_pendingWave);
    }
    else if (m_dataLoaded && finalStage() > 1)
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
        if (m_selectingFromPrep) {
            m_selectingFromPrep = false;
            openPrepMenu(m_pendingWave);
        } else m_stateMachine.changeState(GameStateMachine::State::MENU);
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
        m_editingPartySlot = 0;
        m_selectingFromPrep = false;
        m_heroCreate.setSlotLabel("CHON THU LINH (1/3)");
        m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
        break;
    case WaveSelectAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::PREP_MENU:
    if (IsKeyPressed(KEY_T)) {
        openPartyMemberSelection(std::min(static_cast<int>(m_partyConfigs.size()),
                                          static_cast<int>(Party::MAX_SIZE) - 1));
        break;
    }
    if (IsKeyPressed(KEY_G) && !m_gearIds.empty()) {
        m_prepGearIndex = (m_prepGearIndex + 1) % (static_cast<int>(m_gearIds.size()) + 1);
        openPrepMenu(m_pendingWave);
        break;
    }
    switch (m_prepMenu.handleInput())
    {
    case PrepMenuAction::EDIT_MEMBER_1:
        openPartyMemberSelection(0);
        break;
    case PrepMenuAction::EDIT_MEMBER_2:
        openPartyMemberSelection(1);
        break;
    case PrepMenuAction::EDIT_MEMBER_3:
        openPartyMemberSelection(2);
        break;
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
    for (size_t i = m_party.size(); i < m_combatants.size(); ++i)
    {
        if (m_combatants[i]->isAlive()) {
            m_selectedTarget = m_combatants[i].get();
            return;
        }
    }
}

void GameEngine::cycleTarget()
{
    const size_t first = m_party.size();
    const size_t count = m_combatants.size();
    if (count <= first) return;
    int current = indexOf(m_selectedTarget);
    for (size_t step = 1; step <= count - first; ++step) {
        size_t index = first + ((static_cast<size_t>(std::max(current, static_cast<int>(first))) - first + step) % (count - first));
        if (m_combatants[index]->isAlive()) {
            m_selectedTarget = m_combatants[index].get();
            return;
        }
    }
}

void GameEngine::selectAlly(int index)
{
    if (index >= 0 && static_cast<std::size_t>(index) < m_party.size())
        m_selectedAlly = m_party.getMember(static_cast<std::size_t>(index));
}

std::vector<ItemPtr> GameEngine::availableItems() const
{
    std::vector<ItemPtr> items;
    for (const ItemPtr& item : m_party.getInventory().getItems())
        if (item && item->getCategory() != ItemCategory::HELD &&
            std::none_of(items.begin(), items.end(), [&](const ItemPtr& existing) { return existing->getId() == item->getId(); }))
            items.push_back(item);
    return items;
}

void GameEngine::openItemMenu()
{
    m_isSelectingItem = !m_isSelectingItem;
    m_isSelectingSkill = false;
    m_selectedItemIndex = 0;
    m_selectedAlly = hero();
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

    if (IsKeyPressed(KEY_I)) openItemMenu();
    if (IsKeyPressed(KEY_S)) {
        m_isSelectingItem = false;
        m_isSelectingSkill = !m_isSelectingSkill;
    }
    if (m_isSelectingItem) {
        const auto items = availableItems();
        if (items.empty()) {
            m_battleLog = "Tui do khong co vat pham dung trong tran!";
            m_isSelectingItem = false;
            return;
        }
        const KeyboardKey keys[] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE};
        for (std::size_t i = 0; i < items.size() && i < 9; ++i)
            if (IsKeyPressed(keys[i])) m_selectedItemIndex = static_cast<int>(i);
        if (IsKeyPressed(KEY_F1)) selectAlly(0);
        if (IsKeyPressed(KEY_F2)) selectAlly(1);
        if (IsKeyPressed(KEY_F3)) selectAlly(2);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) doUseItem();
        return;
    }

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
                m_battlePhase = BattlePhase::PLAYER_CONFIRM; // ENTER xac nhan; ESC/Z Undo
            }
        }
        return;
    }

    if (IsKeyPressed(KEY_A))
        doBasicAttack();

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
    const auto items = availableItems();
    if (m_selectedItemIndex < 0 || static_cast<std::size_t>(m_selectedItemIndex) >= items.size()) return;
    const ItemPtr item = items[static_cast<std::size_t>(m_selectedItemIndex)];
    Character* target = m_selectedAlly ? m_selectedAlly : hero();
    if (!item || !target) return;
    if (!item->canUse(*target)) {
        m_battleLog = "Khong the dung " + item->getName() + " cho " + target->getName();
        return;
    }
    auto amount = std::make_shared<int>(0);
    auto executeAction = [this, item, target, amount]() {
        *amount = item->use(*target);
        m_party.getInventory().removeItem(item);
        syncHealth(target);
    };
    auto undoAction = [this, item, target, amount]() {
        item->revert(*target, *amount);
        m_party.addItem(item);
        syncHealth(target);
    };
    if (!queuePlayerCommand(std::make_unique<UseItemCommand>(executeAction, undoAction))) return;
    m_battleLog = hero()->getName() + " dung " + item->getName() + " cho " + target->getName()
                + " (+" + std::to_string(*amount) + ") [ENTER] Xac nhan, [Z] Undo";
    m_isSelectingItem = false;
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

    // Snapshot combat state before executing. Undo must restore MP, HP and status effects.
    auto targetSnapshot = std::make_shared<CharacterSnapshot>(*target);
    auto casterSnapshot = std::make_shared<CharacterSnapshot>(*h);
    auto previousEffects = std::make_shared<StatusEffectManager>(m_statusEffects[target]);
    auto executeAction = [this, h, target, skill, offensive, targetSnapshot, casterSnapshot, previousEffects]() {
        *targetSnapshot = CharacterSnapshot(*target);
        *casterSnapshot = CharacterSnapshot(*h);
        *previousEffects = m_statusEffects[target];
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
            spendMpWithoutDamage(h, skill);
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

    auto undoAction = [this, h, target, targetSnapshot, casterSnapshot, previousEffects]() {
        targetSnapshot->restore(*target);
        if (target != h) casterSnapshot->restore(*h);
        m_statusEffects[target] = *previousEffects;
        syncHealth(target);
        syncHealth(h);
    };
    return queuePlayerCommand(std::make_unique<UseSkillCommand>(executeAction, undoAction));
}

void GameEngine::confirmTurn()
{
    m_commandStack.clear();    // chot hanh dong: het kha nang Undo
    m_isSelectingSkill = false;
    m_isSelectingItem = false;
    Character* actor = m_currentCharacter;
    if (actor) {
        actor->onTurnEnd();
        syncHealth(actor);
        if (actor->isAlive() && actor->modifiers().consumeExtraTurn()) {
            m_battleLog = actor->getName() + " duoc them 1 luot!";
            m_battlePhase = BattlePhase::PLAYER_TURN;
            return;
        }
    }
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

    // SLOW must be calculated against the original speed, never stacked each turn.
    auto baseSpeed = m_baseSpeed.find(c);
    if (baseSpeed != m_baseSpeed.end()) c->setSpeed(baseSpeed->second);
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
    m_isSelectingItem = false;

    // Gioi han vong lap: phong truong hop moi nhan vat deu mat luot lien tuc
    for (int guard = 0; guard < 64; ++guard)
    {
        checkBattleCondition();
        if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
            return;

        m_currentCharacter = m_turnManager.getNextTurn();
        if (!m_currentCharacter)
            continue;

        if (!beginTurn(m_currentCharacter)) {
            if (m_currentCharacter->isAlive()) {
                m_currentCharacter->onTurnEnd();
                syncHealth(m_currentCharacter);
            }
            continue;
        }

        m_battlePhase = isHero(m_currentCharacter) ? BattlePhase::PLAYER_TURN : BattlePhase::AI_THINKING;
        if (isHero(m_currentCharacter)) refreshCurrentSkills();
        return;
    }
    m_battlePhase = BattlePhase::PLAYER_TURN;
}

void GameEngine::executeAiTurn(Character* aiChar)
{
    std::vector<Character*> targets;
    for (const auto& member : m_party.getMembers())
        if (member->isAlive()) targets.push_back(member.get());
    Character* h = CombatAI::selectTarget(targets);
    if (!aiChar || !aiChar->isAlive() || !h) return;

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

    aiChar->onTurnEnd();
    syncHealth(aiChar);
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

    if (m_party.isDefeated())
    {
        m_stateMachine.changeState(GameStateMachine::State::GAME_OVER);
        return;
    }

    for (size_t i = m_party.size(); i < m_combatants.size(); ++i)
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

void GameEngine::spendMpWithoutDamage(Character* caster, const Skill& skill)
{
    // Non-damaging skills still consume MP, without simulating an attack.
    if (caster) caster->consumeMp(skill.m_mpCost);
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
    if (m_currentCharacter && isHero(m_currentCharacter)) return m_currentCharacter;
    return m_party.getLeader();
}

bool GameEngine::isHero(const Character* character) const
{
    if (!character) return false;
    for (const auto& member : m_party.getMembers())
        if (member.get() == character) return true;
    return false;
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
        const Color tint = (i < m_party.size()) ? WHITE : elementColor(m_combatants[i]->getElement());
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
        m_ui.drawCommandHelp(TextFormat("[A] Tan Cong | [S] Ky Nang | [I] Vat pham x%d | [TAB] Doi Muc Tieu | [ESC] Menu",
                                        static_cast<int>(availableItems().size())));
    m_ui.drawBattleLog("BATTLE LOG: " + m_battleLog);

    // ----- Menu ky nang (popup giua man hinh, ve cuoi cung de nam tren cung) -----
    if (m_isSelectingSkill)
        m_ui.drawSkillMenu(m_heroSkills, m_selectedSkillIndex, hero() ? hero()->getCurrentMp() : 0);
    if (m_isSelectingItem) {
        const auto items = availableItems();
        DrawRectangle(340, 160, 600, 390, Color{20, 25, 40, 235});
        DrawText("TUI DO: [1-9] Chon  [F1-F3] Dong doi  [ENTER] Dung", 355, 177, 18, WHITE);
        if (m_selectedAlly) DrawText(TextFormat("Muc tieu: %s", m_selectedAlly->getName().c_str()), 355, 209, 18, YELLOW);
        for (std::size_t i = 0; i < items.size() && i < 9; ++i)
            DrawText(TextFormat("%d. %s", static_cast<int>(i + 1), items[i]->getName().c_str()),
                     365, 246 + static_cast<int>(i) * 28, 18,
                     static_cast<int>(i) == m_selectedItemIndex ? YELLOW : RAYWHITE);
    }
}
