// Test cho T08: CharacterFactory, Party (toi da 3), Item/Potion + Inventory da hinh,
// getter man Chi tiet, ham hoi MP. Khong can Raylib.
// Chay: cmake --build <build> --target test_entities  roi  ./test_entities <thu_muc_data>
#include <iostream>
#include <stdexcept>
#include "data/GameData.h"
#include "entities/CharacterFactory.h"
#include "entities/Party.h"
#include "entities/Item.h"
#include "entities/Inventory.h"
#include "entities/Enemy.h"

static int g_failed = 0;
#define CHECK(cond) do { if (!(cond)) { ++g_failed; std::cout << "[FAIL] " #cond " (line " << __LINE__ << ")\n"; } } while (0)

template <typename Ex, typename Fn>
static bool throwsAs(Fn fn) {
    try { fn(); } catch (const Ex&) { return true; } catch (...) { return false; }
    return false;
}

static void testFactory(const GameData& data) {
    auto hero = CharacterFactory::createHero(data, "Anh Hung", "Human", "Swordman", Element::KIM);
    CHECK(hero != nullptr);
    CHECK(hero->getName() == "Anh Hung");
    CHECK(hero->getRaceName() == "Human" && hero->getClassName() == "Swordman");
    CHECK(hero->getElement() == Element::KIM);

    // Base Stats = Race + Class (Human 100/50/10/10/10/8/8 + Swordman 15/5/2/12/0/6/2)
    CHECK(hero->getMaxHp() == 115 && hero->getMaxMp() == 55 && hero->getSpeed() == 12);
    CHECK(hero->getAttack() == 22 && hero->getMagicAttack() == 10);
    CHECK(hero->getDefense() == 14 && hero->getMagicDefense() == 10);
    CHECK(hero->getCurrentHp() == 115 && hero->getCurrentMp() == 55);

    // 4 skill: 2 Race (Human) + 2 Class (Swordman)
    CHECK(hero->getSkills().size() == 4);
    CHECK(hero->getSkills()[0].m_id == "R_HUMAN_1");
    CHECK(hero->getSkills()[2].m_id == "C_SWORD_1");
    CHECK(hero->getSkills()[1].m_element == Element::KIM);   // skill khong hieu ung -> theo he hero

    auto mage = CharacterFactory::createHero(data, "Phap Su", "Elf", "Mage", Element::THUY);
    CHECK(mage->getSkills().size() == 4);
    CHECK(mage->getSkills()[2].m_isMagic);                   // C_MAGE_1: class Mage -> phep
    CHECK(mage->getSkills()[2].m_element == Element::HOA);   // BURN -> HOA

    // Loi dau vao: nem exception ro rang, khong tao hero do dang
    CHECK(throwsAs<std::out_of_range>([&] { CharacterFactory::createHero(data, "X", "Alien", "Mage", Element::KIM); }));
    CHECK(throwsAs<std::out_of_range>([&] { CharacterFactory::createHero(data, "X", "Human", "Ninja", Element::KIM); }));
    CHECK(throwsAs<std::invalid_argument>([&] { CharacterFactory::createHero(data, "", "Human", "Mage", Element::KIM); }));
    GameData empty;
    CHECK(throwsAs<std::invalid_argument>([&] { CharacterFactory::createHero(empty, "X", "Human", "Mage", Element::KIM); }));
}

static void testMpRecovery() {
    Player p("P", Stats{ 100, 50, 10, 10, 5, 5, 10 }, Element::KIM);
    CHECK(p.consumeMp(30) && p.getCurrentMp() == 20);
    CHECK(!p.consumeMp(21) && p.getCurrentMp() == 20);        // thieu MP -> khong tru
    CHECK(!p.consumeMp(-5) && p.getCurrentMp() == 20);

    CHECK(p.restoreMp(10) == 10 && p.getCurrentMp() == 30);
    CHECK(p.restoreMp(100) == 20 && p.getCurrentMp() == 50);   // khong vuot MP toi da
    CHECK(p.restoreMp(5) == 0);                                // da day
    CHECK(p.restoreMp(-5) == 0 && p.getCurrentMp() == 50);     // so am bi bo qua

    p.takeDamage(1000);                                        // chet
    p.consumeMp(50);
    CHECK(!p.isAlive());
    CHECK(p.restoreMp(10) == 0 && p.getCurrentMp() == 0);      // nguoi chet khong hoi MP
}

static void testItems() {
    Player p("P", Stats{ 100, 50, 10, 10, 5, 5, 10 }, Element::KIM);
    Potion hp(40);
    ManaPotion mp(30);

    CHECK(hp.getKind() == ItemKind::POTION && mp.getKind() == ItemKind::MANA_POTION);
    CHECK(hp.getName() == "Potion" && mp.getName() == "Mana Potion");

    // Day HP/MP -> khong dung duoc
    CHECK(!hp.canUse(p) && !mp.canUse(p));
    CHECK(hp.use(p) == 0 && mp.use(p) == 0);

    // Potion
    p.takeDamage(70);                       // HP 30
    CHECK(hp.canUse(p));
    CHECK(hp.use(p) == 40 && p.getCurrentHp() == 70);
    p.takeDamage(10);                       // HP 60 -> chi hoi duoc 40 (100 max)
    CHECK(hp.use(p) == 40 && p.getCurrentHp() == 100);
    p.takeDamage(20);                       // HP 80 -> chi hoi 20 (bi chan boi max)
    const int healed = hp.use(p);
    CHECK(healed == 20 && p.getCurrentHp() == 100);
    hp.revert(p, healed);                   // Undo tra lai dung luong da hoi
    CHECK(p.getCurrentHp() == 80);

    // ManaPotion
    p.consumeMp(40);                        // MP 10
    CHECK(mp.canUse(p));
    const int restored = mp.use(p);
    CHECK(restored == 30 && p.getCurrentMp() == 40);
    mp.revert(p, restored);
    CHECK(p.getCurrentMp() == 10);
    CHECK(mp.use(p) == 30);                 // dung lai sau khi undo
    CHECK(mp.use(p) == 10 && p.getCurrentMp() == 50);   // bi chan boi max
}

