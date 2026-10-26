#pragma once
#include <memory>
#include <string>
#include "data/GameData.h"
#include "data/Skill.h"
#include "entities/Player.h"

// Factory Method: tạo Hero từ dữ liệu CSV (Race + Class).
class CharacterFactory
{
public:
    // Base Stats = Race Stats + Class Stats; nạp đủ 4 skill (2 Race + 2 Class).
    // Ném std::invalid_argument nếu data chưa nạp hoặc tên rỗng,
    //     std::out_of_range nếu Race/Class/Skill không tồn tại trong data.
    static std::unique_ptr<Player> createHero(const GameData& data,
                                              const std::string& name,
                                              const std::string& raceName,
                                              const std::string& className,
                                              Element element);

    // SkillData (CSV) -> Skill dùng trong chiến đấu
    static Skill toBattleSkill(const SkillData& data, Element heroElement);
};
