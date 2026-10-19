#pragma once
#include <memory>
#include <string>
#include <utility>
#include "data/Element.h"
#include "data/Stats.h"

struct Skill;

class Character
{
public:
    Character(const std::string& name, const Stats& stats, Element element);
    virtual ~Character() = default;

    virtual void takeDamage(int amount, Element atkElement, bool isMagic);
    virtual void executeSkill(Skill* skill, Character* target) = 0;

    const std::string& getName() const      { return m_name; }
    const Stats&       getStats() const     { return m_stats; }
    Element            getElement() const   { return m_element; }
    int                getCurrentHp() const { return m_currentHp; }
    int                getCurrentMp() const { return m_currentMp; }
    int                getSpeed() const     { return m_stats.m_spd; }
    bool               isAlive() const      { return m_currentHp > 0; }

protected:
    bool canAct(const Skill* skill, const Character* target) const;
    int  calcSkillDamage(const Skill* skill) const;

    std::string m_name;
    Stats       m_stats;
    Element     m_element;
    int         m_currentHp;
    int         m_currentMp;
};

using CharacterPtr = std::unique_ptr<Character>;

template <typename T, typename... Args>
CharacterPtr makeCharacter(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
