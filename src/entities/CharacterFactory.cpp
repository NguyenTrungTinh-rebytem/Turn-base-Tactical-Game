#include "entities/CharacterFactory.h"
#include <stdexcept>

std::unique_ptr<Player> CharacterFactory::createHero(const GameData& data,
                                                     const std::string& name,
                                                     const std::string& raceName,
                                                     const std::string& className,
                                                     Element element)
{
    if (!data.isLoaded())
        throw std::invalid_argument("CharacterFactory::createHero: GameData chua nap");
    if (name.empty())
        throw std::invalid_argument("CharacterFactory::createHero: ten Hero rong");

    // Lấy hết dữ liệu trước khi tạo object: lỗi thì không tạo ra Hero dở dang
    const Stats stats = data.getBaseStats(raceName, className);
    std::vector<Skill> skills;
    for (const auto& id : data.getHeroSkillIds(raceName, className))
        skills.push_back(toBattleSkill(data.getSkill(id), element));

    auto hero = std::make_unique<Player>(name, stats, element, raceName, className);
    for (const auto& skill : skills)
        hero->addSkill(skill);
    return hero;
}

Skill CharacterFactory::toBattleSkill(const SkillData& data, Element heroElement)
{
    // CSV chưa có cột element/isMagic nên suy ra tạm từ hiệu ứng và chủ sở hữu.
    Skill skill;
    skill.m_id = data.m_id;
    skill.m_name = data.m_name;
    skill.m_ownerType = data.m_ownerType;
    skill.m_owner = data.m_owner;
    skill.m_type = data.m_type;
    skill.m_power = data.m_power;     // DAMAGE/DEBUFF: % của ATK; HEAL: lượng HP hồi cố định
    skill.m_mpCost = data.m_mpCost;
    skill.m_effect = data.m_effect;
    skill.m_effectTurns = data.m_effectTurns;

    switch (data.m_effect)
    {
    case EffectType::BURN:     skill.m_element = Element::HOA;  break;
    case EffectType::SLOW:     skill.m_element = Element::THUY; break;
    case EffectType::STUN:     skill.m_element = Element::THO;  break;
    case EffectType::WEAKNESS: skill.m_element = Element::MOC;  break;
    default:                   skill.m_element = heroElement;   break;
    }

    skill.m_isMagic = (data.m_ownerType == SkillOwnerType::CLASS)
        ? (data.m_owner == "Mage" || data.m_owner == "Enchanter")
        : (data.m_owner == "Elf");
    return skill;
}
