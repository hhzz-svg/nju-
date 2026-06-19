#include "unit.h"
#include <algorithm>

int Unit::s_nextId = 0;

Unit::Unit(const QString& name,
           Owner owner,
           const UnitStats& stats,
           const QVector<Trait>& traits)
    : m_id(s_nextId++)
    , m_name(name)
    , m_owner(owner)
    , m_state(UnitState::Idle)
    , m_position(-1, -1)
    , m_traits(traits)
    , m_stats(stats)
    , m_star(1)
{}

bool Unit::isEnemyOf(const Unit* other) const
{
    return other && m_owner != other->owner();
}

void Unit::takeDamage(int amount)
{
    if (amount <= 0 || !isAlive()) {
        return;
    }

    m_stats.hp = std::max(0, m_stats.hp - amount);
    if (m_stats.hp == 0) {
        m_state = UnitState::Dead;
    }
}

void Unit::heal(int amount)
{
    if (amount <= 0 || !isAlive()) {
        return;
    }

    m_stats.hp = std::min(m_stats.maxHp, m_stats.hp + amount);
}

void Unit::gainMana(int amount)
{
    if (amount <= 0 || !isAlive()) {
        return;
    }

    m_stats.mana = std::min(m_stats.maxMana, m_stats.mana + amount);
}

void Unit::spendAllMana()
{
    m_stats.mana = 0;
}

void Unit::resetForCombat()
{
    m_stats.hp = m_stats.maxHp;
    m_stats.mana = 0;
    m_stats.attackTimer = 0.0;
    m_state = UnitState::Idle;
}

void Unit::castSkill(QVector<Unit*>& allies, QVector<Unit*>& enemies)
{
    if (m_name == QStringLiteral("Mage")) {
        for (Unit* enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                enemy->takeDamage(22 + 8 * m_star);
            }
        }
    } else if (m_name == QStringLiteral("Priest") || m_name == QStringLiteral("Paladin")) {
        Unit* bestTarget = nullptr;
        int missingHp = -1;

        for (Unit* ally : allies) {
            if (!ally || !ally->isAlive()) {
                continue;
            }

            const int missing = ally->maxHp() - ally->hp();
            if (missing > missingHp) {
                missingHp = missing;
                bestTarget = ally;
            }
        }

        if (bestTarget) {
            bestTarget->heal(30 + 15 * m_star);
        }
    } else if (m_name == QStringLiteral("Knight")) {
        heal(15 + 5 * m_star);
        for (Unit* enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                enemy->takeDamage(22 + 8 * m_star);
                break;
            }
        }
    } else if (m_name == QStringLiteral("Archer") || m_name == QStringLiteral("Stalker")) {
        for (Unit* enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                enemy->takeDamage(28 + 8 * m_star);
                break;
            }
        }
    } else {
        for (Unit* enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                enemy->takeDamage(20 + 6 * m_star);
                break;
            }
        }
    }

    spendAllMana();
}

bool Unit::addEquipment(const Equipment& equipment)
{
    if (m_equipments.size() >= 3) {
        return false;
    }

    m_equipments.append(equipment);
    m_stats.maxHp += equipment.bonusHp;
    m_stats.hp += equipment.bonusHp;
    m_stats.atk += equipment.bonusAtk;
    m_stats.maxMana += equipment.bonusMana;
    return true;
}
