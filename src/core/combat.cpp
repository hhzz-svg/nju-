#include "combat.h"
#include <cmath>

void CombatSystem::update(double, Board& board, QVector<Unit*>& units)
{
    removeDeadUnits(board, units);

    for (Unit* unit : units) {
        if (!unit || !unit->isAlive() || !isOnBoard(unit)) {
            continue;
        }

        Unit* target = findNearestEnemy(unit, units);
        if (!target) {
            unit->setState(UnitState::Idle);
            continue;
        }

        if (unit->mana() >= unit->maxMana()) {
            unit->setState(UnitState::Casting);

            QVector<Unit*> allies;
            QVector<Unit*> enemies;
            for (Unit* other : units) {
                if (!other || !other->isAlive() || !isOnBoard(other)) {
                    continue;
                }

                if (other->owner() == unit->owner()) {
                    allies.append(other);
                } else {
                    enemies.append(other);
                }
            }

            unit->castSkill(allies, enemies);
            removeDeadUnits(board, units);
            continue;
        }

        if (inRange(unit, target)) {
            unit->setState(UnitState::Attacking);
            basicAttack(unit, target);
            removeDeadUnits(board, units);
            continue;
        }

        unit->setState(UnitState::Moving);
        const QPoint next = nextStepToward(unit->position(), target->position(),
                                           unit->range(), board);
        if (next != unit->position()) {
            board.moveUnit(unit, next);
        }
    }
}

bool CombatSystem::isCombatFinished(const QVector<Unit*>& units) const
{
    bool playerAlive = false;
    bool enemyAlive = false;

    for (Unit* unit : units) {
        if (!unit || !unit->isAlive() || !isOnBoard(unit)) {
            continue;
        }

        if (unit->owner() == Owner::PlayerCtrl) {
            playerAlive = true;
        } else {
            enemyAlive = true;
        }
    }

    return !playerAlive || !enemyAlive;
}

Owner CombatSystem::winner(const QVector<Unit*>& units) const
{
    for (Unit* unit : units) {
        if (unit && unit->isAlive() && isOnBoard(unit)) {
            return unit->owner();
        }
    }
    return Owner::EnemyCtrl;
}

Unit* CombatSystem::findNearestEnemy(Unit* unit, const QVector<Unit*>& units) const
{
    Unit* best = nullptr;
    int bestDist = 999999;

    for (Unit* other : units) {
        if (!other || !other->isAlive() || !isOnBoard(other) || !unit->isEnemyOf(other)) {
            continue;
        }

        const int d = distance(unit->position(), other->position());
        if (d < bestDist) {
            bestDist = d;
            best = other;
        }
    }

    return best;
}

bool CombatSystem::isOnBoard(Unit* unit) const
{
    return unit && unit->position().x() >= 0 && unit->position().x() < Board::COLS
        && unit->position().y() >= 0 && unit->position().y() < Board::ROWS;
}

int CombatSystem::distance(const QPoint& a, const QPoint& b) const
{
    return std::abs(a.x() - b.x()) + std::abs(a.y() - b.y());
}

bool CombatSystem::inRange(Unit* unit, Unit* target) const
{
    return unit && target && distance(unit->position(), target->position()) <= unit->range();
}

QPoint CombatSystem::nextStepToward(const QPoint& from, const QPoint& to, int attackRange,
                                    const Board& board) const
{
    const QPoint directions[] = {
        QPoint(1, 0),
        QPoint(-1, 0),
        QPoint(0, 1),
        QPoint(0, -1)
    };

    const auto indexOf = [](const QPoint& p) {
        return p.y() * Board::COLS + p.x();
    };

    QVector<QPoint> queue;
    QVector<QPoint> previous(Board::ROWS * Board::COLS, QPoint(-1, -1));
    QVector<bool> visited(Board::ROWS * Board::COLS, false);
    queue.append(from);
    visited[indexOf(from)] = true;

    for (int i = 0; i < queue.size(); ++i) {
        const QPoint current = queue.at(i);
        if (current != from && distance(current, to) <= attackRange) {
            QPoint step = current;
            while (previous[indexOf(step)] != from) {
                step = previous[indexOf(step)];
            }
            return step;
        }

        for (const QPoint& direction : directions) {
            const QPoint next = current + direction;
            if (!board.isValidPosition(next) || board.hasUnitAt(next)) {
                continue;
            }

            const int nextIndex = indexOf(next);
            if (visited[nextIndex]) {
                continue;
            }

            visited[nextIndex] = true;
            previous[nextIndex] = current;
            queue.append(next);
        }
    }

    return from;
}

void CombatSystem::basicAttack(Unit* attacker, Unit* target)
{
    if (!attacker || !target || !attacker->isAlive() || !target->isAlive()) {
        return;
    }

    const int damage = attacker->atk();
    target->takeDamage(damage);
    attacker->gainMana(10);
    target->gainMana(5);

    CombatEvent ev;
    ev.attackerPos = attacker->position();
    ev.targetPos = target->position();
    ev.damage = damage;
    m_events.push_back(ev);
}

void CombatSystem::removeDeadUnits(Board& board, const QVector<Unit*>& units) const
{
    for (Unit* unit : units) {
        if (unit && !unit->isAlive()) {
            board.removeUnit(unit);
        }
    }
}
