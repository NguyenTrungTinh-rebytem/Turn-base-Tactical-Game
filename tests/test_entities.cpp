// Test cho T08: CharacterFactory, Party (toi da 3), Inventory da hinh, ham hoi MP, getter,
// va 16 vat pham (data/items.csv): Consumable, Booster, Held Gear. Khong can Raylib.
// Chay: cmake --build <build> --target test_entities  roi  ctest  (hoac ./test_entities <thu_muc_data>)
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "data/GameData.h"
#include "entities/CharacterFactory.h"
#include "entities/ItemDatabase.h"
#include "entities/ItemFactory.h"
#include "entities/Party.h"
#include "entities/Item.h"
#include "entities/Inventory.h"
#include "entities/Enemy.h"
#include "entities/Boss.h"
#include "data/Skill.h"

static int g_failed = 0;
#define CHECK(cond) do { if (!(cond)) { ++g_failed; std::cout << "[FAIL] " #cond " (line " << __LINE__ << ")\n"; } } while (0)

template <typename Ex, typename Fn>
static bool throwsAs(Fn fn) {
    try { fn(); } catch (const Ex&) { return true; } catch (...) { return false; }
    return false;
}

static GameData g_data;        // chi dung cho Hero (CharacterFactory)
static ItemDatabase g_items;   // vat pham doc truc tiep tu items.csv
static std::string g_dataDir = "data";

static ItemPtr item(const char* id) { return ItemFactory::createItem(g_items, id); }
static std::shared_ptr<HeldGear> gear(const char* id) { return std::dynamic_pointer_cast<HeldGear>(item(id)); }

// Hero thu nghiem: HP/MP tuy chinh, A = 100, khong giap -> sat thuong de tinh nham
static Player makeHero(int hp = 200, int mp = 100) { return Player("Hero", Stats{ hp, mp, 100, 100, 0, 0, 10 }, Element::KIM); }
static Enemy makeDummy(int hp = 10000) { return Enemy("Dummy", Stats{ hp, 0, 100, 100, 0, 0, 5 }, Element::KIM); }   // KIM vs KIM = x1.0

static Skill physical() { Skill s; s.m_power = 0; s.m_mpCost = 0; s.m_element = Element::KIM; s.m_isMagic = false; return s; }
static Skill magical()  { Skill s = physical(); s.m_isMagic = true; return s; }

// Danh 1 don, tra ve sat thuong thuc te len muc tieu
static int hit(Character& attacker, Character& target, Skill skill = physical()) {
    const int before = target.getCurrentHp();
    attacker.executeSkill(&skill, &target);
    return before - target.getCurrentHp();
}

// ---------------------------------------------------------------- Factory / Party / Inventory / MP

static void testFactory() {
    auto hero = CharacterFactory::createHero(g_data, "Anh Hung", "Human", "Swordman", Element::KIM);
    CHECK(hero != nullptr);
    CHECK(hero->getName() == "Anh Hung");
    CHECK(hero->getRaceName() == "Human" && hero->getClassName() == "Swordman");
    CHECK(hero->getElement() == Element::KIM);

    // Base Stats = Race + Class (Human 100/50/10/10/10/8/8 + Swordman 15/5/2/12/0/6/2)
    CHECK(hero->getMaxHp() == 115 && hero->getMaxMp() == 55 && hero->getSpeed() == 12);
    CHECK(hero->getAttack() == 22 && hero->getMagicAttack() == 10);
    CHECK(hero->getDefense() == 14 && hero->getMagicDefense() == 10);
    CHECK(hero->getCurrentHp() == 115 && hero->getCurrentMp() == 55);

    CHECK(hero->getSkills().size() == 4);
    CHECK(hero->getSkills()[0].m_id == "R_HUMAN_1");
    CHECK(hero->getSkills()[2].m_id == "C_SWORD_1");
    CHECK(hero->getSkills()[1].m_element == Element::KIM);

    auto mage = CharacterFactory::createHero(g_data, "Phap Su", "Elf", "Mage", Element::THUY);
    CHECK(mage->getSkills().size() == 4);
    CHECK(mage->getSkills()[2].m_isMagic);
    CHECK(mage->getSkills()[2].m_element == Element::HOA);

    CHECK(throwsAs<std::out_of_range>([&] { CharacterFactory::createHero(g_data, "X", "Alien", "Mage", Element::KIM); }));
    CHECK(throwsAs<std::out_of_range>([&] { CharacterFactory::createHero(g_data, "X", "Human", "Ninja", Element::KIM); }));
    CHECK(throwsAs<std::invalid_argument>([&] { CharacterFactory::createHero(g_data, "", "Human", "Mage", Element::KIM); }));
    GameData empty;
    CHECK(throwsAs<std::invalid_argument>([&] { CharacterFactory::createHero(empty, "X", "Human", "Mage", Element::KIM); }));
}

