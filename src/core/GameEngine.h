#pragma once

#include "GameStateMachine.h"
#include "CommandStack.h"
#include "data/GameData.h"
#include "data/Skill.h"
#include "dsa/TurnManager.h"
#include "dsa/CombatAI.h"
#include "dsa/WaveManager.h"
#include "dsa/StatusEffect.h"
#include "dsa/CombatObserver.h"
#include "graphics/BattleMap.h"
#include "graphics/BattleUI.h"
#include "graphics/StartMenu.h" 
#include "graphics/HeroCreateScreen.h"
#include "graphics/PrepMenuScreen.h"
#include "graphics/WaveSelectScreen.h"    
#include "graphics/CombatantView.h"
#include "graphics/TurnOrderBar.h"
#include "graphics/SpriteAnimation.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "entities/Enemy.h"
#include "entities/Boss.h"
#include "entities/Inventory.h"
#include "entities/Party.h"
#include "entities/ItemDatabase.h"
#include "entities/ItemFactory.h"
#include "audio/AudioManager.h"
#include "data/ShopConfig.h"
#include "graphics/ItemIcons.h"
#include "graphics/SceneShader.h"
#include "graphics/ShopScreen.h"
#include "raylib.h"
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <algorithm>

// GameEngine (Core) la noi NOI tat ca module:
//   Data     -> GameData (CSV: hero, quai, skill)
//   Entities -> Character/Player/Enemy/Boss, Inventory<T>
//   DSA      -> TurnManager (vong luot), StatusEffectManager, SubjectCombat (Observer)
//   Graphics -> BattleMap, BattleUI, CombatantView (SpriteAnimation)
// GameEngine ke thua rieng ICombatObserver de tu nhan thong bao mau/chet.
class GameEngine : private ICombatObserver
{
    friend struct GameEngineTestAccess;

public:
    GameEngine() = default;
    ~GameEngine() override = default;

    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    void run();
    void requestState(GameStateMachine::State state);
    bool queuePlayerCommand(std::unique_ptr<ICommand> command);
    bool cancelLastAction();

private:
    // ----- vong doi -----
    void initialize();
    void shutdown();
    void processEvents();
    void update(float deltaTime);
    void render();
    void renderBattleUI();
    void drawActiveMarker() const;
    void drawFieldStats() const;
    void renderCombatants();
    

    // ----- thiet lap tran dau -----
    void startNewGame(int startStage = 1, bool preservePreparedRun = false);
    void loadStage(int stage);
    void createHero();
    void spawnEnemies(int stage);
    void addCombatant(std::unique_ptr<Character> character, const std::string& spritePath = "");
    void layoutViews();
    std::vector<Character*> upcomingOrder() const;   // thu tu luot de hien thi (Turn Order)
    int  finalStage();
    bool isBossStage(int stage);
    int  m_finalStageCache = 0;
    void setupHeroCreateScreen();
    void openWaveSelect();  
    void leavePrepMenu();
    void openPrepMenu(int wave);
    void markWaveCleared(int stage); 
    void applyHeroSelection(const HeroSelection& selection);
    void openPartyMemberSelection(int slot);
    static Skill toBattleSkill(const SkillData& data, Element heroElement);

    // ----- chuyen di (run), tien, cua hang -----
    void beginRun(int startStage);                       // dung doi hinh + tui do + tien (chua vao tran)
    bool waveHasBoss(int wave);
    void recoverPartyAfterWave();
    void enterBossBreak(int nextWave);                   // dung chan truoc wave boss: mo man Chuan bi co Shop
    void beginBattleAfterBreak();
    void settleGold();                                   // tra tien cho quai da chet (chi khi khong con Undo)
    int  computeGoldDrop(const Character* enemy);
    void addGoldPopup(Vector2 pos, int amount);
    void openShop();
    void closeShop();
    void refreshShop();
    void handleShopInput();
    bool handleDialogInput();
    void drawDialog(const std::string& title, const std::string& body,
                    const std::vector<std::string>& buttons) const;
    void buyShopItem(int stockIndex);
    void equipFromBag(int bagIndex, int memberIndex);
    void unequipMember(int memberIndex);
    int  ownedCount(const std::string& itemId) const;    // trong tui + dang deo
    Rectangle shopButtonRect() const;
    void drawGoldHud(float x, float y) const;
    void drawPrepExtras() const;

    // ----- am thanh -----
    void updateMusic();
    void playActionSfx(Character* character, AnimState state);
    void playDeathSfx(Character* character);

    // ----- input nguoi choi -----
    enum class BattlePhase {
        PLAYER_TURN,      // cho nguoi choi chon lenh
        PLAYER_CONFIRM,   // da danh/dung item -> ENTER ket thuc luot, ESC/Z hoan tac
        AI_THINKING       // quai dang suy nghi
    };
    enum class TargetAction { BASIC_ATTACK, SKILL };

    void processPlayerInput();
    void handleBattleClick(Vector2 position);
    void rejectTargetClick(const char* reason, Vector2 position) const;
    void cycleTarget();
    void ensureValidTarget();
    void doBasicAttack();
    void doUseItem();
    void openItemMenu();
    void selectAlly(int index);
    std::vector<ItemPtr> availableItems() const;
    void refreshCurrentSkills();
    void seedItems();
    bool castSkill(size_t skillIndex);
    void confirmTurn();

