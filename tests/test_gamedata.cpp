// Test don gian cho Sprint 1 (khong can Raylib).
// Build: g++ -std=c++17 -I src/data tests/test_gamedata.cpp src/data/GameData.cpp -o test_gamedata
// Chay : ./test_gamedata data
#include <iostream>
#include "GameData.h"

static int g_failed = 0;
#define CHECK(cond) do { if (!(cond)) { ++g_failed; std::cout << "[FAIL] " #cond " (line " << __LINE__ << ")\n"; } } while (0)

int main(int argc, char** argv) {
    std::string dir = argc > 1 ? argv[1] : "data";

    GameData data;
    data.loadAll(dir);
    CHECK(data.getRaces().size() == 6);
    CHECK(data.getClasses().size() == 6);
    CHECK(data.getSkills().size() == 24);
    CHECK(data.getHeroSkillIds("Elf", "Mage").size() == 4);

    Stats s = data.getBaseStats("Human", "Swordman");
    CHECK(s.m_hp == 115 && s.m_spd == 12 && s.m_a == 22);

    CHECK(GameData::getElementMultiplier(Element::KIM, Element::MOC) == 1.5);
    CHECK(GameData::getElementMultiplier(Element::MOC, Element::KIM) == 0.5);
    CHECK(GameData::getElementMultiplier(Element::KIM, Element::KIM) == 1.0);
    CHECK(GameData::getElementMultiplier(Element::THO, Element::THUY) == 1.5);
    CHECK(GameData::getElementMultiplier(Element::HOA, Element::KIM) == 1.5);

    CHECK(data.getMonstersByStage(1).size() == 5);
    CHECK(data.getMonstersByStage(2).size() == 1);

    // Thieu file / thieu thu muc -> phai nem DataLoadError voi thong bao ro rang
    bool threw = false;
    try {
        GameData bad;
        bad.loadAll("thu_muc_khong_ton_tai");
    } catch (const DataLoadError& e) {
        threw = true;
        std::cout << "Thong bao loi mong doi: " << e.what() << "\n";
    }
    CHECK(threw);

    std::cout << (g_failed == 0 ? "ALL TESTS PASSED\n" : "SOME TESTS FAILED\n");
    return g_failed == 0 ? 0 : 1;
}