static void testMpRecovery() {
    Player p("P", Stats{ 100, 50, 10, 10, 5, 5, 10 }, Element::KIM);
    CHECK(p.consumeMp(30) && p.getCurrentMp() == 20);
    CHECK(!p.consumeMp(21) && p.getCurrentMp() == 20);
    CHECK(!p.consumeMp(-5) && p.getCurrentMp() == 20);

    CHECK(p.restoreMp(10) == 10 && p.getCurrentMp() == 30);
    CHECK(p.restoreMp(100) == 20 && p.getCurrentMp() == 50);   // khong vuot MP toi da
    CHECK(p.restoreMp(5) == 0);
    CHECK(p.restoreMp(-5) == 0 && p.getCurrentMp() == 50);

    p.takeDamage(1000);
    p.consumeMp(50);
    CHECK(!p.isAlive());
    CHECK(p.restoreMp(10) == 0 && p.getCurrentMp() == 0);      // nguoi chet khong hoi MP
}

static void testInventory() {
    Inventory<ItemPtr> bag;
    CHECK(bag.isEmpty() && bag.size() == 0);

    ItemPtr a = item("ITM_HP_MINOR");
    ItemPtr b = item("ITM_HP_MINOR");
    ItemPtr c = item("ITM_ENERGY");
    bag.addItem(a); bag.addItem(c); bag.addItem(b);
    CHECK(bag.size() == 3);

    auto isMana = [](const ItemPtr& it) { return it->getKind() == ItemKind::MANA_POTION; };
    auto isPotion = [](const ItemPtr& it) { return it->getKind() == ItemKind::POTION; };
    CHECK(bag.findIndex(isMana) == 1);
    CHECK(bag.countIf(isPotion) == 2);
    CHECK(bag.getItem(1) == c);
    CHECK(throwsAs<std::out_of_range>([&] { bag.getItem(3); }));

    Player p("P", Stats{ 100, 50, 10, 10, 5, 5, 10 }, Element::KIM);
    p.takeDamage(50); p.consumeMp(50);
    CHECK(bag.getItem(0)->use(p) == 30);     // Potion: 30% x 100 HP
    CHECK(bag.getItem(1)->use(p) == 25);     // Energy Capsule: 50% x 50 MP

    CHECK(bag.removeItem(a) && bag.size() == 2);
    CHECK(!bag.removeItem(a));
    CHECK(bag.removeAt(0) && bag.size() == 1 && bag.getItem(0) == b);
    CHECK(!bag.removeAt(5));
    bag.clear();
    CHECK(bag.isEmpty());

    Inventory<std::string> names;
    names.addItem("x");
    CHECK(names.getItems().size() == 1 && names.removeItem("x"));
}

static void testParty() {
    Party party;
    CHECK(party.isEmpty() && !party.isFull() && party.getLeader() == nullptr);
    CHECK(party.isDefeated());

    std::shared_ptr<Player> h1 = CharacterFactory::createHero(g_data, "H1", "Human", "Swordman", Element::KIM);
    std::shared_ptr<Player> h2 = CharacterFactory::createHero(g_data, "H2", "Elf", "Mage", Element::THUY);
    std::shared_ptr<Player> h3 = CharacterFactory::createHero(g_data, "H3", "Ogre", "Tanker", Element::THO);
    std::shared_ptr<Player> h4 = CharacterFactory::createHero(g_data, "H4", "Oni", "Assassin", Element::HOA);

    CHECK(party.addMember(h1));
    CHECK(!party.addMember(h1));
    CHECK(!party.addMember(nullptr));
    CHECK(party.addMember(h2));
    CHECK(party.addMember(h3));
    CHECK(party.isFull() && party.size() == Party::MAX_SIZE);
    CHECK(!party.addMember(h4));

    CHECK(party.getLeader() == h1.get());
    CHECK(party.getMember(2) == h3.get());
    CHECK(party.getMember(3) == nullptr);

    CHECK(!party.isDefeated());
    h1->takeDamage(9999); h2->takeDamage(9999);
    CHECK(!party.isDefeated());
    h3->takeDamage(9999);
    CHECK(party.isDefeated());

    CHECK(party.removeMember(5) == nullptr);
    CHECK(party.removeMember(0) == h1);
    CHECK(party.getLeader() == h2.get());
    CHECK(party.addMember(h4) && party.isFull());

    party.addItem(item("ITM_HP_MINOR"));
    CHECK(party.getInventory().size() == 1);
    party.clear();
    CHECK(party.isEmpty() && party.getInventory().isEmpty());
}

