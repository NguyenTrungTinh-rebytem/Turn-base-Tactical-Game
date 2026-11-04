#include "core/GameEngine.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/FontManager.h"
#include "graphics/Strings.h"
#include "graphics/UiTheme.h"
#include "graphics/UiBlur.h"
#include "graphics/UiCursor.h"
#include "graphics/UiDevGallery.h"
#include "graphics/UiIcons.h"
#include "graphics/UiInput.h"
#include "graphics/UiShapes.h"
#include "graphics/UiWidgets.h"
#include "graphics/RaylibScope.h"
#include <string>
#include <unordered_map>
#include "core/AttackCommand.h"
#include "core/UseSkillCommand.h"
#include "core/UseItemCommand.h"
#include "core/CharacterSnapshot.h"
#include "entities/CharacterFactory.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "data/HeroPresetConfig.h"
#include "data/CharacterRules.h"
#include "graphics/SceneShader.h"
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
    
    // Tuong nu (giong hero_<id> trong heroes.csv) -> dung tieng nu. Con lai dung tieng nam.
    bool isFemaleHeroName(const std::string& heroName)
    {
        std::string id;
        for (char ch : heroName) id += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        while (!id.empty() && (std::isdigit(static_cast<unsigned char>(id.back())) || id.back() == ' ')) id.pop_back();
        return id == "lyra" || id == "sylwen" || id == "kael";
    }
    int itemCategoryIndex(ItemCategory c)
    {
        switch (c) {
            case ItemCategory::CONSUMABLE: return 0;
            case ItemCategory::BOOSTER:    return 1;
            case ItemCategory::HELD:       return 2;
        }
        return 0;
    }
    constexpr float GOLD_POPUP_LIFE = 1.4f;

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
        { "M_GOBLIN_KIM_E", "assets/images/goblin_sheet.png" },
        { "M_WOLF_MOC_E", "assets/images/wolf_sheet.png" },
        { "M_SLIME_THUY_E", "assets/images/slime_sheet.png" },
        { "M_IMP_HOA_E", "assets/images/imp_sheet.png" },
        { "M_GOLEM_THO_E", "assets/images/golem_sheet.png" },
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
    constexpr int   STARTER_POTIONS = 3;   // bo do khoi dau (con lai phai mua o Shop)
    constexpr int   STARTER_ENERGY = 1;
    constexpr int   STAGE1_ENEMY_COUNT = 3;
    constexpr float WAVE_CLEAR_RECOVER_RATE = 0.2f; // sau moi wave: hoi 20% HP va 20% MP (nguoi con song)
    const char* const POTION_NAME = "ITM_HP_MINOR";
    const char* skillTypeText(SkillType t) {
    switch (t) {
        case SkillType::DAMAGE: return "Sát thương";
        case SkillType::HEAL:   return "Hồi máu";
        case SkillType::BUFF:   return "Tăng cường";
        case SkillType::DEBUFF: return "Giảm sức mạnh";
    }
    return "";
}

std::string effectText(EffectType e, int turns) {
    const char* name = "";
    switch (e) {
        case EffectType::ATK_BOOST: name = "Tăng công";  break;
        case EffectType::DEF_BOOST: name = "Tăng thủ";   break;
        case EffectType::REGEN:     name = "Hồi máu";    break;
        case EffectType::STUN:      name = "Choáng";     break;
        case EffectType::BURN:      name = "Bỏng";       break;
        case EffectType::SLOW:      name = "Chậm";       break;
        case EffectType::WEAKNESS:  name = "Suy yếu";    break;
        case EffectType::NONE:      return "";
    }
    return std::string(name) + " " + std::to_string(turns) + " lượt";
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
        case StatusType::ATK_BOOST: return "CÔNG+";
        case StatusType::DEF_BOOST: return "THỦ+";
        case StatusType::REGEN:     return "HỒI";
        case StatusType::STUN:      return "CHOÁNG";
        case StatusType::BURN:      return "BỎNG";
        case StatusType::SLOW:      return "CHẬM";
        case StatusType::WEAKNESS:  return "YẾU";
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

    Rectangle dialogBounds()
    {
        return Rectangle{ 330.0f, 230.0f, 620.0f, 260.0f };
    }

    Rectangle dialogButtonBounds(int index, int count)
    {
        const Rectangle dialog = dialogBounds();
        const float width = count == 3 ? 170.0f : 210.0f;
        const float gap = count == 3 ? 16.0f : 20.0f;
        const float total = count * width + (count - 1) * gap;
        const float x = dialog.x + (dialog.width - total) * 0.5f +
                        index * (width + gap);
        return Rectangle{ x, dialog.y + 188.0f, width, 48.0f };
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, Strings::gameTitle);
    SetWindowTitle(Strings::gameTitle);
    SetWindowMinSize(960, 540);
    if (!UiCanvas::initialize())
        throw std::runtime_error("Unable to create the 1920x1080 UI canvas");
    UiCursor::initialize();
    if (!FontManager::initialize(resolvePath("assets/fonts/NotoSans-Vietnamese.ttf"))) {
        UiCursor::shutdown();
        UiCanvas::shutdown();
        CloseWindow();
        throw std::runtime_error("Unable to load the bundled Vietnamese font");
    }
    UiShapes::initialize(resolvePath("assets/shaders/ui_shapes.fs").c_str());
    UiBlur::initialize(resolvePath("assets/shaders/ui_blur.fs").c_str());
    UiIcons::initialize(resolvePath("assets/ui/icons").c_str());
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);   // ESC do game tu xu ly (Undo / ve Menu), khong thoat thang
    m_audio.init();                                          // khong co loa -> game van chay, chi im lang
    m_audio.loadAll(resolvePath("assets/audio"));
    m_sceneShader.init(UiCanvas::width(), UiCanvas::height()); // loi shader -> tu tat, game ve binh thuong

    try {
        m_gameData.loadAll(resolvePath("data"));
        m_dataLoaded = true;
        m_battleLog = "Dữ liệu CSV đã tải thành công!";
    }
    catch (const std::exception& e) {
        m_dataLoaded = false;
        m_battleLog = std::string("Lỗi tải dữ liệu: ") + e.what();
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
    {
        // Gia ban + cau hinh tien roi (data/shop.csv, data/economy.csv) va icon vat pham (assets/images/items/<id>.png)
        m_shopConfig.load(resolvePath("data"));
        if (m_itemsLoaded) {
            if (m_shopConfig.getEntries().empty()) {        // thieu shop.csv -> ban het, gia theo nhom
                std::vector<ShopEntry> entries;
                for (const ItemData& item : m_itemDatabase.getItems())
                    entries.push_back({ item.m_id, item.m_category == ItemCategory::HELD ? 140
                                                 : item.m_category == ItemCategory::BOOSTER ? 60 : 40 });
                m_shopConfig.setEntries(entries);
            }
            for (const ItemData& item : m_itemDatabase.getItems())
                m_itemIcons.load(item.m_id, resolvePath("assets/images/items/" + item.m_id + ".png"),
                                 itemCategoryIndex(item.m_category));
        }
        m_prepMenu.setItemIcons(&m_itemIcons);
        m_shop.setIcons(&m_itemIcons);
    }
    m_startMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));   // thieu anh -> nen thu tuc
    m_heroConfig = { HERO_NAME, HERO_RACE, HERO_CLASS, HERO_ELEMENT };
    m_partyConfigs.clear();
    m_heroCreate.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_waveSelect.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_prepMenu.loadBackground(resolvePath("assets/images/bg_menu.png"));
    m_heroCreate.loadPreviewSprite(resolvePath(SPRITE_SHEET_PATH), FRAME_W, FRAME_H);
    setupHeroCreateScreen();
    m_combatSubject.addObserver(this);
    {
        const bool bossBg = isBossStage(m_stage);
        m_battleMap.setStage(bossBg ? StageId::STAGE_2 : StageId::STAGE_1,
                             resolvePath(bossBg ? "assets/images/bg_wave2.png"
                                                : "assets/images/bg_wave1.png"));
    }
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
    m_actedThisRound.clear();
    m_startMenu.unloadAll();
    m_heroCreate.unloadAll();
    m_waveSelect.unloadAll();
    m_prepMenu.unloadAll();
    m_shop.unloadAll();
    m_battleMap.unloadAll();
    m_itemIcons.unloadAll();
    UiIcons::shutdown();
    UiBlur::shutdown();
    UiShapes::shutdown();
    m_sceneShader.unload();
    FontManager::shutdown();
    UiCanvas::shutdown();
    UiCursor::shutdown();
    m_audio.unloadAll();                 // dong thiet bi am thanh TRUOC CloseWindow
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
        m_isSelectingTarget = false;
        m_battlePhase = BattlePhase::PLAYER_TURN;
        m_battleLog = "Đã hoàn tác hành động vừa rồi! Hãy chọn lệnh khác.";
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

