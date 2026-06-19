#include "shop.h"
#include <QRandomGenerator>

static UnitStats makeStats(int hp, int atk, int range, int mana)
{
    UnitStats stats;
    stats.maxHp = hp;
    stats.hp = hp;
    stats.atk = atk;
    stats.range = range;
    stats.maxMana = mana;
    stats.mana = 0;
    return stats;
}

Shop::Shop()
{
    m_pool.append({"Knight", 2, 2, makeStats(210, 18, 1, 60), {Trait::Warrior, Trait::Guardian}});
    m_pool.append({"Archer", 1, 1, makeStats(135, 15, 3, 55), {Trait::Ranger}});
    m_pool.append({"Mage", 2, 2, makeStats(120, 12, 3, 55), {Trait::Mage}});
    m_pool.append({"Priest", 2, 2, makeStats(150, 10, 2, 60), {Trait::Healer}});
    m_pool.append({"Stalker", 3, 3, makeStats(135, 22, 1, 65), {Trait::Assassin}});
    m_pool.append({"Paladin", 4, 3, makeStats(260, 17, 1, 75), {Trait::Guardian, Trait::Healer}});
    refresh();
}

void Shop::refresh()
{
    m_offers.clear();
    for (int i = 0; i < 5; ++i) {
        const int index = QRandomGenerator::global()->bounded(m_pool.size());
        m_offers.append(m_pool.at(index));
    }
}

Unit* Shop::createUnitFromOffer(int index, Owner owner) const
{
    if (index < 0 || index >= m_offers.size()) {
        return nullptr;
    }

    const UnitTemplate& t = m_offers.at(index);
    return new Unit(t.name, owner, t.stats, t.traits);
}
