#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "DataTypes.h"
// Loi doc/kiem tra du lieu. what() chua ten file + dong + nguyen nhan.
class DataLoadError : public std::runtime_error {
public:
    explicit DataLoadError(const std::string& message) : std::runtime_error(message) {}
};

class GameData {
public:
    static constexpr int EXPECTED_RACES = 6;
    static constexpr int EXPECTED_CLASSES = 6;
    static constexpr int EXPECTED_SKILLS = 24;

    // Doc races.csv, classes.csv, skills.csv, monsters.csv trong dataDir.
    // Nem DataLoadError neu thieu file, sai dinh dang hoac sai so luong.
    void loadAll(const std::string& dataDir);

    const std::vector<RaceData>& getRaces() const { return m_races; }
    const std::vector<ClassData>& getClasses() const { return m_classes; }
    const std::vector<SkillData>& getSkills() const { return m_skills; }
    const std::vector<MonsterData>& getMonsters() const { return m_monsters; }

    // Nem std::out_of_range neu khong tim thay.
    const RaceData& getRace(const std::string& name) const;
    const ClassData& getClass(const std::string& name) const;
    const SkillData& getSkill(const std::string& id) const;
    const MonsterData& getMonster(const std::string& id) const;

    // Base Stats = Race Stats + Class Stats
    Stats getBaseStats(const std::string& raceName, const std::string& className) const;

    // 4 skill khi khoi tao: 2 cua Race + 2 cua Class (id).
    std::vector<std::string> getHeroSkillIds(const std::string& raceName,
                                             const std::string& className) const;

    std::vector<MonsterData> getMonstersByStage(int stage) const;

    // Ma tran khac he 5x5: 1.5 (khac), 0.5 (bi khac), 1.0 (binh hoa)
    static double getElementMultiplier(Element attacker, Element defender);

    bool isLoaded() const { return m_loaded; }

private:
    void loadRaces(const std::string& path);
    void loadClasses(const std::string& path);
    void loadSkills(const std::string& path);
    void loadMonsters(const std::string& path);
    void validate() const;

    std::vector<RaceData> m_races;
    std::vector<ClassData> m_classes;
    std::vector<SkillData> m_skills;
    std::vector<MonsterData> m_monsters;
    bool m_loaded = false;
};