int GameEngine::finalStage()
{
    if (!m_dataLoaded) return 1;
    if (m_finalStageCache > 0) return m_finalStageCache;
    // Wave cuoi = wave lon nhat co quai trong data/monsters.csv (3 wave: 2 thuong + Boss)
    int last = 1;
    for (int s = 1; s <= 10; ++s)
    {
        try
        {
            if (m_gameData.getMonstersByStage(s).empty()) break;
            last = s;
        }
        catch (const std::exception&) { break; }
    }
    m_finalStageCache = last;
    return last;
}

bool GameEngine::isBossStage(int stage)
{
    return m_dataLoaded && waveHasBoss(stage);
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
    if (!character) {
        std::cerr << "[BattleUI] initialization skipped a null combatant\n";
        return;
    }

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
    if (m_views.size() != m_combatants.size()) {
        std::cerr << "[BattleUI] layout mismatch: combatants=" << m_combatants.size()
                  << " views=" << m_views.size() << '\n';
    }
    if (m_views.empty() || members == 0) {
        std::cerr << "[BattleUI] layout skipped: party=" << members
                  << " views=" << m_views.size() << '\n';
        return;
    }

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
    std::vector<Character*> alive;
    for (Character* c : m_turnManager.getCharacters())
        if (c && c->isAlive()) alive.push_back(c);

    // Cùng quy tắc với priority_queue: tốc độ cao đi trước
    std::stable_sort(alive.begin(), alive.end(), [](const Character* a, const Character* b) {
        return a->getStats().m_spd > b->getStats().m_spd;
    });

    const auto acted = [this](const Character* c) {
        return std::find(m_actedThisRound.begin(), m_actedThisRound.end(), c) !=
               m_actedThisRound.end();
    };

    std::vector<Character*> order;
    if (m_currentCharacter && m_currentCharacter->isAlive()) order.push_back(m_currentCharacter);
    for (Character* c : alive)                       // chưa đi trong vòng này
        if (c != m_currentCharacter && !acted(c)) order.push_back(c);
    for (Character* c : alive)                       // đã đi => xếp sang vòng sau
        if (c != m_currentCharacter && acted(c)) order.push_back(c);
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
        w.m_title = TextFormat(Strings::stage, stage);

        const std::vector<MonsterData> monsters = m_gameData.getMonstersByStage(stage);
        for (const MonsterData& m : monsters)
        {
            w.m_enemies.push_back({ Strings::localizedName(m.m_name), m.m_element,
                                    m.m_stats.m_hp, m.m_isBoss });
            if (m.m_isBoss) w.m_isBoss = true;
        }

        // Wave thuong chi xuat hien ngau nhien STAGE1_ENEMY_COUNT quai trong so quai co the gap (xem spawnEnemies)
        const int appear = (!w.m_isBoss) ? std::min(static_cast<int>(monsters.size()), STAGE1_ENEMY_COUNT)
                                        : static_cast<int>(monsters.size());
        w.m_description = w.m_isBoss ? std::string("Trận chiến với trùm")
                                     : std::string("Xuất hiện ngẫu nhiên ") +
                                           std::to_string(appear) + " quái";
        w.m_locked = !m_waveManager.isWaveUnlocked(stage);
        w.m_cleared = static_cast<size_t>(stage) < m_wavesCleared.size() && m_wavesCleared[static_cast<size_t>(stage)];
        waves.push_back(std::move(w));
    }

    m_waveSelect.setWaves(waves);
    m_waveSelect.setHeroSummary(Strings::localizedName(m_heroConfig.m_name) + " - " +
        Strings::localizedName(m_heroConfig.m_race) + " · " +
        Strings::localizedName(m_heroConfig.m_class));
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
    info.m_hero.m_portraitPath = resolvePath(heroSpritePath(m_heroConfig.m_name));
    info.m_hero.m_element = m_heroConfig.m_element;
    info.m_hero.m_stats = Stats{ 120, 50, 25, 20, 5, 8, 15 };   // du phong
    info.m_teammateSlots = static_cast<int>(Party::MAX_SIZE) - static_cast<int>(m_partyConfigs.size());
    for (const HeroConfig& config : m_partyConfigs) {
        PrepHeroInfo member;
        member.m_name = config.m_name;
        member.m_race = config.m_race;
        member.m_class = config.m_class;
        member.m_portraitPath = resolvePath(heroSpritePath(config.m_name));
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
    // Tab 3: Tui do (giua 2 wave: tui that cua doi; luc bat dau: bo do khoi dau)
    if (m_itemsLoaded) {
        if (!m_party.isEmpty()) {
            std::vector<std::string> seen;
            for (const ItemPtr& item : m_party.getInventory().getItems()) {
                if (!item || std::find(seen.begin(), seen.end(), item->getId()) != seen.end()) continue;
                seen.push_back(item->getId());
                info.m_items.push_back({ item->getNameVi(), static_cast<int>(m_party.countItem(item->getId())), item->getId() });
            }
            for (const ItemPtr& gear : m_equippedGear)
                if (gear) info.m_items.push_back({ gear->getNameVi() + " (đang đeo)", 1, gear->getId() });
        } else {
            info.m_items.push_back({ "Bình hồi máu nhỏ", STARTER_POTIONS, "ITM_HP_MINOR" });
            info.m_items.push_back({ "Năng lượng", STARTER_ENERGY, "ITM_ENERGY" });
        }
    }

    // Tab 4: Thong tin cong
    info.m_gate.m_title = TextFormat(Strings::stage, wave);
    info.m_gate.m_deadlineTurns = deadlineTurnsForStage(wave);
    for (const MonsterData& m : m_gameData.getMonstersByStage(wave))
    {
        if (m.m_isBoss) {
            info.m_gate.m_hasBoss = true;
            info.m_gate.m_bossName = Strings::localizedName(m.m_name);
            info.m_gate.m_bossElement = m.m_element;
            info.m_gate.m_bossHp = m.m_stats.m_hp;
        } else {
            info.m_gate.m_enemies.push_back({ Strings::localizedName(m.m_name), m.m_element });
        }
    }
    info.m_gate.m_description = info.m_gate.m_hasBoss
        ? std::string("Trận chiến với trùm\nHạ gác cổng để tiến lên")
        : std::string("Quét sạch quái canh cổng\nXuất hiện ngẫu nhiên");

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
        ? std::string(Strings::heroLead)
        : std::string("CHỌN ĐỒNG ĐỘI ") + std::to_string(m_editingPartySlot + 1) + "/3");
    m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
}

void GameEngine::createHero()
{
    m_heroSkills.clear();
    m_party.clear();
    for (ItemPtr& gear : m_equippedGear) gear.reset();
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
    // Bo do khoi dau. Muon them do: mua o Shop (mo truoc wave Boss) bang tien roi tu quai.
    if (!m_itemsLoaded) return;
    auto seed = [this](const char* id, int count) {
        if (!m_itemDatabase.findItem(id)) return;
        for (int i = 0; i < count; ++i)
            m_party.addItem(ItemFactory::createItem(m_itemDatabase, id));
    };
    seed(POTION_NAME, STARTER_POTIONS);
    seed("ITM_ENERGY", STARTER_ENERGY);
}

