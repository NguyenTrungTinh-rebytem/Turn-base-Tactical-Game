#include "core/AttackCommand.h"
#include "core/UseItemCommand.h"
#include "entities/CharacterFactory.h"
#include "entities/Party.h"
#include "entities/ItemDatabase.h"
#include "entities/ItemFactory.h"
#include "entities/Enemy.h"
#include <cassert>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
    const std::string dataDir = argc > 1 ? argv[1] : "data";
    GameData data;
    data.loadAll(dataDir);
    ItemDatabase db;
    db.load(dataDir);
    const auto& race = data.getRaces().front();
    const auto& job = data.getClasses().front();
    auto leader = CharacterFactory::createHero(data, "Test", race.m_name, job.m_name, Element::KIM);
    assert(leader && leader->getSkills().size() == 4);
    Party party;
    assert(party.addMember(std::shared_ptr<Player>(std::move(leader))));
    assert(party.size() == 1);
    assert(party.getLeader() != nullptr);
    // Three independently configured characters, not auto-generated placeholder members.
    const auto& races = data.getRaces();
    const auto& classes = data.getClasses();
    auto teammate2 = CharacterFactory::createHero(data, "Second",
        races[races.size() > 1 ? 1 : 0].m_name,
        classes[classes.size() > 1 ? 1 : 0].m_name, Element::MOC);
    auto teammate3 = CharacterFactory::createHero(data, "Third",
        races[races.size() > 2 ? 2 : 0].m_name,
        classes[classes.size() > 2 ? 2 : 0].m_name, Element::THUY);
    assert(party.addMember(std::shared_ptr<Player>(std::move(teammate2))));
    assert(party.addMember(std::shared_ptr<Player>(std::move(teammate3))));
    assert(party.size() == Party::MAX_SIZE);
    assert(party.getMember(0)->getName() == "Test");
    assert(party.getMember(1)->getName() == "Second");
    assert(party.getMember(2)->getName() == "Third");

    auto potion = ItemFactory::createItem(db, "ITM_HP_MINOR");
    assert(party.addItem(potion));
    assert(party.countItem("ITM_HP_MINOR") == 1);
    auto* player = party.getLeader();
    player->takeDamage(10);
    int hpBeforePotion = player->getCurrentHp();
    int amount = potion->use(*player);
    assert(amount > 0 && player->getCurrentHp() > hpBeforePotion);
    potion->revert(*player, amount);
    assert(player->getCurrentHp() == hpBeforePotion);
    auto booster = ItemFactory::createItem(db, "ITM_POWER_SERUM");
    assert(booster->use(*player) == 1);
    assert(player->modifiers().hasBuff(BuffType::DAMAGE_UP));
    booster->revert(*player, 1);
    assert(!player->modifiers().hasBuff(BuffType::DAMAGE_UP));
    auto gear = ItemFactory::createItem(db, "ITM_LEFTOVER");
    assert(party.addItem(gear));
    assert(party.equipGear(0, gear));
    assert(player->modifiers().getGear());
    int beforeRegen = player->getCurrentHp();
    player->onTurnEnd();
    assert(player->getCurrentHp() >= beforeRegen);
    assert(party.unequipGear(0));

    Enemy enemy("Enemy", Stats{100, 0, 20, 0, 0, 0, 10}, Element::MOC);
    const int hpPlayer = player->getCurrentHp();
    const int hpEnemy = enemy.getCurrentHp();
    AttackCommand attack(player, &enemy);
    attack.execute();
    assert(enemy.getCurrentHp() < hpEnemy);
    attack.undo();
    assert(enemy.getCurrentHp() == hpEnemy);
    assert(player->getCurrentHp() == hpPlayer);
    std::cout << "CORE INTEGRATION TESTS PASSED\n";
}