static void testEnemyStillWorks() {
    Enemy e("Goblin", Stats{ 60, 0, 28, 0, 5, 2, 10 }, Element::MOC);
    CHECK(e.getMaxHp() == 60 && e.getMaxMp() == 0);
    CHECK(e.restoreMp(10) == 0);
}

// ---------------------------------------------------------------- Du lieu items.csv

static void testItemData() {
    CHECK(g_items.isLoaded() && g_items.getItems().size() >= 16);

    int consumables = 0, boosters = 0, held = 0;
    for (const auto& d : g_items.getItems()) {
        if (d.m_category == ItemCategory::CONSUMABLE) ++consumables;
        else if (d.m_category == ItemCategory::BOOSTER) ++boosters;
        else ++held;
        auto it = ItemFactory::createItem(d);     // moi dong CSV deu tao duoc vat pham
        CHECK(it != nullptr && it->getId() == d.m_id);
        CHECK(!it->getDescription().empty() && !it->getName().empty() && !it->getNameVi().empty());
    }
    CHECK(consumables >= 4 && boosters >= 4 && held >= 8);
    CHECK(g_items.getItemsByCategory(ItemCategory::HELD).size() == static_cast<size_t>(held));

    const ItemData& hp = g_items.getItem("ITM_HP_MINOR");
    CHECK(hp.m_name == "Minor Health Potion" && hp.m_value == 30);
    CHECK(item("ITM_HP_MINOR")->getDescription() == hp.m_descriptionVi);   // mo ta cua designer tu CSV
    CHECK(g_items.getItem("ITM_PHOENIX").m_maxCarry == 3);
    CHECK(g_items.getItem("ITM_POWER_SERUM").m_duration == 2);
    CHECK(g_items.findItem("ITM_KHONG_CO") == nullptr);
    CHECK(throwsAs<std::out_of_range>([&] { g_items.getItem("ITM_KHONG_CO"); }));

    // Phan loai Kind theo lop con
    CHECK(item("ITM_HP_MAJOR")->getKind() == ItemKind::POTION);
    CHECK(item("ITM_ENERGY")->getKind() == ItemKind::MANA_POTION);
    CHECK(item("ITM_PHOENIX")->getKind() == ItemKind::REVIVE);
    CHECK(item("ITM_HASTE")->getKind() == ItemKind::BOOSTER);
    CHECK(item("ITM_SASH")->getKind() == ItemKind::HELD_GEAR);

    // Factory tu choi du lieu sai nhom
    ItemData bad = hp;
    bad.m_category = ItemCategory::HELD;
    CHECK(throwsAs<std::invalid_argument>([&] { ItemFactory::createItem(bad); }));
}

// Ghi 1 file items.csv tam roi nap bang ItemDatabase
static std::filesystem::path writeTempItems(const std::string& content) {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "t08_items_test_dir";
    fs::create_directories(dir);
    std::ofstream out(dir / "items.csv", std::ios::binary);
    out << content;
    return dir;
}

static const char* HEADER = "id,name,nameVi,category,effect,value,value2,duration,maxCarry,description\n";