void GameEngine::spawnEnemies(int stage)
{
    if (m_dataLoaded)
    {
        std::vector<MonsterData> monsters = m_gameData.getMonstersByStage(stage);
        const bool hasBoss = std::any_of(monsters.begin(), monsters.end(),
                                         [](const MonsterData& md) { return md.m_isBoss; });
        if (!hasBoss)
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
    std::cerr << "[BattleUI] battle initialization enter: stage=" << stage
              << " party=" << m_party.size()
              << " combatants-before-reset=" << m_combatants.size()
              << " views-before-reset=" << m_views.size() << '\n';
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
    m_goldPaid.clear();      // quai moi co the trung dia chi voi quai cu da bi huy
    m_ui.unloadAll();
    m_isSelectingSkill = false;
    m_isSelectingItem = false;
    m_isSelectingTarget = false;
    m_battleLogOpen = false;
    m_aiTurnTimer = 0.0f;

    for (const auto& member : m_party.getMembers()) {
        Character* h = member.get();
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
    if (m_combatants.size() != m_views.size()) {
        std::cerr << "[BattleUI] initialization mismatch after layout: combatants="
                  << m_combatants.size() << " views=" << m_views.size() << '\n';
    }
    std::cerr << "[BattleUI] initialization complete: combatants=" << m_combatants.size()
              << " views=" << m_views.size()
              << " selectedTarget=" << (m_selectedTarget ? "set" : "none") << '\n';
    {
        const bool bossBg = isBossStage(m_stage);
        m_battleMap.setStage(bossBg ? StageId::STAGE_2 : StageId::STAGE_1,
                             resolvePath(bossBg ? "assets/images/bg_wave2.png"
                                                : "assets/images/bg_wave1.png"));
    }

    m_currentCharacter = nullptr;   // update() se goi advanceTurn() de lay luot dau tien
    m_battlePhase = BattlePhase::AI_THINKING;
    m_battleLog = std::string("MÀN ") + std::to_string(stage) + " bắt đầu!";
}

void GameEngine::beginRun(int startStage)
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
    m_goldPaid.clear();
    m_goldPopups.clear();
    m_battleLogOpen = false;
    m_shopOpen = false;
    m_resumeRun = false;

    createHero();
    seedItems();

    // Tien khoi dau; vao thang wave cao thi duoc tang bu cho cac wave bi bo qua
    const EconomyConfig& eco = m_shopConfig.getEconomy();
    m_gold = eco.m_startGold + std::max(0, startStage - 1) * eco.m_skipWaveGold;
}

void GameEngine::startNewGame(int startStage, bool preservePreparedRun)
{
    if (!preservePreparedRun) beginRun(startStage);
    loadStage(startStage);
    advanceTurn();

    m_battleLog = std::string("Trận đấu bắt đầu! Đến lượt: ") +
        Strings::localizedName(m_currentCharacter ? m_currentCharacter->getName() : std::string{});
}

bool GameEngine::waveHasBoss(int wave)
{
    if (!m_dataLoaded) return false;
    for (const MonsterData& m : m_gameData.getMonstersByStage(wave))
        if (m.m_isBoss) return true;
    return false;
}

void GameEngine::recoverPartyAfterWave()
{
    // Hoi 20% HP + 20% MP toi da cho nguoi con song. Nguoi da guc phai dung Revive.
    int hpTotal = 0, mpTotal = 0;
    for (const auto& member : m_party.getMembers())
    {
        Character* h = member.get();
        if (!h->isAlive()) continue;

        const int hpAdd = static_cast<int>(static_cast<float>(h->getMaxHp()) * WAVE_CLEAR_RECOVER_RATE);
        const int hpBefore = h->getCurrentHp();
        h->heal(hpAdd);
        hpTotal += h->getCurrentHp() - hpBefore;

        const int maxMp = h->getStats().m_mp;
        const int mpAdd = std::min(static_cast<int>(static_cast<float>(maxMp) * WAVE_CLEAR_RECOVER_RATE),
                                   std::max(0, maxMp - h->getCurrentMp()));
        if (mpAdd > 0) h->consumeMp(-mpAdd);     // tieu hao am = hoi MP (da gioi han khong vuot toi da)
        mpTotal += mpAdd;
    }
    m_battleLog += std::string("  [Hồi sau màn: +") + std::to_string(hpTotal) +
        " HP, +" + std::to_string(mpTotal) + " MP (20%)]";
}

void GameEngine::enterBossBreak(int nextWave)
{
    // Giu nguyen doi hinh / HP / tui do; chi mo man Chuan bi (co Shop) roi vao wave tiep
    m_resumeRun = true;
    m_shopOpen = false;
    m_isSelectingSkill = false;
    m_isSelectingItem = false;
    m_commandStack.clear();
    openPrepMenu(nextWave);
    m_battleLog = "Chuẩn bị cho màn trùm! Mở cửa hàng để mua đồ và trang bị.";
}

void GameEngine::beginBattleAfterBreak()
{
    m_resumeRun = false;
    m_shopOpen = false;
    m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
    loadStage(m_pendingWave);
    advanceTurn();
    m_battleLog = std::string("Trận đấu bắt đầu! Đến lượt: ") +
        Strings::localizedName(m_currentCharacter ? m_currentCharacter->getName() : std::string{});
}

// =====================================================================
//  TIEN ROI + CUA HANG
// =====================================================================

int GameEngine::computeGoldDrop(const Character* enemy)
{
    if (!enemy) return 0;
    const EconomyConfig& eco = m_shopConfig.getEconomy();
    int gold = eco.m_enemyBaseGold;
    if (eco.m_enemyHpDivisor > 0) gold += enemy->getStats().m_hp / eco.m_enemyHpDivisor;
    if (dynamic_cast<const Boss*>(enemy)) gold *= eco.m_bossMultiplier;
    if (eco.m_variancePercent > 0) {
        std::uniform_int_distribution<int> dist(100 - eco.m_variancePercent, 100 + eco.m_variancePercent);
        gold = gold * dist(m_rng) / 100;
    }
    return std::max(1, gold);
}

void GameEngine::addGoldPopup(Vector2 pos, int amount)
{
    GoldPopup popup;
    popup.m_pos = pos;
    popup.m_amount = amount;
    popup.m_life = GOLD_POPUP_LIFE;
    m_goldPopups.push_back(popup);
}

void GameEngine::settleGold()
{
    // Chi tra tien khi hanh dong da chot (khong con Undo) -> Undo hoi sinh quai khong lam tien bi cong thua.
    int total = 0;
    for (std::size_t i = m_party.size(); i < m_combatants.size() && i < m_views.size(); ++i) {
        const Character* enemy = m_combatants[i].get();
        if (!enemy || enemy->isAlive()) continue;
        if (!m_goldPaid.insert(enemy).second) continue;
        const int drop = computeGoldDrop(enemy);
        total += drop;
        Vector2 pos = m_views[i]->getHeadPos();
        pos.y -= 30.0f;
        addGoldPopup(pos, drop);
    }
    if (total > 0) {
        m_gold += total;
        m_battleLog += std::string("  [+") + std::to_string(total) + " vàng]";
    }
}

int GameEngine::ownedCount(const std::string& itemId) const
{
    int count = static_cast<int>(m_party.countItem(itemId));
    for (const ItemPtr& gear : m_equippedGear)
        if (gear && gear->getId() == itemId) ++count;
    return count;
}

Rectangle GameEngine::shopButtonRect() const
{
    return Rectangle{ 40.0f, 640.0f, 250.0f, 52.0f };
}

void GameEngine::openShop()
{
    if (!m_resumeRun && m_party.isEmpty()) {
        beginRun(m_pendingWave);
        m_shopPreparedRun = true;
    }
    m_shopOpen = true;
    m_shop.open(0);
    refreshShop();
}

void GameEngine::closeShop()
{
    m_shopOpen = false;
    openPrepMenu(m_pendingWave);          // cap nhat lai tab Tui do cua man Chuan bi
}

void GameEngine::refreshShop()
{
    ShopInfo info;
    info.m_gold = m_gold;

    for (const ShopEntry& entry : m_shopConfig.getEntries()) {
        const ItemData* data = m_itemDatabase.findItem(entry.m_id);
        if (!data) continue;
        ShopStockInfo s;
        s.m_id = data->m_id;
        s.m_name = data->m_nameVi;
        try { s.m_description = ItemFactory::createItem(*data)->getDescription(); }
        catch (const std::exception&) { s.m_description = ""; }
        s.m_category = itemCategoryIndex(data->m_category);
        s.m_price = entry.m_price;
        s.m_owned = ownedCount(data->m_id);
        s.m_limit = data->m_maxCarry;
        s.m_effect = data->m_effect;
        s.m_value = data->m_value;
        s.m_value2 = data->m_value2;
        s.m_duration = data->m_duration;
        info.m_stock.push_back(s);
    }

    std::vector<std::string> seen;
    for (const ItemPtr& item : m_party.getInventory().getItems()) {
        if (!item || std::find(seen.begin(), seen.end(), item->getId()) != seen.end()) continue;
        seen.push_back(item->getId());
        ShopBagInfo b;
        b.m_id = item->getId();
        b.m_name = item->getNameVi();
        b.m_description = item->getDescription();
        b.m_category = itemCategoryIndex(item->getCategory());
        b.m_count = static_cast<int>(m_party.countItem(item->getId()));
        b.m_effect = item->getEffect();
        b.m_value = item->getValue();
        b.m_value2 = item->getData().m_value2;
        b.m_duration = item->getDuration();
        info.m_bag.push_back(b);
    }

    for (std::size_t i = 0; i < m_party.size(); ++i) {
        ShopMemberInfo m;
        m.m_name = m_party.getMember(i)->getName();
        m.m_portraitPath = resolvePath(heroSpritePath(m.m_name));
        if (i < Party::MAX_SIZE && m_equippedGear[i]) {
            m.m_gearId = m_equippedGear[i]->getId();
            m.m_gearName = m_equippedGear[i]->getNameVi();
            m.m_gearEffect = m_equippedGear[i]->getEffect();
            m.m_gearValue = m_equippedGear[i]->getValue();
            m.m_gearValue2 = m_equippedGear[i]->getData().m_value2;
            m.m_gearDuration = m_equippedGear[i]->getDuration();
        }
        info.m_members.push_back(m);
    }
    m_shop.setInfo(info);
}

void GameEngine::buyShopItem(int stockIndex)
{
    const auto& entries = m_shopConfig.getEntries();
    std::vector<const ShopEntry*> sold;
    for (const ShopEntry& e : entries)
        if (m_itemDatabase.findItem(e.m_id)) sold.push_back(&e);
    if (stockIndex < 0 || static_cast<std::size_t>(stockIndex) >= sold.size()) return;

    const ShopEntry& entry = *sold[static_cast<std::size_t>(stockIndex)];
    const ItemData& data = m_itemDatabase.getItem(entry.m_id);

    if (data.m_maxCarry > 0 && ownedCount(entry.m_id) >= data.m_maxCarry) {
        m_shop.setMessage("Bạn đã mang đủ số lượng " + data.m_nameVi + "!", false);
        return;
    }
    if (m_gold < entry.m_price) {
        m_shop.setMessage("Không đủ vàng để mua " + data.m_nameVi + "!", false);
        return;
    }
    if (!m_party.addItem(ItemFactory::createItem(data))) {
        m_shop.setMessage("Túi đồ đã đầy, không thể mua thêm " + data.m_nameVi + "!", false);
        return;
    }
    m_gold -= entry.m_price;
    m_audio.playSfx(SfxId::CLICK);
    m_shop.setMessage("Đã mua " + data.m_nameVi + " (-" +
                      std::to_string(entry.m_price) + " vàng)", true);
    refreshShop();
}

void GameEngine::equipFromBag(int bagIndex, int memberIndex)
{
    // Danh sach tui hien thi gom theo id (cung thu tu voi refreshShop)
    std::vector<std::string> ids;
    for (const ItemPtr& item : m_party.getInventory().getItems())
        if (item && std::find(ids.begin(), ids.end(), item->getId()) == ids.end()) ids.push_back(item->getId());
    if (bagIndex < 0 || static_cast<std::size_t>(bagIndex) >= ids.size()) {
        m_shop.setMessage("Chưa chọn vật phẩm để trang bị.", false);
        return;
    }
    if (memberIndex < 0 || static_cast<std::size_t>(memberIndex) >= m_party.size() ||
        static_cast<std::size_t>(memberIndex) >= Party::MAX_SIZE) return;

    const std::string& id = ids[static_cast<std::size_t>(bagIndex)];
    ItemPtr gear;
    for (const ItemPtr& item : m_party.getInventory().getItems())
        if (item && item->getId() == id) { gear = item; break; }
    if (!gear || gear->getCategory() != ItemCategory::HELD) {
        m_shop.setMessage("Chỉ trang bị mới đeo được cho tướng.", false);
        return;
    }
    const ItemPtr previous = m_equippedGear[memberIndex];
    if (!m_party.equipGear(static_cast<std::size_t>(memberIndex), gear)) {
        m_shop.setMessage("Không thể trang bị vật phẩm này.", false);
        return;
    }
    m_equippedGear[memberIndex] = gear;
    m_audio.playSfx(SfxId::CLICK);
    m_shop.setMessage(Strings::localizedName(
                          m_party.getMember(static_cast<std::size_t>(memberIndex))->getName()) +
                      " đã đeo " + gear->getNameVi() +
                      (previous ? " (cất " + previous->getNameVi() + " vào túi)" : ""), true);
    refreshShop();
}

void GameEngine::unequipMember(int memberIndex)
{
    if (memberIndex < 0 || static_cast<std::size_t>(memberIndex) >= m_party.size() ||
        static_cast<std::size_t>(memberIndex) >= Party::MAX_SIZE) return;
    const ItemPtr gear = m_equippedGear[memberIndex];
    if (!gear || !m_party.unequipGear(static_cast<std::size_t>(memberIndex))) {
        m_shop.setMessage("Tướng này chưa đeo trang bị.", false);
        return;
    }
    m_equippedGear[memberIndex].reset();
    m_audio.playSfx(SfxId::CLICK);
    m_shop.setMessage("Đã tháo " + gear->getNameVi() + " vào túi.", true);
    refreshShop();
}

void GameEngine::handleShopInput()
{
    switch (m_shop.handleInput())
    {
    case ShopAction::CLOSE:   closeShop(); break;
    case ShopAction::BUY:     buyShopItem(m_shop.getStockIndex()); break;
    case ShopAction::EQUIP:   equipFromBag(m_shop.getBagIndex(), m_shop.getMemberIndex()); break;
    case ShopAction::UNEQUIP: unequipMember(m_shop.getMemberIndex()); break;
    case ShopAction::NONE:    break;
    }
}

void GameEngine::drawGoldHud(float x, float y) const
{
    DrawRectangleRounded(Rectangle{ x, y, 190.0f, 34.0f }, 0.3f, 6, Color{ 15, 15, 30, 215 });
    DrawRectangleLinesEx(Rectangle{ x, y, 190.0f, 34.0f }, 2.0f, Color{ 200, 170, 80, 255 });
    DrawCircle(static_cast<int>(x + 20), static_cast<int>(y + 17), 10.0f, Color{ 255, 205, 60, 255 });
    DrawCircleLines(static_cast<int>(x + 20), static_cast<int>(y + 17), 10.0f, Color{ 120, 80, 10, 255 });
    UiText::draw(TextFormat(Strings::gold, m_gold), static_cast<int>(x + 40), static_cast<int>(y + 7), 20, Color{ 255, 225, 120, 255 });
}

void GameEngine::drawPrepExtras() const
{
    drawGoldHud(1060.0f, 22.0f);
}

bool GameEngine::handleDialogInput()
{
    const GameStateMachine::State state = m_stateMachine.getState();
    const bool resultDialog = state == GameStateMachine::State::VICTORY ||
                              state == GameStateMachine::State::GAME_OVER;
    if (!m_paused && !resultDialog) return false;

    if (IsKeyPressed(KEY_ESCAPE) && m_paused) {
        m_paused = false;
        return true;
    }

    const int count = state == GameStateMachine::State::GAME_OVER ? 2 : 3;
    m_dialogSelected = std::max(0, std::min(m_dialogSelected, count - 1));
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        m_dialogSelected = (m_dialogSelected + count - 1) % count;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        m_dialogSelected = (m_dialogSelected + 1) % count;

    int activated = -1;
    const Vector2 mouse = UiCanvas::mousePosition();
    for (int i = 0; i < count; ++i) {
        if (CheckCollisionPointRec(mouse, dialogButtonBounds(i, count))) {
            m_dialogSelected = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) activated = i;
        }
    }
    if (activated < 0 && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
                          IsKeyPressed(KEY_SPACE)))
        activated = m_dialogSelected;
    if (activated < 0) return true;

    if (m_paused) {
        if (activated == 0) {
            m_paused = false;
        } else if (activated == 1) {
            m_paused = false;
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
            startNewGame(m_stage);
        } else {
            m_paused = false;
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        }
        return true;
    }

    if (state == GameStateMachine::State::GAME_OVER) {
        if (activated == 0) {
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
            startNewGame(m_stage);
        } else {
            m_stateMachine.changeState(GameStateMachine::State::MENU);
        }
    } else if (activated == 1) {
        m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
        startNewGame(m_stage);
    } else {
        m_stateMachine.changeState(GameStateMachine::State::MENU);
    }
    return true;
}

