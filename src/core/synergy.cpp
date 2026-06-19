#include "synergy.h"

bool SynergySystem::isActiveOnBoard(Unit* unit) const
{
    return unit
        && unit->owner() == Owner::PlayerCtrl
        && unit->isAlive()
        && unit->position().x() >= 0
        && unit->position().y() >= 0;
}

QMap<Trait, int> SynergySystem::countTraits(const QVector<Unit*>& units) const
{
    QMap<Trait, int> result;

    for (Unit* unit : units) {
        if (!isActiveOnBoard(unit)) {
            continue;
        }

        for (Trait trait : unit->traits()) {
            result[trait] += 1;
        }
    }

    return result;
}

void SynergySystem::applyBuffs(QVector<Unit*>& units) const
{
    const QMap<Trait, int> counts = countTraits(units);

    const bool warrior2 = counts.value(Trait::Warrior) >= 2;
    const bool ranger2 = counts.value(Trait::Ranger) >= 2;
    const bool mage2 = counts.value(Trait::Mage) >= 2;
    const bool guardian2 = counts.value(Trait::Guardian) >= 2;

    for (Unit* unit : units) {
        if (!isActiveOnBoard(unit)) {
            continue;
        }


        if (warrior2 && unit->traits().contains(Trait::Warrior)) {
            unit->heal(30);
        }
        if (mage2 && unit->traits().contains(Trait::Mage)) {
            unit->gainMana(20);
        }
        if (guardian2 && unit->traits().contains(Trait::Guardian)) {
            unit->heal(40);
        }
        if (ranger2 && unit->traits().contains(Trait::Ranger)) {
            unit->gainMana(10);
        }
    }
}

QString SynergySystem::summary(const QVector<Unit*>& units) const
{
    const QMap<Trait, int> counts = countTraits(units);
    QString text;

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        if (!text.isEmpty()) {
            text += " | ";
        }
        text += QString("%1:%2").arg(traitName(it.key())).arg(it.value());
    }

    return text.isEmpty() ? "No synergy" : text;
}