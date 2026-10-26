#include "entities/ItemFactory.h"
#include <stdexcept>

ItemPtr ItemFactory::createItem(const ItemData& data)
{
    if (categoryOfEffect(data.m_effect) != data.m_category)
        throw std::invalid_argument("ItemFactory: effect khong khop category cua vat pham '" + data.m_id + "'");

    switch (data.m_effect)
    {
    case ItemEffect::HEAL_HP_PCT:    return std::make_shared<Potion>(data);
    case ItemEffect::RESTORE_MP_PCT: return std::make_shared<ManaPotion>(data);
    case ItemEffect::REVIVE_PCT:     return std::make_shared<ReviveItem>(data);

    case ItemEffect::BOOST_DAMAGE_UP:
    case ItemEffect::BOOST_DAMAGE_REDUCE:
    case ItemEffect::BOOST_EXTRA_TURN:
    case ItemEffect::BOOST_CRIT_RATE:
        return std::make_shared<BoosterItem>(data);

    default:
        return std::make_shared<HeldGear>(data);
    }
}

ItemPtr ItemFactory::createItem(const ItemDatabase& db, const std::string& id)
{
    return createItem(db.getItem(id));
}