    // ----- luot choi / AI -----
    void advanceTurn();
    bool beginTurn(Character* character);
    void executeAiTurn(Character* aiChar);
    void checkBattleCondition();

    // ----- chien dau (dung chung Player + AI) -----
    float damageMultiplier(const Character* attacker, const Character* target) const;
    void spendMpWithoutDamage(Character* caster, const Skill& skill);
    void applySkillEffect(const Skill& skill, Character* target);
    void playAnimation(Character* character, AnimState state);

    // ----- Observer (ICombatObserver) -----
    void onHealthChanged(Character* character, int newHealth) override;
    void onCharacterDied(Character* character) override;
    void syncHealth(Character* character);

    // ----- tien ich -----
    int  indexOf(const Character* character) const;
    bool isHero(const Character* character) const;
    Character* hero() const;
    BattleSlot slotOf(int index) const;
    std::string resolvePath(const std::string& relativePath) const;

    // ----- Core Managers -----
    GameStateMachine m_stateMachine;
    CommandStack m_commandStack;
    GameData m_gameData;
    TurnManager m_turnManager;
    WaveManager m_waveManager;
    BattleMap m_battleMap;
    BattleUI  m_ui;
    TurnOrderBar m_turnOrderBar;
    StartMenu m_startMenu;
    HeroCreateScreen m_heroCreate;
    WaveSelectScreen m_waveSelect;
    PrepMenuScreen m_prepMenu;
    int m_pendingWave = 1;
    int m_unlockedWave = 1;                 // wave cao nhat da mo khoa (trong phien choi hien tai)
    std::vector<bool> m_wavesCleared;       // m_wavesCleared[n] = da hoan thanh wave n

struct HeroConfig {                              // hero ngÆ°á»i chÆ¡i chá»n
    std::string m_name;
    std::string m_race;
    std::string m_class;
    Element m_element = Element::KIM;
};
HeroConfig m_heroConfig;
    SubjectCombat m_combatSubject;

    // ----- Entities (Engine so huu, cac module khac chi quan sat bang con tro tho) -----
    std::vector<std::shared_ptr<Character>> m_combatants;            // [0] = hero
    std::vector<std::unique_ptr<CombatantView>> m_views;       // song song voi m_combatants
    std::unordered_map<const Character*, StatusEffectManager> m_statusEffects;
    std::unordered_map<const Character*, int> m_baseSpeed;
    std::unordered_map<const Character*, float> m_scaleMul;          // he so to sprite (hero khung 128 -> nho lai)
    std::unordered_map<const Character*, int> m_lastHp;              // de Observer tinh delta
    std::vector<Skill> m_heroSkills;
    Party m_party;
    ItemDatabase m_itemDatabase;
    bool m_itemsLoaded = false;
    std::vector<HeroConfig> m_partyConfigs; // cac Hero thuc su duoc nguoi choi chon
    int m_editingPartySlot = 0;
    bool m_selectingFromPrep = false;
    int m_prepGearIndex = 0;
    std::vector<std::string> m_gearIds;
    Character* m_selectedAlly = nullptr;
    bool m_isSelectingItem = false;
    bool m_battleLogOpen = false;
    bool m_paused = false;
    int m_dialogSelected = 0;
    int m_selectedItemIndex = 0;

    // ----- Tien / Shop / Am thanh -----
    struct GoldPopup { Vector2 m_pos{}; int m_amount = 0; float m_life = 1.4f; };
    AudioManager m_audio;
    ItemIcons m_itemIcons;
    SceneShader m_sceneShader;
    ShopConfig m_shopConfig;
    ShopScreen m_shop;
    int m_gold = 0;
    bool m_shopOpen = false;
    bool m_shopPreparedRun = false;
    bool m_resumeRun = false;                            // true = dang nghi giua 2 wave (Chuan bi + Shop), KHONG tao lai doi hinh
    ItemPtr m_equippedGear[Party::MAX_SIZE];             // trang bi dang deo cua tung hero (theo vi tri trong Party)
    std::unordered_set<const Character*> m_goldPaid;     // quai da tra tien (chong tra 2 lan)
    std::vector<GoldPopup> m_goldPopups;

    // ----- Tracking State -----
    Character* m_currentCharacter = nullptr;
    std::vector<const Character*> m_actedThisRound;   // ai đã đi trong vòng hiện tại (cho thanh lượt)
    Character* m_selectedTarget = nullptr;
    BattlePhase m_battlePhase = BattlePhase::PLAYER_TURN;
    int m_stage = 1;
    bool m_dataLoaded = false;

    // ----- UI & Timer -----
    int m_selectedSkillIndex = 0;
    bool m_isSelectingSkill = false;
    bool m_isSelectingTarget = false;
    TargetAction m_targetAction = TargetAction::BASIC_ATTACK;
    float m_aiTurnTimer = 0.0f;
    std::string m_battleLog = "Chao mung den voi Battle Engine!";

    std::mt19937 m_rng{ std::random_device{}() };
    bool m_isRunning = false;
    bool m_isInitialized = false;

    
};
