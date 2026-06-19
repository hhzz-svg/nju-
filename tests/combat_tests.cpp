#include "core/board.h"
#include "core/combat.h"
#include "entity/unit.h"

#include <QVector>
#include <cstdlib>
#include <iostream>

namespace {

UnitStats stats(int hp, int atk, int range, int mana)
{
    UnitStats s;
    s.maxHp = hp;
    s.hp = hp;
    s.atk = atk;
    s.range = range;
    s.maxMana = mana;
    return s;
}

void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

void moves_toward_nearest_enemy()
{
    Board board;
    CombatSystem combat;
    Unit player("Knight", Owner::PlayerCtrl, stats(100, 10, 1, 60), {Trait::Warrior});
    Unit enemy("Brute", Owner::EnemyCtrl, stats(100, 10, 1, 60), {Trait::Warrior});

    board.addUnit(&player, QPoint(0, 7));
    board.addUnit(&enemy, QPoint(0, 5));
    QVector<Unit*> units{&player, &enemy};

    combat.update(0.6, board, units);

    require(player.position() == QPoint(0, 6), "player should step toward the nearest enemy");
    require(player.state() == UnitState::Moving, "moving unit should enter Moving state");
}

void attacks_and_removes_dead_units()
{
    Board board;
    CombatSystem combat;
    Unit player("Knight", Owner::PlayerCtrl, stats(100, 50, 1, 60), {Trait::Warrior});
    Unit enemy("Brute", Owner::EnemyCtrl, stats(40, 10, 1, 60), {Trait::Warrior});

    board.addUnit(&player, QPoint(0, 6));
    board.addUnit(&enemy, QPoint(0, 5));
    QVector<Unit*> units{&player, &enemy};

    combat.update(0.6, board, units);

    require(enemy.state() == UnitState::Dead, "lethal attack should mark target dead");
    require(!board.hasUnitAt(QPoint(0, 5)), "dead unit should be removed from the board");
    require(combat.isCombatFinished(units), "combat should finish when one side has no living units");
    require(combat.winner(units) == Owner::PlayerCtrl, "player should win when enemies are dead");
}

void ranged_unit_attacks_adjacent_one_hp_enemy()
{
    Board board;
    CombatSystem combat;
    Unit player("Archer", Owner::PlayerCtrl, stats(100, 10, 3, 60), {Trait::Ranger});
    Unit enemy("Brute", Owner::EnemyCtrl, stats(1, 10, 1, 60), {Trait::Warrior});

    board.addUnit(&player, QPoint(0, 6));
    board.addUnit(&enemy, QPoint(0, 5));
    QVector<Unit*> units{&player, &enemy};

    combat.update(0.6, board, units);

    require(enemy.state() == UnitState::Dead,
            "ranged unit should attack an adjacent one-HP enemy instead of retreating");
    require(player.position() == QPoint(0, 6),
            "ranged unit should not move away from an enemy already in range");
    require(combat.isCombatFinished(units),
            "combat should finish after the last adjacent enemy dies");
}

void enemy_units_route_around_allies()
{
    Board board;
    CombatSystem combat;
    Unit enemyBehind("Brute", Owner::EnemyCtrl, stats(100, 10, 1, 60), {Trait::Warrior});
    Unit enemyFront("Brute", Owner::EnemyCtrl, stats(100, 10, 1, 60), {Trait::Warrior});
    Unit player("Knight", Owner::PlayerCtrl, stats(100, 10, 1, 60), {Trait::Warrior});

    board.addUnit(&enemyBehind, QPoint(0, 0));
    board.addUnit(&enemyFront, QPoint(0, 1));
    board.addUnit(&player, QPoint(0, 3));
    QVector<Unit*> units{&enemyBehind, &enemyFront, &player};

    combat.update(0.6, board, units);

    require(enemyBehind.position() == QPoint(1, 0),
            "blocked enemy should take a side step and route around an ally");
}

void bench_units_do_not_affect_combat()
{
    Board board;
    CombatSystem combat;
    Unit benchPlayer("Knight", Owner::PlayerCtrl, stats(100, 10, 1, 60), {Trait::Warrior});
    Unit enemy("Brute", Owner::EnemyCtrl, stats(100, 10, 1, 60), {Trait::Warrior});

    board.addUnit(&enemy, QPoint(0, 0));
    QVector<Unit*> units{&benchPlayer, &enemy};

    combat.update(0.6, board, units);

    require(benchPlayer.position() == QPoint(-1, -1),
            "bench unit should stay out of the combat loop");
    require(combat.isCombatFinished(units),
            "bench unit should not keep combat running after the board side is empty");
    require(combat.winner(units) == Owner::EnemyCtrl,
            "enemy should win when only a player bench unit remains");
}

void crowded_battle_eventually_finishes()
{
    Board board;
    CombatSystem combat;
    QVector<Unit*> units;

    for (int i = 0; i < 6; ++i) {
        Unit* player = new Unit(QString("Player%1").arg(i), Owner::PlayerCtrl,
                                stats(80, 14, 1, 1000), {Trait::Warrior});
        Unit* enemy = new Unit(QString("Enemy%1").arg(i), Owner::EnemyCtrl,
                               stats(80, 14, 1, 1000), {Trait::Warrior});
        board.addUnit(player, QPoint(i, 7));
        board.addUnit(enemy, QPoint(i, 0));
        units.append(player);
        units.append(enemy);
    }

    for (int tick = 0; tick < 200 && !combat.isCombatFinished(units); ++tick) {
        combat.update(0.6, board, units);
    }

    require(combat.isCombatFinished(units),
            "crowded battle should finish instead of stalling behind occupied cells");
    qDeleteAll(units);
}

void casts_distinct_skill_effects()
{
    Board board;
    CombatSystem combat;
    Unit mage("Mage", Owner::PlayerCtrl, stats(100, 5, 3, 10), {Trait::Mage});
    Unit priest("Priest", Owner::PlayerCtrl, stats(100, 5, 2, 10), {Trait::Healer});
    Unit knight("Knight", Owner::PlayerCtrl, stats(100, 5, 1, 10), {Trait::Warrior});
    Unit enemyA("Brute", Owner::EnemyCtrl, stats(100, 5, 1, 10), {Trait::Warrior});
    Unit enemyB("Goblin", Owner::EnemyCtrl, stats(100, 5, 1, 10), {Trait::Assassin});

    board.addUnit(&mage, QPoint(1, 6));
    board.addUnit(&priest, QPoint(2, 6));
    board.addUnit(&knight, QPoint(3, 6));
    board.addUnit(&enemyA, QPoint(1, 5));
    board.addUnit(&enemyB, QPoint(2, 5));
    QVector<Unit*> units{&mage, &priest, &knight, &enemyA, &enemyB};

    mage.gainMana(10);
    combat.update(0.6, board, units);
    require(enemyA.hp() < 100 && enemyB.hp() < 100, "Mage skill should damage multiple enemies");

    knight.takeDamage(50);
    priest.gainMana(10);
    combat.update(0.6, board, units);
    require(knight.hp() > 50, "Priest skill should heal a damaged ally");

    const int before = enemyA.hp();
    knight.gainMana(10);
    combat.update(0.6, board, units);
    require(enemyA.hp() < before, "Knight skill should damage one enemy");
}

void equipment_bonus_updates_unit_stats()
{
    Unit knight("Knight", Owner::PlayerCtrl, stats(100, 10, 1, 60), {Trait::Warrior});

    require(knight.addEquipment(makeEquipment(EquipmentType::Sword)),
            "first equipment should be accepted");
    require(knight.addEquipment(makeEquipment(EquipmentType::Armor)),
            "second equipment should be accepted");
    require(knight.addEquipment(makeEquipment(EquipmentType::Tear)),
            "third equipment should be accepted");
    require(!knight.addEquipment(makeEquipment(EquipmentType::Staff)),
            "fourth equipment should be rejected");

    require(knight.atk() == 16, "Sword should add attack");
    require(knight.maxHp() == 180 && knight.hp() == 180,
            "Armor should add current and max HP");
    require(knight.maxMana() == 80, "Tear should add max mana");
}

}

int main()
{
    moves_toward_nearest_enemy();
    attacks_and_removes_dead_units();
    ranged_unit_attacks_adjacent_one_hp_enemy();
    enemy_units_route_around_allies();
    bench_units_do_not_affect_combat();
    crowded_battle_eventually_finishes();
    casts_distinct_skill_effects();
    equipment_bonus_updates_unit_stats();
    return 0;
}