void GameEngine::drawDialog(const std::string& title, const std::string& body,
                            const std::vector<std::string>& buttons) const
{
    if (buttons.empty()) return;
    UiBlur::draw(Rectangle{ 0.0f, 0.0f, 1280.0f, 720.0f }, 0.0f,
                 Color{ 5, 6, 16, 185 });
    const Rectangle dialog = dialogBounds();
    UiShapes::roundedRect(dialog, 18.0f, Color{ 18, 16, 38, 240 },
                          Color{ 200, 170, 80, 255 }, 2.0f);
    UiText::draw(title, static_cast<int>(dialog.x + 34.0f),
                 static_cast<int>(dialog.y + 28.0f), 30, Color{ 255, 214, 96, 255 });
    UiText::draw(body, static_cast<int>(dialog.x + 34.0f),
                 static_cast<int>(dialog.y + 82.0f), 18, Color{ 230, 226, 242, 255 });

    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        const Rectangle bounds = dialogButtonBounds(i, static_cast<int>(buttons.size()));
        const bool selected = i == m_dialogSelected;
        const bool primary = i == 0;
        UiShapes::roundedRect(bounds, 9.0f,
            selected || primary ? Color{ 70, 50, 18, 245 } : Color{ 30, 27, 55, 245 },
            selected ? Color{ 255, 214, 96, 255 } : Color{ 120, 108, 146, 255 },
            selected ? 2.5f : 1.5f);
        const int fontSize = buttons[i].size() > 15 ? 16 : 19;
        const int textWidth = UiText::measure(buttons[i], fontSize);
        UiText::draw(buttons[i],
            static_cast<int>(bounds.x + (bounds.width - textWidth) * 0.5f),
            static_cast<int>(bounds.y + (bounds.height - fontSize) * 0.5f),
            fontSize, Color{ 245, 242, 252, 255 });
    }
}

