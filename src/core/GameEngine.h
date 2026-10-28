#pragma once

#include "GameStateMachine.h"
#include "CommandStack.h"
#include "data/GameData.h"
#include "data/Skill.h"
#include "dsa/TurnManager.h"
#include "dsa/StatusEffect.h"
#include "dsa/CombatObserver.h"
#include "graphics/BattleMap.h"
#include "graphics/BattleUI.h"
#include "graphics/StartMenu.h" 
#include "graphics/HeroCreateScreen.h"
#include "graphics/PrepMenuScreen.h"
#include "graphics/WaveSelectScreen.h"    
#include "graphics/CombatantView.h"
#include "graphics/SpriteAnimation.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "entities/Enemy.h"
#include "entities/Boss.h"
#include "entities/Inventory.h"
#include "raylib.h"
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// GameEngine (Core) la noi NOI tat ca module:
//   Data     -> GameData (CSV: hero, quai, skill)
//   Entities -> Character/Player/Enemy/Boss, Inventory<T>
//   DSA      -> TurnManager (vong luot), StatusEffectManager, SubjectCombat (Observer)
//   Graphics -> BattleMap, BattleUI, CombatantView (SpriteAnimation)
// GameEngine ke thua rieng ICombatObserver de tu nhan thong bao mau/chet.
class GameEngine : private ICombatObserver
{
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
    void renderCombatants();

    // ----- thiet lap tran dau -----
    void startNewGame(int startStage = 1);
    void loadStage(int stage);
    void createHero();
    void spawnEnemies(int stage);
    void addCombatant(std::unique_ptr<Character> character);
    void layoutViews();
    std::vector<Character*> upcomingOrder() const;   // thu tu luot de hien thi (Turn Order)
    int  finalStage() const;
    void setupHeroCreateScreen();
    void openWaveSelect();  
    void leavePrepMenu();
    void openPrepMenu(int wave);
    void markWaveCleared(int stage); 
    void applyHeroSelection(const HeroSelection& selection);
    static Skill toBattleSkill(const SkillData& data, Element heroElement);

    // ----- input nguoi choi -----
    enum class BattlePhase {
        PLAYER_TURN,      // cho nguoi choi chon lenh
        PLAYER_CONFIRM,   // da danh/dung item -> ENTER ket thuc luot, ESC/Z hoan tac
        AI_THINKING       // quai dang suy nghi
    };

    void processPlayerInput();
    void cycleTarget();
    void ensureValidTarget();
    void doBasicAttack();
    void doUseItem();
    bool castSkill(size_t skillIndex);
    void confirmTurn();

    // ----- luot choi / AI -----
    void advanceTurn();
    bool beginTurn(Character* character);
    void executeAiTurn(Character* aiChar);
    void checkBattleCondition();

    // ----- chien dau (dung chung Player + AI) -----
    float damageMultiplier(const Character* attacker, const Character* target) const;
    void spendMpWithoutDamage(const Skill& skill);
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
    BattleMap m_battleMap;
    BattleUI  m_ui;
    StartMenu m_startMenu;
    HeroCreateScreen m_heroCreate;
    WaveSelectScreen m_waveSelect;
    PrepMenuScreen m_prepMenu;
    int m_pendingWave = 1;
    int m_unlockedWave = 1;                 // wave cao nhat da mo khoa (trong phien choi hien tai)
    std::vector<bool> m_wavesCleared;       // m_wavesCleared[n] = da hoan thanh wave n

struct HeroConfig {                              // hero người chơi chọn
    std::string m_name;
    std::string m_race;
    std::string m_class;
    Element m_element = Element::KIM;
};
HeroConfig m_heroConfig;
    SubjectCombat m_combatSubject;

    // ----- Entities (Engine so huu, cac module khac chi quan sat bang con tro tho) -----
    std::vector<std::unique_ptr<Character>> m_combatants;            // [0] = hero
    std::vector<std::unique_ptr<CombatantView>> m_views;       // song song voi m_combatants
    std::unordered_map<const Character*, StatusEffectManager> m_statusEffects;
    std::unordered_map<const Character*, int> m_lastHp;              // de Observer tinh delta
    std::vector<Skill> m_heroSkills;
    Inventory<std::string> m_inventory;

    // ----- Tracking State -----
    Character* m_currentCharacter = nullptr;
    Character* m_selectedTarget = nullptr;
    BattlePhase m_battlePhase = BattlePhase::PLAYER_TURN;
    int m_stage = 1;
    bool m_dataLoaded = false;

    // ----- UI & Timer -----
    int m_selectedSkillIndex = 0;
    bool m_isSelectingSkill = false;
    float m_aiTurnTimer = 0.0f;
    std::string m_battleLog = "Chao mung den voi Battle Engine!";

    std::mt19937 m_rng{ std::random_device{}() };
    bool m_isRunning = false;
    bool m_isInitialized = false;
};