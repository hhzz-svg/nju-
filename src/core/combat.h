#ifndef CORE_COMBAT_H
#define CORE_COMBAT_H

#include "board.h"
#include "entity/unit.h"
#include <QPoint>
#include <QVector>
#include <vector>

enum class GamePhase {
    Prep,
    Combat,
    Resolve
};

// Positions for drawing one simple attack line in the GUI.
struct CombatEvent {
    QPoint attackerPos;
    QPoint targetPos;
    int damage = 0;        // 本次攻击造成的伤害,用于在场景里画飘字
};

class CombatSystem
{
public:
    void update(double dt, Board& board, QVector<Unit*>& units);
    bool isCombatFinished(const QVector<Unit*>& units) const;
    Owner winner(const QVector<Unit*>& units) const;

    const std::vector<CombatEvent>& recentEvents() const { return m_events; }
    void clearRecentEvents() { m_events.clear(); }

private:
    Unit* findNearestEnemy(Unit* unit, const QVector<Unit*>& units) const;
    bool isOnBoard(Unit* unit) const;
    int distance(const QPoint& a, const QPoint& b) const;
    bool inRange(Unit* unit, Unit* target) const;
    QPoint nextStepToward(const QPoint& from, const QPoint& to, int attackRange,
                          const Board& board) const;
    void basicAttack(Unit* attacker, Unit* target);
    void removeDeadUnits(Board& board, const QVector<Unit*>& units) const;

    std::vector<CombatEvent> m_events;
};

#endif // CORE_COMBAT_H