static void testInventory() {
    Inventory<ItemPtr> bag;
    CHECK(bag.isEmpty() && bag.size() == 0);

    ItemPtr a = std::make_shared<Potion>();
    ItemPtr b = std::make_shared<Potion>();
    ItemPtr c = std::make_shared<ManaPotion>();
    bag.addItem(a); bag.addItem(c); bag.addItem(b);
    CHECK(bag.size() == 3);

    auto isMana = [](const ItemPtr& it) { return it->getKind() == ItemKind::MANA_POTION; };
    auto isPotion = [](const ItemPtr& it) { return it->getKind() == ItemKind::POTION; };
    CHECK(bag.findIndex(isMana) == 1);
    CHECK(bag.countIf(isPotion) == 2);
    CHECK(bag.getItem(1) == c);
    CHECK(throwsAs<std::out_of_range>([&] { bag.getItem(3); }));

    // Da hinh: goi qua con tro co so, ra hanh vi cua tung lop con
    Player p("P", Stats{ 100, 50, 10, 10, 5, 5, 10 }, Element::KIM);
    p.takeDamage(50); p.consumeMp(50);
    CHECK(bag.getItem(0)->use(p) == 40);     // Potion
    CHECK(bag.getItem(1)->use(p) == 30);     // ManaPotion

    CHECK(bag.removeItem(a) && bag.size() == 2);
    CHECK(!bag.removeItem(a));               // da xoa roi
    CHECK(bag.removeAt(0) && bag.size() == 1 && bag.getItem(0) == b);   // a da xoa, c o vi tri 0
    CHECK(!bag.removeAt(5));
    bag.clear();
    CHECK(bag.isEmpty());

    // Template van dung duoc voi kieu khac (khong pha vo API cu)
    Inventory<std::string> names;
    names.addItem("x");
    CHECK(names.getItems().size() == 1 && names.removeItem("x"));
}

static void testParty(const GameData& data) {
    Party party;
    CHECK(party.isEmpty() && !party.isFull() && party.getLeader() == nullptr);
    CHECK(party.isDefeated());

    std::shared_ptr<Player> h1 = CharacterFactory::createHero(data, "H1", "Human", "Swordman", Element::KIM);
    std::shared_ptr<Player> h2 = CharacterFactory::createHero(data, "H2", "Elf", "Mage", Element::THUY);
    std::shared_ptr<Player> h3 = CharacterFactory::createHero(data, "H3", "Ogre", "Tanker", Element::THO);
    std::shared_ptr<Player> h4 = CharacterFactory::createHero(data, "H4", "Oni", "Assassin", Element::HOA);

    CHECK(party.addMember(h1));
    CHECK(!party.addMember(h1));             // trung -> tu choi
    CHECK(!party.addMember(nullptr));
    CHECK(party.addMember(h2));
    CHECK(!party.isFull());
    CHECK(party.addMember(h3));
    CHECK(party.isFull() && party.size() == Party::MAX_SIZE);
    CHECK(!party.addMember(h4));             // toi da 3
    CHECK(party.size() == 3);

    CHECK(party.getLeader() == h1.get());
    CHECK(party.getMember(2) == h3.get());
    CHECK(party.getMember(3) == nullptr);

    CHECK(!party.isDefeated());
    h1->takeDamage(9999); h2->takeDamage(9999);
    CHECK(!party.isDefeated());              // con h3
    h3->takeDamage(9999);
    CHECK(party.isDefeated());

    // Rut thanh vien -> co cho trong de them nguoi moi
    CHECK(party.removeMember(5) == nullptr);
    CHECK(party.removeMember(0) == h1);
    CHECK(party.getLeader() == h2.get());    // dong doi len dau
    CHECK(party.addMember(h4) && party.isFull());

    // Tui do dung chung cua doi
    party.getInventory().addItem(std::make_shared<Potion>());
    CHECK(party.getInventory().size() == 1);
    party.clear();
    CHECK(party.isEmpty() && party.getInventory().isEmpty());
}

static void testEnemyStillWorks() {
    // Character::restoreMp/getter khong lam hong lop con khac
    Enemy e("Goblin", Stats{ 60, 0, 28, 0, 5, 2, 10 }, Element::MOC);
    CHECK(e.getMaxHp() == 60 && e.getMaxMp() == 0);
    CHECK(e.restoreMp(10) == 0);
}

int main(int argc, char** argv) {
    const std::string dir = argc > 1 ? argv[1] : "data";
    GameData data;
    data.loadAll(dir);

    testFactory(data);
    testMpRecovery();
    testItems();
    testInventory();
    testParty(data);
    testEnemyStillWorks();

    std::cout << (g_failed == 0 ? "ALL TESTS PASSED\n" : "SOME TESTS FAILED\n");
    return g_failed == 0 ? 0 : 1;
}