static void testItemCsvReading() {
    namespace fs = std::filesystem;
    fs::remove_all(fs::temp_directory_path() / "t08_items_test_dir");

    auto loads = [&](const std::string& content, std::string* err = nullptr) {
        const fs::path dir = writeTempItems(content);
        ItemDatabase db;
        return db.tryLoad(dir.string(), err);
    };

    // --- Du lieu dung ---
    CHECK(loads(std::string(HEADER) + "ITM_A,A,A VI,CONSUMABLE,HEAL_HP_PCT,10,0,0,0,mo ta\n"));

    // Them vat pham moi chi bang them dong CSV (khong sua code, khong dung so luong co dinh)
    {
        const fs::path dir = writeTempItems(std::string(HEADER) +
            "ITM_A,A,A,CONSUMABLE,HEAL_HP_PCT,10,0,0,0,\n" "ITM_B,B,B,CONSUMABLE,HEAL_HP_PCT,90,0,0,0,\n" "ITM_C,C,C,HELD,HELD_REGEN,5,0,0,0,\n");
        ItemDatabase db; db.load(dir.string());
        CHECK(db.getItems().size() == 3);
        auto big = ItemFactory::createItem(db, "ITM_B");
        Player p("P", Stats{ 100, 0, 1, 1, 0, 0, 1 }, Element::KIM);
        p.takeDamage(99);
        CHECK(big->use(p) == 90);                                    // 90% x 100 HP
    }

    // File luu tu Excel tren Windows: BOM UTF-8 + xuong dong CRLF
    CHECK(loads("\xEF\xBB\xBF" "id,name,category,effect,value\r\nITM_A,A,CONSUMABLE,HEAL_HP_PCT,10\r\n"));

    // Doi thu tu cot + them cot la + bo cot tuy chon: van doc dung theo TEN cot
    {
        const fs::path dir = writeTempItems("effect,value,CATEGORY,name,id,ghi_chu_moi\nREVIVE_PCT,25,CONSUMABLE,Feather,ITM_F,bo qua\n");
        ItemDatabase db; db.load(dir.string());
        const ItemData& d = db.getItem("ITM_F");
        CHECK(d.m_effect == ItemEffect::REVIVE_PCT && d.m_value == 25 && d.m_nameVi == "Feather" && d.m_maxCarry == 0);
    }

    // O co dau phay dat trong nhay kep
    {
        const fs::path dir = writeTempItems(std::string(HEADER) + "ITM_A,A,A,CONSUMABLE,HEAL_HP_PCT,10,0,0,0,\"Hoi mau, nhanh, gon\"\n");
        ItemDatabase db; db.load(dir.string());
        CHECK(db.getItem("ITM_A").m_descriptionVi == "Hoi mau, nhanh, gon");
    }

    // Dong comment '#' va dong trong duoc bo qua
    CHECK(loads("# ghi chu\n\n" + std::string(HEADER) + "# them ghi chu\nITM_A,A,A,CONSUMABLE,HEAL_HP_PCT,10,0,0,0,x\n\n"));

    // --- Du lieu sai: bao loi ro rang (co ten file + dong), khong crash ---
    const char* bad[] = {
        "ITM_X,X,X,HELD,HEAL_HP_PCT,30,0,0,0,\n",                                    // khong co header... (dong dau bi coi la header)
        "id,name,category,effect\nITM_X,X,CONSUMABLE,HEAL_HP_PCT\n",                // thieu cot bat buoc 'value'
        "id,name,category,effect,value\nITM_X,X,HELD,HEAL_HP_PCT,30\n",             // effect khong thuoc nhom
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,KHONG_CO,30\n",          // effect la
        "id,name,category,effect,value\nITM_X,X,KHONG_CO,HEAL_HP_PCT,30\n",         // category la
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,HEAL_HP_PCT,abc\n",      // value khong phai so
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,HEAL_HP_PCT,0\n",        // value <= 0
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,HEAL_HP_PCT,\n",         // value trong
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,HEAL_HP_PCT,10,5\n",     // thua cot (dau phay khong nhay kep)
        "id,name,category,effect,value\nITM_X,X,CONSUMABLE,HEAL_HP_PCT,10\nITM_X,Y,CONSUMABLE,HEAL_HP_PCT,10\n", // trung id
        "id,name,category,effect,value,duration\nITM_X,X,BOOSTER,BOOST_DAMAGE_UP,40,0\n",                        // booster thieu duration
        "id,name,category,effect,value\nITM_X,X,BOOSTER,BOOST_DAMAGE_REDUCE,150\n",                              // giam > 100%
        "id,name,category,effect,value\nITM_X,X,HELD,HELD_BERSERK,30\n",                                         // thieu value2
        "id,name,category,effect,value\n",                                                                       // khong co dong du lieu
        "",                                                                                                       // file rong
    };
    for (const char* content : bad) {
        std::string err;
        const bool ok = loads(content, &err);
        CHECK(!ok);
        CHECK(!err.empty());
    }
    { std::string err; loads(std::string("id,name,category,effect,value\nITM_X,X,HELD,HEAL_HP_PCT,30\n"), &err); std::cout << "Thong bao loi mong doi: " << err << "\n"; }

    // Thieu file: tryLoad khong nem, load() nem ItemLoadError
    {
        ItemDatabase db;
        std::string err;
        CHECK(!db.tryLoad("thu_muc_khong_ton_tai", &err) && !err.empty() && !db.isLoaded());
        CHECK(throwsAs<ItemLoadError>([&] { db.load("thu_muc_khong_ton_tai"); }));
    }

    // Nap lai loi: du lieu cu khong bi mat; sua file xong reload() nhan so lieu moi (khong can bien dich)
    {
        const fs::path dir = writeTempItems(std::string(HEADER) + "ITM_A,A,A,CONSUMABLE,HEAL_HP_PCT,10,0,0,0,\n");
        ItemDatabase db; db.load(dir.string());
        CHECK(db.getItem("ITM_A").m_value == 10);

        writeTempItems("id,name,category,effect,value\nITM_A,A,CONSUMABLE,HEAL_HP_PCT,xyz\n");   // sua hong file
        CHECK(throwsAs<ItemLoadError>([&] { db.reload(); }));
        CHECK(db.isLoaded() && db.getItem("ITM_A").m_value == 10);                                // giu du lieu cu

        writeTempItems(std::string(HEADER) + "ITM_A,A,A,CONSUMABLE,HEAL_HP_PCT,55,0,0,0,\n");     // sua dung lai
        db.reload();
        CHECK(db.getItem("ITM_A").m_value == 55);
    }

    fs::remove_all(fs::temp_directory_path() / "t08_items_test_dir");
}

