#include "SkillElementConfig.h"
#include <algorithm>
#include <cctype>
#include <fstream>

namespace {
constexpr const char* ELEMENT_NAMES[] = { "KIM", "MOC", "THUY", "HOA", "THO" };

std::string trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}
} // namespace

const char* elementToString(Element element)
{
    const int i = static_cast<int>(element);
    return (i >= 0 && i < 5) ? ELEMENT_NAMES[i] : "UNKNOWN";
}

std::optional<Element> elementFromString(const std::string& text)
{
    std::string up = trim(text);
    std::transform(up.begin(), up.end(), up.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    for (int i = 0; i < 5; ++i)
        if (up == ELEMENT_NAMES[i]) return static_cast<Element>(i);
    return std::nullopt;
}

bool SkillElementConfig::setElement(const std::string& skillId, Element element)
{
    const std::string id = trim(skillId);
    if (id.empty()) return false;
    m_elements[id] = element;
    return true;
}

bool SkillElementConfig::clearElement(const std::string& skillId)
{
    return m_elements.erase(trim(skillId)) > 0;
}

std::optional<Element> SkillElementConfig::getElement(const std::string& skillId) const
{
    auto it = m_elements.find(trim(skillId));
    if (it == m_elements.end()) return std::nullopt;
    return it->second;
}

Element SkillElementConfig::getElementOr(const std::string& skillId, Element fallback) const
{
    return getElement(skillId).value_or(fallback);
}

bool SkillElementConfig::hasElement(const std::string& skillId) const
{
    return m_elements.count(trim(skillId)) > 0;
}

int SkillElementConfig::setElements(const std::vector<std::string>& skillIds, Element element)
{
    int count = 0;
    for (const auto& id : skillIds)
        if (setElement(id, element)) ++count;
    return count;
}

int SkillElementConfig::loadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in) return 0;
    int loaded = 0;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        const size_t comma = line.find(',');
        if (comma == std::string::npos) continue;
        auto element = elementFromString(line.substr(comma + 1));
        if (!element) continue;                       // nguyen to sai -> bo dong
        if (setElement(line.substr(0, comma), *element)) ++loaded;
    }
    return loaded;
}

bool SkillElementConfig::saveToFile(const std::string& path) const
{
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << "# skillId,element(KIM|MOC|THUY|HOA|THO)\n";
    for (const auto& [id, element] : m_elements)
        out << id << ',' << elementToString(element) << '\n';
    return static_cast<bool>(out);
}
