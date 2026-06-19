#ifndef UNIT_H
#define UNIT_H

#include "trait.h"
#include "equipment.h"
#include <QPoint>
#include <QString>
#include <QVector>

enum class Owner {
    PlayerCtrl,
    EnemyCtrl
};

enum class UnitState {
    Idle,
    Moving,
    Attacking,
    Casting,
    Dead
};

struct UnitStats {
    int maxHp = 100;
    int hp = 100;
    int atk = 20;
    int range = 1;
    int maxMana = 60;
    int mana = 0;
    double attackCooldown = 1.0;
    double attackTimer = 0.0;
};

class Unit
{
public:
    Unit(const QString& name,
         Owner owner,
         const UnitStats& stats,
         const QVector<Trait>& traits);
    virtual ~Unit() = default;

    int id() const { return m_id; }
    QString name() const { return m_name; }
    Owner owner() const { return m_owner; }
    UnitState state() const { return m_state; }
    QPoint position() const { return m_position; }
    const QVector<Trait>& traits() const { return m_traits; }

    int hp() const { return m_stats.hp; }
    int maxHp() const { return m_stats.maxHp; }
    int atk() const { return m_stats.atk; }
    int range() const { return m_stats.range; }
    int mana() const { return m_stats.mana; }
    int maxMana() const { return m_stats.maxMana; }
    int star() const { return m_star; }
    const UnitStats& stats() const { return m_stats; }

    void setPosition(const QPoint& pos) { m_position = pos; }
    void setState(UnitState state) { m_state = state; }
    void setStar(int star) { m_star = star; }

    bool isAlive() const { return m_stats.hp > 0 && m_state != UnitState::Dead; }
    bool isEnemyOf(const Unit* other) const;

    bool addEquipment(const Equipment& equipment);
    
    void takeDamage(int amount);
    void heal(int amount);
    void gainMana(int amount);
    void spendAllMana();
    void resetForCombat();

    virtual QString className() const { return "Unit"; }
    virtual void castSkill(QVector<Unit*>& allies, QVector<Unit*>& enemies);

protected:
    UnitStats& mutableStats() { return m_stats; }

private:
    static int s_nextId;

    int m_id;
    QString m_name;
    Owner m_owner;
    UnitState m_state;
    QPoint m_position;
    QVector<Trait> m_traits;
    UnitStats m_stats;
    QVector<Equipment> m_equipments;
    int m_star;
};

#endif
