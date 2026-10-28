#include "entities/ItemDatabase.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>

namespace {

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    return s.substr(b, s.find_last_not_of(ws) - b + 1);
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Tách 1 dòng CSV, hỗ trợ "ô có, dấu phẩy" và "" (nháy kép thoát)
std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string cell;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { cell += '"'; ++i; }
                else inQuotes = false;
            } else cell += c;
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(trim(cell));
            cell.clear();
        } else cell += c;
    }
    fields.push_back(trim(cell));
    return fields;
}

struct EffectName { const char* name; ItemEffect effect; };
const EffectName EFFECT_TABLE[] = {
    { "HEAL_HP_PCT", ItemEffect::HEAL_HP_PCT },           { "RESTORE_MP_PCT", ItemEffect::RESTORE_MP_PCT },
    { "REVIVE_PCT", ItemEffect::REVIVE_PCT },             { "BOOST_DAMAGE_UP", ItemEffect::BOOST_DAMAGE_UP },
    { "BOOST_DAMAGE_REDUCE", ItemEffect::BOOST_DAMAGE_REDUCE }, { "BOOST_EXTRA_TURN", ItemEffect::BOOST_EXTRA_TURN },
    { "BOOST_CRIT_RATE", ItemEffect::BOOST_CRIT_RATE },   { "HELD_REGEN", ItemEffect::HELD_REGEN },
    { "HELD_LIFESTEAL", ItemEffect::HELD_LIFESTEAL },     { "HELD_SURVIVE", ItemEffect::HELD_SURVIVE },
    { "HELD_DAMAGE_REDUCE", ItemEffect::HELD_DAMAGE_REDUCE }, { "HELD_BERSERK", ItemEffect::HELD_BERSERK },
    { "HELD_EXECUTE", ItemEffect::HELD_EXECUTE },         { "HELD_CRIT_DAMAGE", ItemEffect::HELD_CRIT_DAMAGE },
    { "HELD_COUNTER", ItemEffect::HELD_COUNTER },
};

