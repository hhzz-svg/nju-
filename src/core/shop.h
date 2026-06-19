#ifndef CORE_SHOP_H
#define CORE_SHOP_H

#include "entity/unit.h"
#include <QString>
#include <QVector>

struct UnitTemplate {
    QString name;
    int cost = 1;
    int strength = 1;
    UnitStats stats;
    QVector<Trait> traits;
};

class Shop
{
public:
    Shop();

    const QVector<UnitTemplate>& offers() const { return m_offers; }
    const QVector<UnitTemplate>& pool() const { return m_pool; }
    void refresh();
    Unit* createUnitFromOffer(int index, Owner owner) const;

private:
    QVector<UnitTemplate> m_pool;
    QVector<UnitTemplate> m_offers;
};

#endif