// =====================================================================
//  AM THANH
// =====================================================================

void GameEngine::updateMusic()
{
    switch (m_stateMachine.getState())
    {
    case GameStateMachine::State::MENU:       m_audio.playMusic(MusicId::MENU);      break;
    case GameStateMachine::State::IN_BATTLE:  m_audio.playMusic(MusicId::BATTLE);    break;
    case GameStateMachine::State::GAME_OVER:  m_audio.playMusic(MusicId::GAME_OVER); break;
    default:                                  m_audio.playMusic(MusicId::AMBIENT);   break;   // chon tuong / wave / chuan bi / thang
    }
}

void GameEngine::playActionSfx(Character* character, AnimState state)
{
    if (!character) return;
    const bool attack = (state == AnimState::ATTACK);
    const bool hurt = (state == AnimState::TAKE_DAMAGE);
    if (!attack && !hurt) return;

    if (isHero(character)) {
        const bool female = isFemaleHeroName(character->getName());
        if (attack) m_audio.playSfx(female ? SfxId::HERO_FEMALE_ATTACK : SfxId::HERO_MALE_ATTACK);
        else        m_audio.playSfx(female ? SfxId::HERO_FEMALE_HURT : SfxId::HERO_MALE_HURT);
    } else if (dynamic_cast<const Boss*>(character)) {
        m_audio.playSfx(attack ? SfxId::DRAGON_ATTACK : SfxId::DRAGON_HURT);        // boss hien tai la Rong
    } else {
        m_audio.playSfx(attack ? SfxId::MONSTER_ATTACK : SfxId::MONSTER_HURT);
    }
}

void GameEngine::playDeathSfx(Character* character)
{
    if (!character) return;
    if (isHero(character))
        m_audio.playSfx(isFemaleHeroName(character->getName()) ? SfxId::HERO_FEMALE_DEATH : SfxId::HERO_MALE_DEATH);
    else if (dynamic_cast<const Boss*>(character))
        m_audio.playSfx(SfxId::DRAGON_HURT);        // chua co tieng chet rieng cho quai: dung tieng bi danh
    else
        m_audio.playSfx(SfxId::MONSTER_HURT);
}

// =====================================================================
//  SU KIEN / INPUT
// =====================================================================

void GameEngine::processEvents()
{
    if (UiDevGallery::handleInput())
        return;
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
    if (handleDialogInput())
        return;

    // Tieng click cho cac man hinh menu (chuot trai / ENTER / SPACE)
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE) &&
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)))
        m_audio.playSfx(SfxId::CLICK);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
        {
            if (m_battleLogOpen) {
                m_battleLogOpen = false;
            } else if (m_isSelectingSkill || m_isSelectingItem || m_isSelectingTarget) {
                m_isSelectingSkill = false;
                m_isSelectingItem = false;
                m_isSelectingTarget = false;
            } else if (m_battlePhase == BattlePhase::PLAYER_CONFIRM) {
                if (!cancelLastAction()) m_paused = true;
            } else {
                m_paused = true;
                m_dialogSelected = 0;
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
    m_heroCreate.setSlotLabel(Strings::heroLead);
    m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);   // ESC o man Chon Wave = ve Tao Hero
    }
    else if (m_stateMachine.is(GameStateMachine::State::PREP_MENU))
    {
        if (m_shopOpen) { /* ESC do ShopScreen xu ly */ }
        else if (!m_resumeRun) leavePrepMenu();     // giua 2 wave khong cho thoat ve menu bang ESC
    }
        else if (m_stateMachine.is(GameStateMachine::State::MENU)) {
            // Exiting from the start menu is available only through its confirmation dialog.
        }
    }
    if (m_paused)
        return;
    else if (IsKeyPressed(KEY_Z) && m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
    {
        cancelLastAction();
    }

    if (m_stateMachine.is(GameStateMachine::State::IN_BATTLE) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 position = UiCanvas::mousePosition();
        if (CheckCollisionPointRec(position, BattleUI::logButtonBounds()) ||
            (m_battleLogOpen && CheckCollisionPointRec(position, BattleUI::logPanelBounds()))) {
            handleBattleClick(position);
            return;
        }
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
        m_resumeRun = false;
        m_shopOpen = false;
        m_editingPartySlot = 0;
        m_selectingFromPrep = false;
        m_heroCreate.reset(m_heroConfig.m_name);
        m_heroCreate.setSlotLabel(Strings::heroLead);
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
        m_heroCreate.setSlotLabel(Strings::heroLead);
        m_stateMachine.changeState(GameStateMachine::State::CREATE_HERO);
        break;
    case WaveSelectAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::PREP_MENU:
    if (m_shopOpen) {
        handleShopInput();
        break;
    }
    if (m_itemsLoaded && IsKeyPressed(KEY_B)) {
        openShop();
        break;
    }
    if (!m_resumeRun && IsKeyPressed(KEY_T)) {
        openPartyMemberSelection(std::min(static_cast<int>(m_partyConfigs.size()),
                                          static_cast<int>(Party::MAX_SIZE) - 1));
        break;
    }
    switch (m_prepMenu.handleInput())
    {
    case PrepMenuAction::EDIT_MEMBER_1:
        if (!m_resumeRun) openPartyMemberSelection(0);
        break;
    case PrepMenuAction::EDIT_MEMBER_2:
        if (!m_resumeRun) openPartyMemberSelection(1);
        break;
    case PrepMenuAction::EDIT_MEMBER_3:
        if (!m_resumeRun) openPartyMemberSelection(2);
        break;
    case PrepMenuAction::START:
        if (m_resumeRun) {
            beginBattleAfterBreak();                      // tiep tuc chuyen di dang dang do
        } else if (m_itemsLoaded && m_pendingWave > 1 && waveHasBoss(m_pendingWave)) {
            if (!m_shopPreparedRun) beginRun(m_pendingWave);
            m_shopPreparedRun = false;
            m_resumeRun = true;
            openPrepMenu(m_pendingWave);
        } else {
            m_stateMachine.changeState(GameStateMachine::State::IN_BATTLE);
            startNewGame(m_pendingWave, m_shopPreparedRun);
            m_shopPreparedRun = false;
        }
        break;
    case PrepMenuAction::BACK:
        if (!m_resumeRun) leavePrepMenu();
        break;
    case PrepMenuAction::SHOP:
        openShop();
        break;
    case PrepMenuAction::NONE:
        break;
    }
    break;
    case GameStateMachine::State::IN_BATTLE:
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(UiCanvas::mousePosition(),
                                   BattleUI::buttonBounds(BattleUIButton::PAUSE))) {
            handleBattleClick(UiCanvas::mousePosition());
            break;
        }
        if (m_battlePhase == BattlePhase::PLAYER_TURN || m_battlePhase == BattlePhase::PLAYER_CONFIRM)
            processPlayerInput();
        break;

    case GameStateMachine::State::VICTORY:
    case GameStateMachine::State::GAME_OVER:
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
    m_isSelectingTarget = false;
    m_battleLogOpen = false;
    m_selectedItemIndex = 0;
    m_selectedAlly = hero();
}

void GameEngine::rejectTargetClick(const char* reason, Vector2 position) const
{
    std::cerr << "[battle] target click rejected: " << reason
              << " at (" << position.x << ", " << position.y << ")\n";
}