std::vector<ItemData> parseFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
        throw ItemLoadError("Khong tim thay hoac khong mo duoc file vat pham: '" + path +
                            "'. Kiem tra thu muc data/ va duong dan chay game.");

    std::map<std::string, size_t> column;   // ten cot (viet thuong) -> vi tri
    size_t columnCount = 0;
    std::vector<ItemData> items;
    std::string line;
    int lineNo = 0;
    bool first = true;

    while (std::getline(in, line)) {
        ++lineNo;
        if (first) {   // bo BOM UTF-8 (Excel them vao dau file)
            first = false;
            if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF)
                line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        const std::vector<std::string> fields = splitCsvLine(line);
        const std::string where = path + ":" + std::to_string(lineNo) + ": ";

        if (column.empty()) {   // dong dau tien co nghia = header
            for (size_t i = 0; i < fields.size(); ++i) column[lower(fields[i])] = i;
            columnCount = fields.size();
            for (const char* required : { "id", "name", "category", "effect", "value" })
                if (!column.count(required))
                    throw ItemLoadError(where + "header thieu cot bat buoc '" + required + "'");
            continue;
        }

        if (fields.size() != columnCount)
            throw ItemLoadError(where + "can " + std::to_string(columnCount) + " cot (theo header) nhung tim thay " +
                                std::to_string(fields.size()) + " - neu o co dau phay hay dat trong \"nhay kep\"");

        auto text = [&](const char* name) -> std::string {
            auto it = column.find(lower(name));
            return it == column.end() ? std::string() : fields[it->second];
        };
        auto number = [&](const char* name, bool required) -> int {
            const std::string s = text(name);
            if (s.empty()) {
                if (required) throw ItemLoadError(where + "cot '" + name + "' khong duoc de trong");
                return 0;
            }
            try {
                size_t pos = 0;
                const int v = std::stoi(s, &pos);
                if (pos != s.size()) throw std::invalid_argument("trailing");
                return v;
            } catch (const std::exception&) {
                throw ItemLoadError(where + "cot '" + name + "' phai la so nguyen, nhan duoc '" + s + "'");
            }
        };

        ItemData d;
        d.m_id = text("id");
        d.m_name = text("name");
        d.m_nameVi = text("nameVi");
        d.m_descriptionVi = text("description");
        if (d.m_id.empty()) throw ItemLoadError(where + "cot 'id' khong duoc de trong");
        if (d.m_name.empty()) throw ItemLoadError(where + "cot 'name' khong duoc de trong");
        if (d.m_nameVi.empty()) d.m_nameVi = d.m_name;

        const std::string cat = text("category");
        if (cat == "CONSUMABLE") d.m_category = ItemCategory::CONSUMABLE;
        else if (cat == "BOOSTER") d.m_category = ItemCategory::BOOSTER;
        else if (cat == "HELD") d.m_category = ItemCategory::HELD;
        else throw ItemLoadError(where + "category khong hop le '" + cat + "' (CONSUMABLE|BOOSTER|HELD)");

        const std::string eff = text("effect");
        bool found = false;
        for (const auto& e : EFFECT_TABLE)
            if (eff == e.name) { d.m_effect = e.effect; found = true; break; }
        if (!found) throw ItemLoadError(where + "effect khong hop le '" + eff + "'");
        if (categoryOfEffect(d.m_effect) != d.m_category)
            throw ItemLoadError(where + "effect '" + eff + "' khong thuoc nhom '" + cat + "'");

        d.m_value = number("value", true);
        d.m_value2 = number("value2", false);
        d.m_duration = number("duration", false);
        d.m_maxCarry = number("maxCarry", false);

        if (d.m_value <= 0) throw ItemLoadError(where + "value phai > 0");
        if (d.m_value2 < 0 || d.m_duration < 0 || d.m_maxCarry < 0)
            throw ItemLoadError(where + "value2/duration/maxCarry khong duoc am");

        const bool timed = d.m_effect == ItemEffect::BOOST_DAMAGE_UP || d.m_effect == ItemEffect::BOOST_DAMAGE_REDUCE;
        if (timed && d.m_duration <= 0) throw ItemLoadError(where + "Booster co thoi han phai co duration > 0");
        if ((d.m_effect == ItemEffect::HELD_BERSERK || d.m_effect == ItemEffect::HELD_EXECUTE) && d.m_value2 <= 0)
            throw ItemLoadError(where + "effect nay can value2 > 0");
        const bool isReduce = d.m_effect == ItemEffect::BOOST_DAMAGE_REDUCE || d.m_effect == ItemEffect::HELD_DAMAGE_REDUCE;
        if (isReduce && d.m_value > 100) throw ItemLoadError(where + "% giam sat thuong khong duoc > 100");
        if (d.m_effect == ItemEffect::HELD_EXECUTE && d.m_value2 > 100) throw ItemLoadError(where + "nguong % HP khong duoc > 100");

        for (const auto& other : items)
            if (other.m_id == d.m_id) throw ItemLoadError(where + "trung id vat pham '" + d.m_id + "'");
        items.push_back(d);
    }

    if (column.empty()) throw ItemLoadError("File vat pham rong (khong co header): '" + path + "'");
    if (items.empty()) throw ItemLoadError("File vat pham khong co dong du lieu nao: '" + path + "'");
    return items;
}

} // namespace

void ItemDatabase::load(const std::string& dataDir) {
    std::string dir = dataDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';
    const std::string path = dir + "items.csv";

    std::vector<ItemData> parsed = parseFile(path);   // loi -> nem, du lieu cu khong bi mat
    m_items = std::move(parsed);
    m_dataDir = dataDir;
    m_sourcePath = path;
    m_loaded = true;
}

bool ItemDatabase::tryLoad(const std::string& dataDir, std::string* error) {
    try {
        load(dataDir);
        return true;
    } catch (const std::exception& e) {
        if (error) *error = e.what();
        return false;
    }
}

void ItemDatabase::reload() {
    load(m_dataDir);
}

const ItemData* ItemDatabase::findItem(const std::string& id) const {
    for (const auto& item : m_items)
        if (item.m_id == id) return &item;
    return nullptr;
}

const ItemData& ItemDatabase::getItem(const std::string& id) const {
    if (const ItemData* item = findItem(id)) return *item;
    throw std::out_of_range("ItemDatabase: khong tim thay vat pham '" + id + "'");
}

std::vector<ItemData> ItemDatabase::getItemsByCategory(ItemCategory category) const {
    std::vector<ItemData> result;
    for (const auto& item : m_items)
        if (item.m_category == category) result.push_back(item);
    return result;
}