// ---------------------------------------------------------------- Consumables

static void testConsumables() {
    Player p("P", Stats{ 100, 60, 10, 10, 0, 0, 10 }, Element::KIM);
    auto minor = item("ITM_HP_MINOR");
    auto major = item("ITM_HP_MAJOR");
    auto energy = item("ITM_ENERGY");

    CHECK(!minor->canUse(p) && !energy->canUse(p));           // day HP/MP
    CHECK(minor->use(p) == 0 && energy->use(p) == 0);

    p.takeDamage(90);                                         // HP 10
    CHECK(minor->use(p) == 30 && p.getCurrentHp() == 40);     // 30% x 100
    CHECK(major->use(p) == 60 && p.getCurrentHp() == 100);    // 70% = 70, bi chan boi max (con thieu 60)
    p.takeDamage(30);                                         // HP 70
    const int healed = major->use(p);
    CHECK(healed == 30 && p.getCurrentHp() == 100);
    major->revert(p, healed);                                 // Undo
    CHECK(p.getCurrentHp() == 70);

    p.consumeMp(60);                                          // MP 0
    const int restored = energy->use(p);
    CHECK(restored == 30 && p.getCurrentMp() == 30);          // 50% x 60 MP
    energy->revert(p, restored);
    CHECK(p.getCurrentMp() == 0);
    CHECK(energy->use(p) == 30 && energy->use(p) == 30 && p.getCurrentMp() == 60);

    // Phoenix Feather: chi dung len nguoi da guc, hoi 25% HP
    auto phoenix = item("ITM_PHOENIX");
    Player ally("Ally", Stats{ 200, 0, 10, 10, 0, 0, 10 }, Element::KIM);
    CHECK(!phoenix->canUse(ally));                            // con song -> khong dung
    CHECK(phoenix->use(ally) == 0);
    ally.takeDamage(9999);
    CHECK(!ally.isAlive() && phoenix->canUse(ally));
    CHECK(!minor->canUse(ally));                              // bình HP khong hoi sinh duoc
    const int revived = phoenix->use(ally);
    CHECK(revived == 50 && ally.isAlive() && ally.getCurrentHp() == 50);   // 25% x 200
    phoenix->revert(ally, revived);                           // Undo -> guc lai
    CHECK(!ally.isAlive());

    // Gioi han mang theo (2-3 cai/chuyen): maxCarry = 3
    Party party;
    CHECK(party.addItem(item("ITM_PHOENIX")) && party.addItem(item("ITM_PHOENIX")) && party.addItem(item("ITM_PHOENIX")));
    CHECK(!party.addItem(item("ITM_PHOENIX")));
    CHECK(party.countItem("ITM_PHOENIX") == 3);
    for (int i = 0; i < 10; ++i) CHECK(party.addItem(item("ITM_HP_MINOR")));   // vat pham khac khong gioi han
    CHECK(party.countItem("ITM_HP_MINOR") == 10);
    CHECK(!party.addItem(nullptr));
    CHECK(party.getItemsByCategory(ItemCategory::CONSUMABLE).size() == 13);
    CHECK(party.getItemsByCategory(ItemCategory::HELD).empty());
}

