#pragma once
// ============================================================================
//  SkillElementConfig - gan / sua nguyen to (Kim-Moc-Thuy-Hoa-Tho) cho tung skill
//  Luu rieng trong data/skill_elements.csv nen KHONG dong vao skills.csv.
//  Skill nao chua duoc gan thi getElement() tra ve std::nullopt.
// ============================================================================
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "Element.h"   // enum class Element + ELEMENT_MATRIX cua du an

const char*             elementToString(Element element);
std::optional<Element>  elementFromString(const std::string& text);  // khong phan biet hoa/thuong

class SkillElementConfig {
public:
    // Gan hoac DOI nguyen to cua skill. false neu skillId rong.
    bool setElement(const std::string& skillId, Element element);

    // Xoa gan ket -> skill tro ve khong nguyen to. false neu chua tung gan.
    bool clearElement(const std::string& skillId);

    std::optional<Element> getElement(const std::string& skillId) const;
    Element getElementOr(const std::string& skillId, Element fallback) const;
    bool    hasElement(const std::string& skillId) const;

    // Doi nguyen to cho NHIEU skill cung luc (vd. tat ca skill cua 1 class).
    int setElements(const std::vector<std::string>& skillIds, Element element);

    // File CSV: dong "skillId,ELEMENT", dong bat dau '#' la chu thich.
    // load: bo qua dong loi, tra ve so dong doc duoc. save: ghi de ca file.
    int  loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

    const std::map<std::string, Element>& getAll() const { return m_elements; }

private:
    std::map<std::string, Element> m_elements;   // sap theo id -> file ghi ra on dinh
};