void GameEngine::handleBattleClick(Vector2 position)
{
    if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE)) {
        rejectTargetClick("battle is not active", position);
        return;
    }

    const bool listOpen = m_isSelectingSkill || m_isSelectingItem || m_isSelectingTarget;
    if (!listOpen && CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::PAUSE))) {
        m_paused = true;
        m_dialogSelected = 0;
        return;
    }

    if (CheckCollisionPointRec(position, BattleUI::logButtonBounds())) {
        m_battleLogOpen = !m_battleLogOpen;
        if (m_battleLogOpen) {
            m_isSelectingSkill = false;
            m_isSelectingItem = false;
        }
        return;
    }
    if (m_battleLogOpen && CheckCollisionPointRec(position, BattleUI::logPanelBounds()))
        return;

    if (m_battlePhase == BattlePhase::PLAYER_CONFIRM) {
        if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::END_TURN))) {
            confirmTurn();
        } else if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::UNDO))) {
            if (!cancelLastAction())
                rejectTargetClick("there is no pending action to undo", position);
        } else {
            rejectTargetClick("confirmation phase accepts only end-turn or undo controls", position);
        }
        return;
    }

    if (m_battlePhase != BattlePhase::PLAYER_TURN ||
        !m_currentCharacter || !isHero(m_currentCharacter)) {
        rejectTargetClick("it is not a hero action phase", position);
        return;
    }

    if (m_isSelectingItem) {
        if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::CANCEL_TARGET))) {
            m_isSelectingItem = false;
            return;
        }
        for (std::size_t i = 0; i < m_party.size(); ++i) {
            if (CheckCollisionPointRec(position, BattleUI::partyCardBounds(static_cast<int>(i)))) {
                m_selectedAlly = m_party.getMember(i);
                return;
            }
        }
        const auto items = availableItems();
        if (CheckCollisionPointRec(position, BattleUI::itemMenuBounds())) {
            for (std::size_t i = 0; i < items.size() && i < 9; ++i) {
                if (CheckCollisionPointRec(position, BattleUI::itemRowBounds(
                        static_cast<int>(i), static_cast<int>(items.size()), m_selectedItemIndex))) {
                    m_selectedItemIndex = static_cast<int>(i);
                    doUseItem();
                    return;
                }
            }
            rejectTargetClick("item menu has priority over battlefield clicks", position);
            return;
        }
        rejectTargetClick("item menu is active; select an item row first", position);
        return;
    }

    if (m_isSelectingTarget) {
        if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::CANCEL_TARGET))) {
            m_isSelectingTarget = false;
            return;
        }
        if (CheckCollisionPointRec(position, BattleUI::controlPanelBounds())) {
            rejectTargetClick("control panel has priority over combatant bounds", position);
            return;
        }

        std::size_t livingCandidates = 0;
        for (std::size_t i = m_party.size(); i < m_combatants.size(); ++i) {
            Character* candidate = m_combatants[i].get();
            if (!candidate || !candidate->isAlive() || isHero(candidate)) continue;
            ++livingCandidates;
            if (i >= m_views.size() || !m_views[i] || !m_views[i]->containsPoint(position)) continue;

            m_selectedTarget = candidate;
            m_isSelectingTarget = false;
            if (m_targetAction == TargetAction::BASIC_ATTACK) {
                doBasicAttack();
            } else if (castSkill(static_cast<std::size_t>(m_selectedSkillIndex))) {
                m_isSelectingSkill = false;
                m_battlePhase = BattlePhase::PLAYER_CONFIRM;
            }
            return;
        }

        rejectTargetClick(livingCandidates == 0
            ? "no living non-hero targets are available"
            : "click missed all living non-hero draw bounds", position);
        return;
    }

    if (m_isSelectingSkill &&
        CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::CANCEL_TARGET))) {
        m_isSelectingSkill = false;
        return;
    }

    if (m_isSelectingSkill) {
            for (std::size_t i = 0; i < m_heroSkills.size(); ++i) {
                if (!CheckCollisionPointRec(position, BattleUI::skillRowBounds(
                        static_cast<int>(i), static_cast<int>(m_heroSkills.size()), m_selectedSkillIndex)))
                    continue;
                m_selectedSkillIndex = static_cast<int>(i);
                const Skill& skill = m_heroSkills[i];
                const bool offensive = skill.m_type == SkillType::DAMAGE ||
                    (skill.m_type == SkillType::DEBUFF && skill.m_power > 0);
                if (offensive) {
                    m_isSelectingSkill = false;
                    m_targetAction = TargetAction::SKILL;
                    m_isSelectingTarget = true;
                    m_battleLogOpen = false;
                    const bool hasTarget = std::any_of(m_combatants.begin() + static_cast<std::ptrdiff_t>(
                            std::min(m_party.size(), m_combatants.size())), m_combatants.end(),
                        [this](const std::shared_ptr<Character>& character) {
                            return character && character->isAlive() && !isHero(character.get());
                        });
                    if (!hasTarget) {
                        m_isSelectingTarget = false;
                        rejectTargetClick("skill has no living non-hero targets", position);
                    }
                } else if (castSkill(i)) {
                    m_isSelectingSkill = false;
                    m_battlePhase = BattlePhase::PLAYER_CONFIRM;
                }
                return;
            }
            rejectTargetClick("skill menu is active; click a visible skill row", position);
            return;
        }
    if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::ATTACK))) {
        m_battleLogOpen = false;
        m_isSelectingSkill = false;
        m_isSelectingItem = false;
        m_targetAction = TargetAction::BASIC_ATTACK;
        m_isSelectingTarget = true;
        const bool hasTarget = std::any_of(m_combatants.begin() + static_cast<std::ptrdiff_t>(
                std::min(m_party.size(), m_combatants.size())), m_combatants.end(),
            [this](const std::shared_ptr<Character>& character) {
                return character && character->isAlive() && !isHero(character.get());
            });
        if (!hasTarget) {
            m_isSelectingTarget = false;
            rejectTargetClick("attack has no living non-hero targets", position);
        }
        return;
    }

    if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::SKILL))) {
        m_battleLogOpen = false;
        m_isSelectingTarget = false;
        m_isSelectingItem = false;
        m_isSelectingSkill = !m_isSelectingSkill;
        return;
    }

    if (CheckCollisionPointRec(position, BattleUI::buttonBounds(BattleUIButton::ITEM))) {
        openItemMenu();
        if (availableItems().empty()) {
            m_battleLog = Strings::noInventoryItems;
            m_isSelectingItem = false;
        }
        return;
    }

    

    rejectTargetClick("target mode is inactive; choose attack or an offensive skill first", position);
}

void GameEngine::processPlayerInput()
{
    if (!m_currentCharacter || !isHero(m_currentCharacter)) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        handleBattleClick(UiCanvas::mousePosition());
        return;
    }

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
            m_battleLog = Strings::noInventoryItems;
            m_isSelectingItem = false;
            return;
        }
        const KeyboardKey keys[] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE};
        for (std::size_t i = 0; i < items.size() && i < 9; ++i)
            if (IsKeyPressed(keys[i])) m_selectedItemIndex = static_cast<int>(i);
        if (IsKeyPressed(KEY_F1)) selectAlly(0);
        if (IsKeyPressed(KEY_F2)) selectAlly(1);
        if (IsKeyPressed(KEY_F3)) selectAlly(2);
        if (IsKeyPressed(KEY_DOWN))
            m_selectedItemIndex = std::max(0, std::min(static_cast<int>(items.size()) - 1, m_selectedItemIndex + 1));
        if (IsKeyPressed(KEY_UP))
            m_selectedItemIndex = std::max(0, m_selectedItemIndex - 1);
        const Vector2 hoverMouse = UiCanvas::mousePosition();
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            if (CheckCollisionPointRec(hoverMouse, BattleUI::itemRowBounds(
                    static_cast<int>(i), static_cast<int>(items.size()), m_selectedItemIndex)))
            {
                m_selectedItemIndex = static_cast<int>(i);
                break;
            }
        }
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
        if (IsKeyPressed(KEY_DOWN))
            m_selectedSkillIndex = std::max(0, std::min(static_cast<int>(m_heroSkills.size()) - 1, m_selectedSkillIndex + 1));
        if (IsKeyPressed(KEY_UP))
            m_selectedSkillIndex = std::max(0, m_selectedSkillIndex - 1);
        const Vector2 hoverMouse = UiCanvas::mousePosition();
        for (std::size_t i = 0; i < m_heroSkills.size(); ++i)
        {
            if (CheckCollisionPointRec(hoverMouse, BattleUI::skillRowBounds(
                    static_cast<int>(i), static_cast<int>(m_heroSkills.size()), m_selectedSkillIndex)))
            {
                m_selectedSkillIndex = static_cast<int>(i);
                break;
            }
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
    if (!queuePlayerCommand(std::make_unique<AttackCommand>(
            attacker, target, damageMultiplier(attacker, target))))
        return;

    const int dealt = hpBefore - target->getCurrentHp();
    playAnimation(attacker, AnimState::ATTACK);
    syncHealth(target);

    m_battleLog = Strings::localizedName(attacker->getName()) + " tấn công " +
        Strings::localizedName(target->getName()) + " gây " + std::to_string(dealt) +
        " sát thương!";
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
        m_battleLog = "Không thể dùng " + item->getNameVi() + " cho " +
            Strings::localizedName(target->getName());
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
    m_battleLog = Strings::localizedName(hero()->getName()) + " dùng " +
        item->getNameVi() + " cho " + Strings::localizedName(target->getName()) +
        " (+" + std::to_string(*amount) + ")";
    m_isSelectingItem = false;
    m_battlePhase = BattlePhase::PLAYER_CONFIRM;
}

