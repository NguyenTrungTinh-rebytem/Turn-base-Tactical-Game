#pragma once
// ============================================================================
//  HeroPresetConfig - danh sach tuong co san, doc tu data/heroes.csv
//  Dong: id,name,race,class,element     (# la chu thich; dong dau "id,..." duoc bo qua)
//  Vi du: knight,Hiep Si,Human,Swordman,KIM
// ============================================================================
#include <string>
#include <vector>
#include "Element.h"

struct HeroPresetData {
    std::string m_id;
    std::string m_name;
    std::string m_race;     // ten trong races.csv
    std::string m_class;    // ten trong classes.csv
    Element     m_element = Element::KIM;
};

// Doc file. Tra ve so tuong doc duoc (0 neu thieu file / file rong). Bo qua dong loi.
int loadHeroPresets(const std::string& path, std::vector<HeroPresetData>& out);

// Danh sach du phong khi chua co heroes.csv
std::vector<HeroPresetData> defaultHeroPresets();
