#include "CharacterRules.h"

namespace {

// Bang tra cuu [race][class] - true = duoc phep.
// Thu tu cot: MAGE, SWORDMAN, ASSASSIN, TANKER, ENCHANTER, PALADIN
constexpr bool VALID_TABLE[RACE_COUNT][CLASS_COUNT] = {
    //            MAGE   SWORD  ASSAS  TANKER ENCH   PALADIN
    /* OGRE     */ { false, true,  true,  true,  false, false },
    /* HUMAN    */ { true,  true,  true,  false, true,  true  },
    /* DWARF    */ { true,  true,  false, true,  true,  false },
    /* ELF      */ { true,  true,  true,  false, true,  true  },
    /* ONI      */ { false, true,  true,  true,  false, false },
    /* BEASTMAN */ { false, true,  true,  true,  false, false },
};

constexpr const char* RACE_NAMES[RACE_COUNT] = {
    "ogre", "human", "dwarf", "elf", "oni", "beastman"
};
constexpr const char* CLASS_NAMES[CLASS_COUNT] = {
    "mage", "swordman", "assassin", "tanker", "enchanter", "paladin"
};

bool inRange(int value, int count) { return value >= 0 && value < count; }

} // namespace

bool isValidCombination(RaceType race, ClassType pClass)
{
    const int r = static_cast<int>(race);
    const int c = static_cast<int>(pClass);
    if (!inRange(r, RACE_COUNT) || !inRange(c, CLASS_COUNT)) return false;
    return VALID_TABLE[r][c];
}

std::vector<ClassType> getValidClasses(RaceType race)
{
    std::vector<ClassType> result;
    for (int c = 0; c < CLASS_COUNT; ++c)
        if (isValidCombination(race, static_cast<ClassType>(c)))
            result.push_back(static_cast<ClassType>(c));
    return result;
}

std::vector<RaceType> getValidRaces(ClassType pClass)
{
    std::vector<RaceType> result;
    for (int r = 0; r < RACE_COUNT; ++r)
        if (isValidCombination(static_cast<RaceType>(r), pClass))
            result.push_back(static_cast<RaceType>(r));
    return result;
}

ClassType stepValidClass(RaceType race, ClassType current, int direction)
{
    const int step = direction >= 0 ? 1 : -1;
    int c = static_cast<int>(current);
    for (int i = 0; i < CLASS_COUNT; ++i) {
        c = (c + step + CLASS_COUNT) % CLASS_COUNT;
        if (isValidCombination(race, static_cast<ClassType>(c)))
            return static_cast<ClassType>(c);
    }
    return current; // khong co class nao hop le (khong xay ra voi bang hien tai)
}

RaceType stepValidRace(ClassType pClass, RaceType current, int direction)
{
    const int step = direction >= 0 ? 1 : -1;
    int r = static_cast<int>(current);
    for (int i = 0; i < RACE_COUNT; ++i) {
        r = (r + step + RACE_COUNT) % RACE_COUNT;
        if (isValidCombination(static_cast<RaceType>(r), pClass))
            return static_cast<RaceType>(r);
    }
    return current;
}

const char* raceToString(RaceType race)
{
    const int r = static_cast<int>(race);
    return inRange(r, RACE_COUNT) ? RACE_NAMES[r] : "unknown";
}

const char* classToString(ClassType pClass)
{
    const int c = static_cast<int>(pClass);
    return inRange(c, CLASS_COUNT) ? CLASS_NAMES[c] : "unknown";
}
