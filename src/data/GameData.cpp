#include "GameData.h"
#include <fstream>
#include <sstream>

namespace {

struct CsvRow {
    int m_lineNo = 0;
    std::vector<std::string> m_fields;
};

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// Doc file CSV: bo dong trong, dong bat dau bang '#' va dong header (dong du lieu dau tien
// la dong '#' mo ta cot, nen khong co header rieng).
std::vector<CsvRow> readCsv(const std::string& path, size_t expectedFields) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw DataLoadError("Khong tim thay hoac khong mo duoc file du lieu: '" + path +
                            "'. Kiem tra thu muc data/ va duong dan chay game.");
    }
    std::vector<CsvRow> rows;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        CsvRow row;
        row.m_lineNo = lineNo;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) row.m_fields.push_back(trim(cell));
        if (!line.empty() && line.back() == ',') row.m_fields.push_back("");

        if (row.m_fields.size() != expectedFields) {
            throw DataLoadError(path + ":" + std::to_string(lineNo) + ": can " +
                                std::to_string(expectedFields) + " cot nhung tim thay " +
                                std::to_string(row.m_fields.size()));
        }
        rows.push_back(row);
    }
    if (rows.empty()) throw DataLoadError("File du lieu rong (khong co dong du lieu nao): '" + path + "'");
    return rows;
}

int toInt(const std::string& path, const CsvRow& row, size_t col) {
    const std::string& s = row.m_fields[col];
    try {
        size_t pos = 0;
        int v = std::stoi(s, &pos);
        if (pos != s.size()) throw std::invalid_argument("trailing");
        return v;
    } catch (const std::exception&) {
        throw DataLoadError(path + ":" + std::to_string(row.m_lineNo) + ": cot " +
                            std::to_string(col + 1) + " phai la so nguyen, nhan duoc '" + s + "'");
    }
}

[[noreturn]] void badValue(const std::string& path, const CsvRow& row, size_t col, const char* kind) {
    throw DataLoadError(path + ":" + std::to_string(row.m_lineNo) + ": gia tri " + kind + " khong hop le '" +
                        row.m_fields[col] + "' (cot " + std::to_string(col + 1) + ")");
}

Element toElement(const std::string& path, const CsvRow& row, size_t col) {
    const std::string& s = row.m_fields[col];
    if (s == "KIM") return Element::KIM;
    if (s == "MOC") return Element::MOC;
    if (s == "THUY") return Element::THUY;
    if (s == "HOA") return Element::HOA;
    if (s == "THO") return Element::THO;
    badValue(path, row, col, "Element (KIM|MOC|THUY|HOA|THO)");
}

EffectType toEffect(const std::string& path, const CsvRow& row, size_t col) {
    const std::string& s = row.m_fields[col];
    if (s == "NONE") return EffectType::NONE;
    if (s == "ATK_BOOST") return EffectType::ATK_BOOST;
    if (s == "DEF_BOOST") return EffectType::DEF_BOOST;
    if (s == "REGEN") return EffectType::REGEN;
    if (s == "STUN") return EffectType::STUN;
    if (s == "BURN") return EffectType::BURN;
    if (s == "SLOW") return EffectType::SLOW;
    if (s == "WEAKNESS") return EffectType::WEAKNESS;
    badValue(path, row, col, "Effect");
}

SkillType toSkillType(const std::string& path, const CsvRow& row, size_t col) {
    const std::string& s = row.m_fields[col];
    if (s == "DAMAGE") return SkillType::DAMAGE;
    if (s == "HEAL") return SkillType::HEAL;
    if (s == "BUFF") return SkillType::BUFF;
    if (s == "DEBUFF") return SkillType::DEBUFF;
    badValue(path, row, col, "SkillType (DAMAGE|HEAL|BUFF|DEBUFF)");
}

Stats toStats(const std::string& path, const CsvRow& row, size_t firstCol) {
    Stats st;
    st.m_hp = toInt(path, row, firstCol);
    st.m_mp = toInt(path, row, firstCol + 1);
    st.m_spd = toInt(path, row, firstCol + 2);
    st.m_a = toInt(path, row, firstCol + 3);
    st.m_ma = toInt(path, row, firstCol + 4);
    st.m_d = toInt(path, row, firstCol + 5);
    st.m_md = toInt(path, row, firstCol + 6);
    return st;
}

template <typename T, typename Pred>
const T& findOrThrow(const std::vector<T>& v, Pred pred, const std::string& what) {
    for (const auto& item : v) {
        if (pred(item)) return item;
    }
    throw std::out_of_range("GameData: khong tim thay " + what);
}

}  // namespace

void GameData::loadAll(const std::string& dataDir) {
    m_loaded = false;
    m_races.clear();
    m_classes.clear();
    m_skills.clear();
    m_monsters.clear();

    std::string dir = dataDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';

    loadRaces(dir + "races.csv");
    loadClasses(dir + "classes.csv");
    loadSkills(dir + "skills.csv");
    loadMonsters(dir + "monsters.csv");
    validate();
    m_loaded = true;
}

void GameData::loadRaces(const std::string& path) {
    for (const auto& row : readCsv(path, 10)) {
        RaceData r;
        r.m_name = row.m_fields[0];
        r.m_stats = toStats(path, row, 1);
        r.m_skillIds = { row.m_fields[8], row.m_fields[9] };
        m_races.push_back(r);
    }
}