// ---------------------------------------------------------------- Boosters

static void testPowerSerum() {
    Player hero = makeHero();
    Enemy dummy = makeDummy();
    auto serum = item("ITM_POWER_SERUM");

    CHECK(hit(hero, dummy) == 100);                           // co ban
    CHECK(serum->canUse(hero));
    const int applied = serum->use(hero);
    CHECK(applied == 1 && hero.modifiers().hasBuff(BuffType::DAMAGE_UP));
    CHECK(!serum->canUse(hero) && serum->use(hero) == 0);     // khong dung chong

    hero.onTurnEnd();                                         // het luot dung thuoc: chua tinh
    CHECK(hero.modifiers().getBuffTurnsLeft(BuffType::DAMAGE_UP) == 2);
    CHECK(hit(hero, dummy) == 140);                           // luot ke 1: +40%
    hero.onTurnEnd();
    CHECK(hit(hero, dummy) == 140);                           // luot ke 2: van con
    hero.onTurnEnd();
    CHECK(!hero.modifiers().hasBuff(BuffType::DAMAGE_UP));                // het han
    CHECK(hit(hero, dummy) == 100);

    // Undo: go buff
    Player h2 = makeHero();
    serum->revert(h2, serum->use(h2));
    CHECK(!h2.modifiers().hasBuff(BuffType::DAMAGE_UP));
}

static void testIronShielding() {
    Player hero = makeHero(100000);
    Enemy dummy = makeDummy();
    auto shield = item("ITM_IRON_SHIELD");

    CHECK(hit(dummy, hero) == 100);
    CHECK(shield->use(hero) == 1);
    hero.onTurnEnd();
    CHECK(hit(dummy, hero) == 50);                            // giam 50%
    hero.onTurnEnd();
    CHECK(hit(dummy, hero) == 50);
    hero.onTurnEnd();
    CHECK(hit(dummy, hero) == 100);                           // het 2 luot

    // Khong lam sat thuong ve 0: toi thieu 1
    Player weak = makeHero(1000);
    Enemy tiny("Tiny", Stats{ 100, 0, 1, 0, 0, 0, 5 }, Element::KIM);
    item("ITM_IRON_SHIELD")->use(weak);
    CHECK(hit(tiny, weak) == 1);
}

static void testHasteTonic() {
    Player hero = makeHero();
    auto haste = item("ITM_HASTE");

    CHECK(!hero.modifiers().consumeExtraTurn());
    CHECK(haste->canUse(hero) && haste->use(hero) == 1);
    CHECK(hero.modifiers().hasBuff(BuffType::EXTRA_TURN));
    CHECK(!haste->canUse(hero));                              // chua dung luot them thi khong dung tiep
    hero.onTurnEnd();
    CHECK(hero.modifiers().hasBuff(BuffType::EXTRA_TURN));                // khong tu het han theo luot
    CHECK(hero.modifiers().consumeExtraTurn());                           // nhan dung 1 luot them
    CHECK(!hero.modifiers().consumeExtraTurn());

    Player h2 = makeHero();
    haste->revert(h2, haste->use(h2));                        // Undo
    CHECK(!h2.modifiers().consumeExtraTurn());
}

static void testPrecisionLens() {
    Player hero = makeHero();
    Enemy dummy = makeDummy();
    auto lens = item("ITM_PRECISION");

    CHECK(hero.modifiers().getCritRate() == 0 && hero.modifiers().getCritDamagePercent() == 150);
    CHECK(lens->use(hero) == 1);
    CHECK(hero.modifiers().getCritRate() == 100);
    CHECK(hit(hero, dummy) == 150);                           // don ke tiep chi mang x1.5
    CHECK(hero.modifiers().getCritRate() == 0);                           // dung 1 lan
    CHECK(hit(hero, dummy) == 100);

    Player h2 = makeHero();
    lens->revert(h2, lens->use(h2));
    CHECK(h2.modifiers().getCritRate() == 0);

    // Heavy Strike Stone: chi mang +50% -> x2.0
    Player h3 = makeHero();
    h3.modifiers().equipGear(gear("ITM_HEAVY_STONE"));
    CHECK(h3.modifiers().getCritDamagePercent() == 200);
    CHECK(hit(h3, dummy) == 100);                             // khong chi mang thi khong co tac dung
    item("ITM_PRECISION")->use(h3);
    CHECK(hit(h3, dummy) == 200);
}

