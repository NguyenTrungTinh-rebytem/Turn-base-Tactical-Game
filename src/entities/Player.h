#pragma once
#include <string>
#include <vector>
#include "entities/Character.h"
#include "data/Skill.h"

class Player : public Character
{
public:
    Player(const std::string& name, const Stats& stats, Element element,
           const std::string& raceName = "", const std::string& className = "");

    void executeSkill(Skill* skill, Character* target) override;

    // Kỹ năng của Hero (Factory nạp khi khởi tạo)
    void addSkill(const Skill& skill) { m_skills.push_back(skill); }
    const std::vector<Skill>& getSkills() const { return m_skills; }

    // Getter cho màn hình Chi tiết
    const std::string& getRaceName() const  { return m_raceName; }
    const std::string& getClassName() const { return m_className; }

private:
    std::string        m_raceName;
    std::string        m_className;
    std::vector<Skill> m_skills;
};
