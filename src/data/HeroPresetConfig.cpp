#include "HeroPresetConfig.h"
#include "SkillElementConfig.h"   // elementFromString
#include <fstream>
#include <sstream>

namespace {
std::string trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    const size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
} // namespace

int loadHeroPresets(const std::string& path, std::vector<HeroPresetData>& out)
{
    out.clear();
    std::ifstream file(path);
    if (!file.is_open()) return 0;

    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line.size() >= 3 && (line.compare(0, 3, "id,") == 0 || line.compare(0, 3, "ID,") == 0)) continue;   // dong tieu de

        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) f.push_back(trim(cell));
        if (f.size() < 5 || f[0].empty() || f[1].empty() || f[2].empty() || f[3].empty()) continue;

        const auto element = elementFromString(f[4]);
        if (!element) continue;

        HeroPresetData d;
        d.m_id = f[0];
        d.m_name = f[1];
        d.m_race = f[2];
        d.m_class = f[3];
        d.m_element = *element;
        out.push_back(d);
    }
    return static_cast<int>(out.size());
}

std::vector<HeroPresetData> defaultHeroPresets()
{
    return {
        { "lyra",     "Lyra",     "Elf",      "Mage",      Element::HOA  },
        { "kael",     "Kael",     "Human",    "Assassin",  Element::MOC  },
        { "sylwen",   "Sylwen",   "Elf",      "Enchanter", Element::THUY },
        { "kurogane", "Kurogane", "Oni",      "Swordman",  Element::HOA  },
        { "fenrir",   "Fenrir",   "Beastman", "Swordman",  Element::MOC  },
        { "grumm",    "Grumm",    "Ogre",     "Tanker",    Element::THO  },
        { "thorin",   "Thorin",   "Dwarf",    "Paladin",   Element::KIM  },
        { "aldric",   "Aldric",   "Human",    "Paladin",   Element::THUY },
        { "roland",   "Roland",   "Human",    "Swordman",  Element::KIM  },
        { "brokk",    "Brokk",    "Dwarf",    "Tanker",    Element::THO  },
    };
}
