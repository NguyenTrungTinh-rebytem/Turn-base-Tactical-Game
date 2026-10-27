#pragma once
// ============================================================================
//  CharacterRules.h - Luat ket hop Race x Class
//  LUU Y: Neu du an da co RaceType / ClassType (vd. trong GameData.h) thi XOA
//  khoi enum ben duoi va #include file do. Thu tu gia tri phai giu nguyen.
// ============================================================================
#include <array>
#include <vector>

enum class RaceType  { OGRE = 0, HUMAN, DWARF, ELF, ONI, BEASTMAN, COUNT };
enum class ClassType { MAGE = 0, SWORDMAN, ASSASSIN, TANKER, ENCHANTER, PALADIN, COUNT };

constexpr int RACE_COUNT  = static_cast<int>(RaceType::COUNT);
constexpr int CLASS_COUNT = static_cast<int>(ClassType::COUNT);

// Kiem tra mot to hop Race + Class co hop le hay khong.
bool isValidCombination(RaceType race, ClassType pClass);

// Danh sach class hop le cua mot race (dung cho menu chon nhan vat).
std::vector<ClassType> getValidClasses(RaceType race);

// Danh sach race hop le cua mot class.
std::vector<RaceType> getValidRaces(ClassType pClass);

// Chuyen sang class hop le ke tiep (direction = +1 / -1), co xoay vong.
// Dung khi nguoi choi bam mui ten trai/phai o menu.
ClassType stepValidClass(RaceType race, ClassType current, int direction);

// Chuyen sang race hop le ke tiep (direction = +1 / -1), co xoay vong.
RaceType stepValidRace(ClassType pClass, RaceType current, int direction);

// Ten dung de ghep duong dan file anh: "ogre", "mage", ...
const char* raceToString(RaceType race);
const char* classToString(ClassType pClass);