bool GameEngine::castSkill(size_t skillIndex)
{
    if (skillIndex >= m_heroSkills.size()) return false;

    Character* h = hero();
    const Skill skill = m_heroSkills[skillIndex];   // ban sao: thao tac an toan trong lambda

    if (h->getCurrentMp() < skill.m_mpCost) {
        m_battleLog = "Không đủ MP để dùng " + Strings::localizedName(skill.m_name) + "!";
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
            m_battleLog = Strings::localizedName(h->getName()) + " dùng " +
                Strings::localizedName(skill.m_name) + " lên " +
                Strings::localizedName(target->getName()) + " gây " +
                std::to_string(hpBefore - target->getCurrentHp()) + " sát thương!";
        else if (skill.m_type == SkillType::HEAL)
            m_battleLog = Strings::localizedName(h->getName()) + " dùng " +
                Strings::localizedName(skill.m_name) + " hồi " +
                std::to_string(target->getCurrentHp() - hpBefore) + " HP!";
        else
            m_battleLog = Strings::localizedName(h->getName()) + " dùng " +
                Strings::localizedName(skill.m_name) + "!";
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
    m_isSelectingTarget = false;
    Character* actor = m_currentCharacter;
    if (actor) {
        actor->onTurnEnd();
        syncHealth(actor);
        if (actor->isAlive() && actor->modifiers().consumeExtraTurn()) {
            m_battleLog = Strings::localizedName(actor->getName()) + " được thêm một lượt!";
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
        m_battleLog = Strings::localizedName(c->getName()) + " gục ngã do hiệu ứng bỏng!";
        return false;
    }

    if (wasStunned) {
        m_battleLog = Strings::localizedName(c->getName()) + " bị choáng, mất lượt!";
        return false;
    }

    if (c->getCurrentHp() < hpBefore)
        m_battleLog = Strings::localizedName(c->getName()) + " chịu sát thương từ hiệu ứng!";
    return true;
}

void GameEngine::advanceTurn()
{
    m_aiTurnTimer = 0.0f;
    m_isSelectingSkill = false;
    m_isSelectingItem = false;
    m_isSelectingTarget = false;

    // Gioi han vong lap: phong truong hop moi nhan vat deu mat luot lien tuc
    for (int guard = 0; guard < 64; ++guard)
    {
        checkBattleCondition();
        if (!m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
            return;

        m_currentCharacter = m_turnManager.getNextTurn();
        if (!m_currentCharacter)
            continue;
            // Ghi nhớ ai đã đi; gặp lại người đã đi nghĩa là bắt đầu vòng mới
        if (std::find(m_actedThisRound.begin(), m_actedThisRound.end(), m_currentCharacter) !=
            m_actedThisRound.end())
            m_actedThisRound.clear();
        m_actedThisRound.push_back(m_currentCharacter);
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
    m_battleLog = Strings::localizedName(aiChar->getName()) + " tấn công " +
        Strings::localizedName(h->getName()) + " gây " + std::to_string(dealt) +
        " sát thương!";
}

void GameEngine::update(float deltaTime)
{   
    UiWidgets::update(deltaTime);
    m_audio.update(deltaTime);
    updateMusic();
    if (IsKeyPressed(KEY_F9)) m_sceneShader.setEnabled(!m_sceneShader.isEnabled());   // bat/tat shader canh
    if (m_paused) return;
    if (m_shopOpen) m_shop.update(deltaTime);
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
    if (m_battlePhase == BattlePhase::PLAYER_TURN && m_currentCharacter &&
        isHero(m_currentCharacter)) {
        const Vector2 mouse = UiCanvas::mousePosition();
        const float wheel = UiInput::wheel();
        if (wheel != 0.0f && m_isSelectingItem &&
            CheckCollisionPointRec(mouse, BattleUI::itemMenuBounds())) {
            const int maximum = static_cast<int>(availableItems().size()) - 1;
            m_selectedItemIndex = std::max(0, std::min(maximum,
                m_selectedItemIndex + (wheel < 0.0f ? 1 : -1)));
        } else if (wheel != 0.0f && m_isSelectingSkill &&
                   CheckCollisionPointRec(mouse, BattleUI::listPanelBounds(
                       static_cast<int>(m_heroSkills.size())))) {
            const int maximum = static_cast<int>(m_heroSkills.size()) - 1;
            m_selectedSkillIndex = std::max(0, std::min(maximum,
                m_selectedSkillIndex + (wheel < 0.0f ? 1 : -1)));
        }
    }
    for (GoldPopup& p : m_goldPopups) { p.m_life -= deltaTime; p.m_pos.y -= 35.0f * deltaTime; }
    m_goldPopups.erase(std::remove_if(m_goldPopups.begin(), m_goldPopups.end(),
                                      [](const GoldPopup& p) { return p.m_life <= 0.0f; }), m_goldPopups.end());

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
    settleGold();                      // tra tien cho quai chet (chi duoc goi khi khong con cho Undo)

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
    recoverPartyAfterWave();
    const int clearBonus = m_shopConfig.getEconomy().m_waveClearBonus;
    if (clearBonus > 0) {
        m_gold += clearBonus;
        m_battleLog += std::string("  [Qua màn: +") + std::to_string(clearBonus) + " vàng]";
    }
    if (m_stage < finalStage())
    {
        if (m_itemsLoaded && waveHasBoss(m_stage + 1))
            enterBossBreak(m_stage + 1);       // nghi truoc wave Boss: Shop + trang bi
        else
            loadStage(m_stage + 1);
    }
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
    playActionSfx(character, state);
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
    if (character) {
        m_battleLog += std::string("  >> ") + Strings::localizedName(character->getName()) +
            " gục ngã!";
        playDeathSfx(character);
    }
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
    RaylibScope::Drawing drawing;
    if (!drawing) return;
    UiCanvas::FrameGuard canvasFrame;
    if (!canvasFrame) {
        ClearBackground(Color{ 244, 244, 244, 255 });
        DrawRectangleRounded(Rectangle{ 28, 28, 1224, 110 }, 0.05f, 8, Color{ 15, 15, 30, 245 });
        UiText::draw("Không thể mở khung giao diện", 56, 58, 24, WHITE);
        std::cerr << "[BattleUI] render fallback: UiCanvas frame could not start\n";
        return;
    }
    UiWidgets::beginFrame();
    ClearBackground(RAYWHITE);
    if (UiDevGallery::visible()) {
        UiDevGallery::draw();
    } else switch (m_stateMachine.getState())
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
    drawPrepExtras();
    if (m_shopOpen) m_shop.draw();
    break;
    case GameStateMachine::State::IN_BATTLE:
        {
        static std::uint64_t battleDrawFrame = 0;
        ++battleDrawFrame;
        std::cerr << "[BattleUI] draw frame=" << battleDrawFrame
                  << " state=IN_BATTLE stage=" << m_stage
                  << " combatants=" << m_combatants.size()
                  << " views=" << m_views.size()
                  << " selectedTarget=" << (m_selectedTarget ? "set" : "none") << '\n';
        }
        if (m_sceneShader.isActive())
        {
            m_sceneShader.begin();            // nen + nhan vat -> RenderTexture
            m_battleMap.draw();
            renderCombatants();
            m_sceneShader.end();
            m_sceneShader.draw(isBossStage(m_stage) ? SceneMood::BOSS : SceneMood::NORMAL,
                               static_cast<float>(GetTime()));
        }
        else
        {
            m_battleMap.draw();
            renderCombatants();
        }
        renderBattleUI();                     // UI ve sau shader -> van sac net
        m_ui.drawFloatingTexts();
        break;

    case GameStateMachine::State::VICTORY:
        m_battleMap.draw();
        renderCombatants();
        break;

    case GameStateMachine::State::GAME_OVER:
        m_battleMap.draw();
        renderCombatants();
        break;
    }

    const GameStateMachine::State state = m_stateMachine.getState();
    if (m_paused) {
        drawDialog("Tạm dừng", "Trận đấu đang được tạm dừng.",
                   { "Tiếp tục", "Thử lại", "Về menu" });
    } else if (state == GameStateMachine::State::VICTORY) {
        drawDialog(Strings::victory, "Bạn đã hoàn thành toàn bộ hành trình.",
                   { "Tiếp tục", "Thử lại", "Về menu" });
    } else if (state == GameStateMachine::State::GAME_OVER) {
        drawDialog(Strings::gameOver, "Đội hình đã gục ngã. Hãy thử lại!",
                   { "Thử lại", "Về menu" });
    }

    UiWidgets::Toast();
    UiCursor::draw();
}

void GameEngine::renderCombatants()
{
    const std::size_t count = std::min(m_combatants.size(), m_views.size());
    if (m_combatants.empty() || m_views.empty()) {
        std::cerr << "[BattleUI] combatant draw skipped: combatants=" << m_combatants.size()
                  << " views=" << m_views.size() << '\n';
        return;
    }
    if (m_combatants.size() != m_views.size()) {
        std::cerr << "[BattleUI] drawing only matched entries: combatants="
                  << m_combatants.size() << " views=" << m_views.size() << '\n';
    }
    // 2.5D: ve hang xa truoc, hang gan sau (sap theo y cua chan)
    std::vector<size_t> order;
    order.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        if (m_combatants[i] && m_views[i]) order.push_back(i);
        else std::cerr << "[BattleUI] skipped null combatant/view at index=" << i << '\n';
    }
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return m_views[a]->getSlot().m_feetPos.y < m_views[b]->getSlot().m_feetPos.y;
    });

    // Bong do: nguon sang o tren-trai -> bong keo dai sang phai-xuong, mem o mep
    for (size_t i : order)
    {
        if (!m_combatants[i]->isAlive()) continue;
        const Vector2 f = m_views[i]->getSlot().m_feetPos;
        const bool big = (i >= m_party.size()) && isBossStage(m_stage);
        const float k = big ? 1.9f : 1.0f;
        for (int layer = 0; layer < 3; ++layer)            // 3 lop chong nhau -> mep mem
        {
            const float g = 1.0f - 0.22f * static_cast<float>(layer);
            DrawEllipse(static_cast<int>(f.x + 26.0f * k), static_cast<int>(f.y + 3.0f),
                        62.0f * k * g, 15.0f * k * g, Color{ 0, 0, 0, 38 });
        }
        DrawEllipse(static_cast<int>(f.x + 4.0f), static_cast<int>(f.y + 1.0f),
                    30.0f * k, 8.0f * k, Color{ 0, 0, 0, 90 });       // bong tiep dat dam o chan
    }

    for (size_t i : order)
    {
        const Color tint = (i < m_party.size()) ? WHITE : elementColor(m_combatants[i]->getElement());
        m_views[i]->draw(m_battleMap, tint);   // da gom bong + mo dan khi chet
    }
}

