#ifndef CORE_PLAYER_H
#define CORE_PLAYER_H

#include "entity/unit.h"
#include <QVector>

class Player
{
public:
    int gold() const { return m_gold; }
    int level() const { return m_level; }
    int exp() const { return m_exp; }
    int expToNext() const { return m_expToNext; }
    int boardCap() const { return m_level; }

    int hp() const { return m_hp; }
    int maxHp() const { return m_maxHp; }
    bool isAlive() const { return m_hp > 0; }
    void loseHp(int amount);

    const QVector<Unit*>& bench() const { return m_bench; }

    bool spendGold(int amount);
    void addGold(int amount);
    void addExp(int amount);
    bool buyExp(int goldCost = 4, int expGain = 4);

    bool addToBench(Unit* unit);
    void removeFromBench(Unit* unit);
    bool benchFull() const;

    void setGold(int v) { m_gold = v; }
    void setLevel(int v) { m_level = v; }
    void setExp(int v) { m_exp = v; }
    void setHp(int v) { m_hp = v; }
    void setMaxHp(int v) { m_maxHp = v; }
    void clearBench() { m_bench.clear(); }

private:
    int m_gold = 10;
    int m_level = 3;
    int m_exp = 0;
    int m_expToNext = 4;
    int m_hp = 30;
    int m_maxHp = 30;
    QVector<Unit*> m_bench;
};

#endif
