#ifndef CORE_SYNERGY_H
#define CORE_SYNERGY_H

#include "entity/unit.h"
#include <QMap>

class SynergySystem
{
public:
    QMap<Trait, int> countTraits(const QVector<Unit*>& units) const;
    void applyBuffs(QVector<Unit*>& units) const;
    QString summary(const QVector<Unit*>& units) const;

private:
    bool isActiveOnBoard(Unit* unit) const;
};

#endif