void GameEngine::renderBattleUI()
{
    const std::size_t count = std::min(m_combatants.size(), m_views.size());
    if (m_combatants.size() != m_views.size())
        std::cerr << "[BattleUI] HUD drawing only matched entries: combatants="
                  << m_combatants.size() << " views=" << m_views.size() << '\n';

    std::vector<Character*> party;
    party.reserve(m_party.size());
    for (const auto& member : m_party.getMembers())
        if (member) party.push_back(member.get());

    m_ui.drawPartyCards(party, m_currentCharacter, m_selectedAlly);
    m_ui.drawRoundChip(m_stage, m_currentCharacter, m_gold);
    m_turnOrderBar.draw(upcomingOrder(), m_currentCharacter, m_views);

    // ----- HP/MP + target indicator + status effects -----
    for (size_t i = 0; i < count; ++i)
    {
        const Character* c = m_combatants[i].get();
        if (!c || !m_views[i]) continue;
        if (!c->isAlive()) continue;

        const Vector2 head = m_views[i]->getHeadPos();
        m_ui.drawHpMpBar(*c, head,
            i >= m_party.size() ? static_cast<int>(i - m_party.size()) : -1);
        UiWidgets::Tooltip(m_views[i]->getDrawBounds(),
            Strings::localizedName(c->getName()) + TextFormat(" · HP %d/%d",
                std::max(0, c->getCurrentHp()), c->getStats().m_hp));

        if (c == m_selectedTarget && m_isSelectingTarget)
            m_ui.drawTargetCursor(m_views[i]->getDrawBounds());

        // Hieu ung dang co (giu logic cu cua Core; BattleUI chua ve phan nay)
        auto fx = m_statusEffects.find(c);
        if (fx != m_statusEffects.end())
        {
            int ex = static_cast<int>(head.x) - 60;
            const int ey = static_cast<int>(head.y) - 12;
            for (const StatusEffect& effect : fx->second.getEffects())
            {
                const char* label = TextFormat("%s(%d)", statusName(effect.type), effect.durationTurns);
                UiText::draw(label, ex, ey, 12, ORANGE);
                ex += UiText::measure(label, 12) + 6;
            }
        }
    }
    drawFieldStats();
    drawActiveMarker();
    if (count == 0)
        std::cerr << "[BattleUI] placeholder HUD: no matched combatants/views\n";

    // ----- Floating gold -----
    for (const GoldPopup& p : m_goldPopups) {
        const float t = std::max(0.0f, std::min(1.0f, p.m_life / GOLD_POPUP_LIFE));
        const unsigned char a = static_cast<unsigned char>(255.0f * t);
        const char* text = TextFormat("+%d vàng", p.m_amount);
        const int w = UiText::measure(text, 22);
        UiText::draw(text, static_cast<int>(p.m_pos.x - w / 2.0f) + 2, static_cast<int>(p.m_pos.y) + 2, 22, Color{ 0, 0, 0, a });
        UiText::draw(text, static_cast<int>(p.m_pos.x - w / 2.0f), static_cast<int>(p.m_pos.y), 22, Color{ 255, 215, 70, a });
    }

    // ----- Command cluster and optional panels -----
    m_ui.drawControlPanel();
    m_ui.drawTurnBanner(m_stage, m_currentCharacter,
                        m_currentCharacter && isHero(m_currentCharacter),
                        m_battlePhase == BattlePhase::PLAYER_CONFIRM);

    const bool playerControlsVisible = m_battlePhase == BattlePhase::PLAYER_CONFIRM ||
        (m_battlePhase == BattlePhase::PLAYER_TURN && m_currentCharacter &&
         isHero(m_currentCharacter));
    if (playerControlsVisible)
        m_ui.drawBattleControls(m_battlePhase == BattlePhase::PLAYER_CONFIRM,
                                m_isSelectingTarget,
                                m_isSelectingSkill || m_isSelectingItem,
                                !m_commandStack.empty());

    m_ui.drawLogButton(m_battleLogOpen);
    if (m_battleLogOpen)
        m_ui.drawBattleLog(m_battleLog.empty() ? "Chưa có sự kiện nào." : m_battleLog);

    if (m_isSelectingSkill)
        m_ui.drawSkillMenu(m_heroSkills, m_selectedSkillIndex, hero() ? hero()->getCurrentMp() : 0);
    if (m_isSelectingItem) {
        const auto items = availableItems();
        std::vector<std::string> labels;
        labels.reserve(items.size());
        for (std::size_t index = 0; index < items.size(); ++index) {
            const ItemPtr& item = items[index];
            labels.push_back(std::to_string(index + 1) + ". " +
                Strings::localizedName(item->getName()) + "  ×" +
                std::to_string(m_party.countItem(item->getId())));
        }
        m_ui.drawItemMenu(labels, m_selectedItemIndex);
        for (std::size_t index = 0; index < items.size(); ++index) {
            const Rectangle row = BattleUI::itemRowBounds(static_cast<int>(index),
                static_cast<int>(items.size()), m_selectedItemIndex);
            if (row.width > 0.0f)
                UiWidgets::Tooltip(row, items[index]->getDescription());
        }
    }
}
void GameEngine::drawActiveMarker() const
{
    if (!m_currentCharacter || !m_currentCharacter->isAlive()) return;
    for (std::size_t i = 0; i < m_views.size() && i < m_combatants.size(); ++i)
    {
        if (!m_views[i] || m_combatants[i].get() != m_currentCharacter) continue;
        const Vector2 head = m_views[i]->getHeadPos();
        const float bob = std::sin(static_cast<float>(GetTime()) * 6.0f) * 4.0f;
        const float r = 10.0f;
        const Vector2 center{ head.x, head.y - 14.0f + bob };
        DrawPoly(center, 3, r, 90.0f, Color{ 0, 0, 0, 160 });
        DrawPoly(Vector2{ center.x, center.y - 1.0f }, 3, r - 2.0f, 90.0f, UiTheme::cyan);
        return;
    }
}

void GameEngine::drawFieldStats() const
{
    for (std::size_t i = 0; i < m_combatants.size() && i < m_views.size(); ++i)
    {
        if (i < m_party.size()) continue;          // hero: đã có bảng thông tin ở góc dưới
        const Character* c = m_combatants[i].get();
        if (!c || !m_views[i] || !c->isAlive()) continue;
        const Vector2 f = m_views[i]->getSlot().m_feetPos;
        const char* text = TextFormat("HP %d/%d   MP %d/%d",
            std::max(0, c->getCurrentHp()), c->getStats().m_hp,
            std::max(0, c->getCurrentMp()), c->getStats().m_mp);
        const int w = UiText::measure(text, 13);
        const int x = static_cast<int>(f.x - w * 0.5f);
        const int y = static_cast<int>(f.y + UiTheme::fieldBarOffsetY + 9.0f);
        UiText::draw(text, x + 1, y + 1, 13, Color{ 0, 0, 0, 200 });
        UiText::draw(text, x, y, 13, UiTheme::silver);
    }
}