// ---------------------------------------------------------------- Held Gear

static void testLeftoverBand() {
    Player hero = makeHero(200);
    hero.modifiers().equipGear(gear("ITM_LEFTOVER"));
    hero.takeDamage(100);
    hero.onTurnEnd();
    CHECK(hero.getCurrentHp() == 116);                        // +8% x 200 = 16
    hero.heal(1000);
    hero.onTurnEnd();
    CHECK(hero.getCurrentHp() == 200);                        // khong vuot max

    Player dead = makeHero(200);
    dead.modifiers().equipGear(gear("ITM_LEFTOVER"));
    dead.takeDamage(9999);
    dead.onTurnEnd();
    CHECK(!dead.isAlive());                                   // nguoi guc khong duoc hoi
}

static void testVampiricCharm() {
    Player hero = makeHero(200);
    Enemy dummy = makeDummy();
    hero.modifiers().equipGear(gear("ITM_VAMPIRIC"));
    hero.takeDamage(100);
    CHECK(hit(hero, dummy) == 100);
    CHECK(hero.getCurrentHp() == 115);                        // +15% x 100 sat thuong gay ra
}

static void testSurvivalSash() {
    Enemy big("Big", Stats{ 100, 0, 500, 500, 0, 0, 5 }, Element::KIM);
    auto sash = gear("ITM_SASH");

    Player hero("H", Stats{ 100, 0, 10, 10, 0, 0, 10 }, Element::KIM);
    hero.modifiers().equipGear(sash);
    CHECK(hit(big, hero) == 99 && hero.getCurrentHp() == 1);  // day HP + don dut diem -> con 1 HP
    CHECK(sash->isSurviveUsed());
    hero.heal(1000);
    CHECK(hit(big, hero) == 100 && !hero.isAlive());          // chi 1 lan/tran

    // Khong day HP -> khong kich hoat
    Player hurt("H2", Stats{ 100, 0, 10, 10, 0, 0, 10 }, Element::KIM);
    hurt.modifiers().equipGear(gear("ITM_SASH"));
    hurt.takeDamage(1);
    hit(big, hurt);
    CHECK(!hurt.isAlive());

    // Don khong dut diem -> khong ton luot kich hoat
    Player sturdy("H3", Stats{ 1000, 0, 10, 10, 0, 0, 10 }, Element::KIM);
    auto sash3 = gear("ITM_SASH");
    sturdy.modifiers().equipGear(sash3);
    hit(big, sturdy);
    CHECK(sturdy.getCurrentHp() == 500 && !sash3->isSurviveUsed());

    // resetBattleState nap lai cho tran moi
    Player again("H4", Stats{ 100, 0, 10, 10, 0, 0, 10 }, Element::KIM);
    auto sash4 = gear("ITM_SASH");
    again.modifiers().equipGear(sash4);
    hit(big, again);
    CHECK(sash4->isSurviveUsed());
    again.heal(1000);
    again.resetBattleState();
    CHECK(!sash4->isSurviveUsed());
    hit(big, again);
    CHECK(again.isAlive() && again.getCurrentHp() == 1);
}

static void testGuardianArmor() {
    Player hero = makeHero(100000);
    Enemy dummy = makeDummy();
    hero.modifiers().equipGear(gear("ITM_GUARDIAN"));
    CHECK(hit(dummy, hero) == 85);                            // -15%
    item("ITM_IRON_SHIELD")->use(hero);
    CHECK(hit(dummy, hero) == 42);                            // 100 -> 50 (Iron) -> 42 (Guardian, lam tron xuong)
}

static void testBerserkCore() {
    Player hero = makeHero(200);
    Enemy dummy = makeDummy();
    hero.modifiers().equipGear(gear("ITM_BERSERK"));
    CHECK(hit(hero, dummy) == 130);                           // +30%
    CHECK(hero.getCurrentHp() == 190);                        // mat 5% x 200 = 10 sau khi danh

    // Khong tu giet minh: chua lai 1 HP
    Player low = makeHero(200);
    low.modifiers().equipGear(gear("ITM_BERSERK"));
    low.takeDamage(195);                                      // HP 5
    hit(low, dummy);
    CHECK(low.isAlive() && low.getCurrentHp() == 1);
}