void GameData::loadClasses(const std::string& path) {
    for (const auto& row : readCsv(path, 10)) {
        ClassData c;
        c.m_name = row.m_fields[0];
        c.m_stats = toStats(path, row, 1);
        c.m_skillIds = { row.m_fields[8], row.m_fields[9] };
        m_classes.push_back(c);
    }
}

void GameData::loadSkills(const std::string& path) {
    for (const auto& row : readCsv(path, 9)) {
        SkillData s;
        s.m_id = row.m_fields[0];
        s.m_name = row.m_fields[1];
        if (row.m_fields[2] == "RACE") s.m_ownerType = SkillOwnerType::RACE;
        else if (row.m_fields[2] == "CLASS") s.m_ownerType = SkillOwnerType::CLASS;
        else badValue(path, row, 2, "ownerType (RACE|CLASS)");
        s.m_owner = row.m_fields[3];
        s.m_type = toSkillType(path, row, 4);
        s.m_power = toInt(path, row, 5);
        s.m_mpCost = toInt(path, row, 6);
        s.m_effect = toEffect(path, row, 7);
        s.m_effectTurns = toInt(path, row, 8);
        m_skills.push_back(s);
    }
}

void GameData::loadMonsters(const std::string& path) {
    for (const auto& row : readCsv(path, 12)) {
        MonsterData m;
        m.m_id = row.m_fields[0];
        m.m_name = row.m_fields[1];
        m.m_element = toElement(path, row, 2);
        m.m_isBoss = toInt(path, row, 3) != 0;
        m.m_stage = toInt(path, row, 4);
        m.m_stats = toStats(path, row, 5);
        m_monsters.push_back(m);
    }
}

void GameData::validate() const {
    auto checkCount = [](const char* what, size_t got, int expected) {
        if (got != static_cast<size_t>(expected)) {
            throw DataLoadError(std::string("Sai so luong ") + what + ": can " + std::to_string(expected) +
                                ", tim thay " + std::to_string(got));
        }
    };
    checkCount("Race", m_races.size(), EXPECTED_RACES);
    checkCount("Class", m_classes.size(), EXPECTED_CLASSES);
    checkCount("Skill", m_skills.size(), EXPECTED_SKILLS);

    auto checkSkillRefs = [this](const std::string& ownerName, SkillOwnerType type,
                                 const std::vector<std::string>& ids) {
        if (ids.size() != 2) throw DataLoadError(ownerName + " phai co dung 2 skill");
        for (const auto& id : ids) {
            const SkillData* found = nullptr;
            for (const auto& s : m_skills) {
                if (s.m_id == id) { found = &s; break; }
            }
            if (!found) throw DataLoadError(ownerName + " tham chieu skill khong ton tai: '" + id + "'");
            if (found->m_ownerType != type || found->m_owner != ownerName) {
                throw DataLoadError("Skill '" + id + "' khong thuoc ve '" + ownerName + "' (kiem tra skills.csv)");
            }
        }
    };
    for (const auto& r : m_races) checkSkillRefs(r.m_name, SkillOwnerType::RACE, r.m_skillIds);
    for (const auto& c : m_classes) checkSkillRefs(c.m_name, SkillOwnerType::CLASS, c.m_skillIds);

    bool hasStage1 = false, hasBoss = false;
    for (const auto& m : m_monsters) {
        if (m.m_stage == 1) hasStage1 = true;
        if (m.m_isBoss) hasBoss = true;
    }
    if (!hasStage1) throw DataLoadError("monsters.csv thieu quai thuong cho Stage 1");
    if (!hasBoss) throw DataLoadError("monsters.csv thieu Boss cho Stage 2");
}

const RaceData& GameData::getRace(const std::string& name) const {
    return findOrThrow(m_races, [&](const RaceData& r) { return r.m_name == name; }, "Race '" + name + "'");
}

const ClassData& GameData::getClass(const std::string& name) const {
    return findOrThrow(m_classes, [&](const ClassData& c) { return c.m_name == name; }, "Class '" + name + "'");
}

const SkillData& GameData::getSkill(const std::string& id) const {
    return findOrThrow(m_skills, [&](const SkillData& s) { return s.m_id == id; }, "Skill '" + id + "'");
}

const MonsterData& GameData::getMonster(const std::string& id) const {
    return findOrThrow(m_monsters, [&](const MonsterData& m) { return m.m_id == id; }, "Monster '" + id + "'");
}

Stats GameData::getBaseStats(const std::string& raceName, const std::string& className) const {
    return getRace(raceName).m_stats + getClass(className).m_stats;
}

std::vector<std::string> GameData::getHeroSkillIds(const std::string& raceName,
                                                   const std::string& className) const {
    std::vector<std::string> ids = getRace(raceName).m_skillIds;
    const auto& classSkills = getClass(className).m_skillIds;
    ids.insert(ids.end(), classSkills.begin(), classSkills.end());
    return ids;
}

std::vector<MonsterData> GameData::getMonstersByStage(int stage) const {
    std::vector<MonsterData> result;
    for (const auto& m : m_monsters) {
        if (m.m_stage == stage) result.push_back(m);
    }
    return result;
}

double GameData::getElementMultiplier(Element attacker, Element defender) {
    // Tuong khac: Kim>Moc>Tho>Thuy>Hoa>Kim (index theo thu tu enum: KIM,MOC,THUY,HOA,THO)
    static const int BEATS[5] = { 1, 4, 3, 0, 2 };
    int a = static_cast<int>(attacker);
    int d = static_cast<int>(defender);
    if (BEATS[a] == d) return 1.5;
    if (BEATS[d] == a) return 0.5;
    return 1.0;
}
