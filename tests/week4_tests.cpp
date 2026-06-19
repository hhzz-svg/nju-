#include "core/game.h"
#include "core/player.h"
#include "core/shop.h"
#include "entity/equipment.h"
#include "entity/unit.h"
#include "gui/unitvisuals.h"

#include <QApplication>
#include <QSet>
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace {

UnitStats stats(int hp = 100, int atk = 10, int range = 1, int mana = 60)
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

void shop_always_has_five_offers()
{
    Shop shop;
    require(shop.offers().size() == 5, "shop should start with five offers");
    shop.refresh();
    require(shop.offers().size() == 5, "shop refresh should keep five offers");
}

void stronger_shop_units_cost_more()
{
    Shop shop;
    int cheapest = 999;
    int strongest = 0;

    for (const UnitTemplate& unit : shop.pool()) {
        require(unit.strength >= 1 && unit.strength <= 3,
                "shop unit strength should use the visible 1-3 scale");
        require(unit.cost >= unit.strength,
                "stronger shop units should not be cheaper than their strength tier");
        cheapest = std::min(cheapest, unit.cost);
        strongest = std::max(strongest, unit.cost);
    }

    require(cheapest == 1 && strongest == 4,
            "shop should contain both affordable and premium units");
}

void shop_units_have_distinct_sprite_paths()
{
    Shop shop;
    QSet<QString> paths;

    for (const UnitTemplate& unit : shop.pool()) {
        const QString path = unitSpriteRelativePath(unit.name);
        require(!path.isEmpty(), "every shop unit should have a sprite path");
        require(!paths.contains(path), "shop units should not reuse the same sprite image");
        paths.insert(path);
    }
}

int enemyCount(const Game& game)
{
    int count = 0;
    for (Unit* unit : game.allUnits()) {
        if (unit && unit->owner() == Owner::EnemyCtrl) {
            ++count;
        }
    }
    return count;
}

void enemy_count_starts_balanced_and_grows()
{
    Game game;
    game.initialize();
    require(enemyCount(game) == 2, "round one should start with two enemies");

    game.clearAllUnits();
    game.setRound(5);
    game.initialize();
    require(enemyCount(game) == 4, "round five should grow to four enemies");
}

void game_refresh_and_buy_spend_gold()
{
    Game game;
    game.initialize();

    const int goldBeforeRefresh = game.player()->gold();
    game.refreshShop();
    require(game.player()->gold() == goldBeforeRefresh - 2,
            "refreshing shop should spend 2 gold");
    require(game.shop()->offers().size() == 5,
            "game shop should still show five offers after refresh");

    const int cost = game.shop()->offers().at(0).cost;
    const int goldBeforeBuy = game.player()->gold();
    const int benchBeforeBuy = game.player()->bench().size();
    require(game.buyShopUnit(0), "buying an affordable shop unit should succeed");
    require(game.player()->gold() == goldBeforeBuy - cost,
            "buying a shop unit should spend its cost");
    require(game.player()->bench().size() == benchBeforeBuy + 1,
            "buying a shop unit should place it on the bench");
}

void bench_capacity_is_eight()
{
    Player player;
    QVector<Unit*> units;
    for (int i = 0; i < 9; ++i) {
        units.append(new Unit(QString("Bench%1").arg(i),
                              Owner::PlayerCtrl,
                              stats(),
                              {Trait::Warrior}));
    }

    for (int i = 0; i < 8; ++i) {
        require(player.addToBench(units.at(i)), "first eight bench units should fit");
    }
    require(player.benchFull(), "bench should be full at eight units");
    require(!player.addToBench(units.at(8)), "ninth bench unit should be rejected");

    qDeleteAll(units);
}

void dragging_board_unit_outside_grid_sends_it_to_bench()
{
    Game game;
    game.initialize();

    Unit* unit = nullptr;
    for (Unit* candidate : game.allUnits()) {
        if (candidate && candidate->owner() == Owner::PlayerCtrl
            && candidate->position() == QPoint(2, 7)) {
            unit = candidate;
            break;
        }
    }

    require(unit, "starter player unit should be deployed on the board");
    const int benchBefore = game.player()->bench().size();
    const QPoint source = unit->position();

    game.handleDragStarted(unit->id(), source, QPointF());
    game.handleDropCommand(unit->id(), source, QPointF(10000.0, 10000.0));

    require(game.player()->bench().size() == benchBefore + 1,
            "dropping a board unit outside the grid should send it to the bench");
    require(game.player()->bench().contains(unit),
            "dropped unit should be present on the bench");
    require(unit->position() == QPoint(-1, -1),
            "bench unit should no longer have a board position");
}

void three_same_units_merge_to_higher_star()
{
    Game game;
    game.initialize();

    require(game.addUnitToBench(new Unit("Scout", Owner::PlayerCtrl, stats(), {Trait::Ranger})),
            "first duplicate unit should enter bench");
    require(game.addUnitToBench(new Unit("Scout", Owner::PlayerCtrl, stats(), {Trait::Ranger})),
            "second duplicate unit should enter bench");
    require(game.addUnitToBench(new Unit("Scout", Owner::PlayerCtrl, stats(), {Trait::Ranger})),
            "third duplicate unit should trigger merge");

    require(game.player()->bench().size() == 1,
            "three same bench units should merge into one unit");
    require(game.player()->bench().first()->star() == 2,
            "merged unit should have a higher star level");
}

void game_reports_synergy_and_equipment_information()
{
    Game game;
    game.initialize();

    const QString synergy = game.synergyText();
    require(synergy.contains(QStringLiteral("战士")) && synergy.contains(QStringLiteral("守护"))
                && synergy.contains(QStringLiteral("游侠")) && synergy.contains(QStringLiteral("法师")),
            "starter board should report at least four traits");

    const QString equipment = game.equipmentText();
    require(equipment.contains(QStringLiteral("铁剑")) && equipment.contains(QStringLiteral("锁子甲"))
                && equipment.contains(QStringLiteral("急速手套")) && equipment.contains(QStringLiteral("蓝水晶")),
            "equipment text should list the four week 4 equipment items");
}

}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    shop_always_has_five_offers();
    stronger_shop_units_cost_more();
    shop_units_have_distinct_sprite_paths();
    enemy_count_starts_balanced_and_grows();
    game_refresh_and_buy_spend_gold();
    bench_capacity_is_eight();
    dragging_board_unit_outside_grid_sends_it_to_bench();
    three_same_units_merge_to_higher_star();
    game_reports_synergy_and_equipment_information();
    return 0;
}