static void testExecutionersRing() {
    Player hero = makeHero();
    hero.modifiers().equipGear(gear("ITM_EXECUTIONER"));

    Enemy healthy = makeDummy(1000);
    CHECK(hit(hero, healthy) == 100);                         // HP cao: khong thuong

    Enemy edge = makeDummy(1000);
    edge.takeDamage(700);                                     // dung 30% HP: chua "duoi 30%"
    CHECK(hit(hero, edge) == 100);

    Enemy weak = makeDummy(1000);
    weak.takeDamage(701);                                     // < 30%
    CHECK(hit(hero, weak) == 140);                            // +40%
}

static void testCounterHelmet() {
    Player attacker = makeHero(1000);
    Enemy wearer = makeDummy();
    wearer.modifiers().equipGear(gear("ITM_COUNTER"));

    CHECK(hit(attacker, wearer) == 100);                      // vat ly (can chien)
    CHECK(attacker.getCurrentHp() == 985);                    // phan lai 15%

    CHECK(hit(attacker, wearer, magical()) == 100);           // phep: khong phan don
    CHECK(attacker.getCurrentHp() == 985);

    // Phan don co the ha guc ke tan cong nhung khong de quy
    Player glass("Glass", Stats{ 10, 0, 1000, 0, 0, 0, 5 }, Element::KIM);
    Enemy wearer2 = makeDummy(100000);
    wearer2.modifiers().equipGear(gear("ITM_COUNTER"));
    hit(glass, wearer2);                                      // gay 1000 -> phan 150 > 10 HP
    CHECK(!glass.isAlive());
}

static void testBossStillWorks() {
    // Boss van gay x1.25 va nhan x0.8 khi khong trang bi gi
    Boss boss("Boss", Stats{ 10000, 0, 100, 0, 0, 0, 5 }, Element::KIM);
    Player hero = makeHero(10000);
    CHECK(hit(boss, hero) == 125);
    CHECK(hit(hero, boss) == 80);
    // Boss + vat pham
    boss.modifiers().equipGear(gear("ITM_BERSERK"));
    CHECK(hit(boss, hero) == 162);                            // 125 * 1.3 = 162.5 -> 162
}

static void testPartyEquipment() {
    Party party;
    auto h1 = CharacterFactory::createHero(g_data, "H1", "Human", "Swordman", Element::KIM);
    party.addMember(std::shared_ptr<Player>(std::move(h1)));

    auto sash = item("ITM_SASH");
    auto ring = item("ITM_EXECUTIONER");
    auto potion = item("ITM_HP_MINOR");
    party.addItem(sash); party.addItem(ring); party.addItem(potion);

    CHECK(!party.equipGear(0, potion));                       // khong phai trang bi
    CHECK(!party.equipGear(5, sash));                         // hero khong ton tai
    CHECK(!party.equipGear(0, item("ITM_GUARDIAN")));         // khong nam trong tui

    CHECK(party.equipGear(0, sash));
    CHECK(party.getMember(0)->modifiers().getGear() == std::dynamic_pointer_cast<HeldGear>(sash));
    CHECK(party.getInventory().size() == 2);                  // sash roi khoi tui

    CHECK(party.equipGear(0, ring));                          // doi do: sash tro ve tui
    CHECK(party.getInventory().size() == 2);
    CHECK(party.countItem("ITM_SASH") == 1 && party.countItem("ITM_EXECUTIONER") == 0);

    CHECK(party.unequipGear(0));
    CHECK(party.getMember(0)->modifiers().getGear() == nullptr);
    CHECK(party.countItem("ITM_EXECUTIONER") == 1);
    CHECK(!party.unequipGear(0));                             // khong con gi de thao
    CHECK(party.getItemsByCategory(ItemCategory::HELD).size() == 2);
}

int main(int argc, char** argv) {
    g_dataDir = argc > 1 ? argv[1] : "data";
    g_data.loadAll(g_dataDir);
    g_items.load(g_dataDir);

    testFactory();
    testMpRecovery();
    testInventory();
    testParty();
    testEnemyStillWorks();

    testItemData();
    testItemCsvReading();
    testConsumables();
    testPowerSerum();
    testIronShielding();
    testHasteTonic();
    testPrecisionLens();
    testLeftoverBand();
    testVampiricCharm();
    testSurvivalSash();
    testGuardianArmor();
    testBerserkCore();
    testExecutionersRing();
    testCounterHelmet();
    testBossStillWorks();
    testPartyEquipment();

    std::cout << (g_failed == 0 ? "ALL TESTS PASSED\n" : "SOME TESTS FAILED\n");
    return g_failed == 0 ? 0 : 1;
}
