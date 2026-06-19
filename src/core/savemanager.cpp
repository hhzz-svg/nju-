#include "savemanager.h"
#include "core/game.h"
#include "core/board.h"
#include "core/player.h"
#include "entity/unit.h"
#include "entity/trait.h"
#include "entity/equipment.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

namespace {

QString ownerToString(Owner o) {
    return o == Owner::PlayerCtrl ? "player" : "enemy";
}

Owner ownerFromString(const QString& s) {
    return s == "player" ? Owner::PlayerCtrl : Owner::EnemyCtrl;
}

QString traitToString(Trait t) {
    switch (t) {
    case Trait::Warrior:  return "Warrior";
    case Trait::Ranger:   return "Ranger";
    case Trait::Mage:     return "Mage";
    case Trait::Guardian: return "Guardian";
    case Trait::Assassin: return "Assassin";
    case Trait::Healer:   return "Healer";
    }
    return "Warrior";
}

Trait traitFromString(const QString& s) {
    if (s == "Warrior")  return Trait::Warrior;
    if (s == "Ranger")   return Trait::Ranger;
    if (s == "Mage")     return Trait::Mage;
    if (s == "Guardian") return Trait::Guardian;
    if (s == "Assassin") return Trait::Assassin;
    if (s == "Healer")   return Trait::Healer;
    return Trait::Warrior;
}

} // namespace

QJsonObject SaveManager::serializeUnit(const Unit* unit)
{
    QJsonObject obj;
    obj["name"]      = unit->name();
    obj["owner"]     = ownerToString(unit->owner());
    obj["star"]      = unit->star();
    obj["posX"]      = unit->position().x();
    obj["posY"]      = unit->position().y();

    const UnitStats& s = unit->stats();
    QJsonObject stats;
    stats["maxHp"]   = s.maxHp;
    stats["hp"]      = s.hp;
    stats["atk"]     = s.atk;
    stats["range"]   = s.range;














    stats["maxMana"] = s.maxMana;
    stats["mana"]    = s.mana;
    obj["stats"]     = stats;

    QJsonArray traits;
    for (Trait t : unit->traits()) {
        traits.append(traitToString(t));
    }
    obj["traits"] = traits;

    return obj;
}

Unit* SaveManager::deserializeUnit(const QJsonObject& obj)
{
    UnitStats s;
    const QJsonObject stats = obj.value("stats").toObject();
    s.maxHp   = stats.value("maxHp").toInt(100);
    s.hp      = stats.value("hp").toInt(s.maxHp);
    s.atk     = stats.value("atk").toInt(20);
    s.range   = stats.value("range").toInt(1);
    s.maxMana = stats.value("maxMana").toInt(60);
    s.mana    = stats.value("mana").toInt(0);

    QVector<Trait> traits;
    for (const QJsonValue& v : obj.value("traits").toArray()) {
        traits.append(traitFromString(v.toString()));
    }

    Unit* unit = new Unit(obj.value("name").toString("Unit"),
                          ownerFromString(obj.value("owner").toString("player")),
                          s,
                          traits);
    unit->setStar(obj.value("star").toInt(1));

    const int x = obj.value("posX").toInt(-1);
    const int y = obj.value("posY").toInt(-1);
    unit->setPosition(QPoint(x, y));
    return unit;
}

QJsonObject SaveManager::serializeGame(const Game& game)
{
    QJsonObject root;
    root["version"] = 1;
    root["round"]   = game.round();

    const Player& p = const_cast<Game&>(game).playerRef();
    QJsonObject playerObj;
    playerObj["gold"]  = p.gold();
    playerObj["level"] = p.level();
    playerObj["exp"]   = p.exp();
    playerObj["hp"]    = p.hp();
    playerObj["maxHp"] = p.maxHp();
    root["player"] = playerObj;

    QJsonArray boardUnits;
    QJsonArray benchUnits;
    QJsonArray enemyUnits;

    for (Unit* u : game.allUnits()) {
        if (!u) continue;
        QJsonObject obj = serializeUnit(u);

        if (u->owner() == Owner::EnemyCtrl) {
            enemyUnits.append(obj);
            continue;
        }

        const bool onBench = p.bench().contains(u);
        if (onBench) {
            benchUnits.append(obj);
        } else {
            boardUnits.append(obj);
        }
    }

    root["boardUnits"] = boardUnits;
    root["benchUnits"] = benchUnits;
    root["enemyUnits"] = enemyUnits;
    return root;
}

bool SaveManager::saveToFile(const QString& path, const Game& game)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    const QJsonDocument doc(serializeGame(game));
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

bool SaveManager::applyToGame(const QJsonObject& root, Game& game)
{
    if (root.isEmpty()) {
        return false;
    }

    game.clearAllUnits();

    const QJsonObject playerObj = root.value("player").toObject();
    Player& p = game.playerRef();
    p.setGold(playerObj.value("gold").toInt(10));
    p.setLevel(playerObj.value("level").toInt(3));
    p.setExp(playerObj.value("exp").toInt(0));
    p.setHp(playerObj.value("hp").toInt(30));
    p.setMaxHp(playerObj.value("maxHp").toInt(30));
    p.clearBench();

    game.setRound(root.value("round").toInt(1));

    QVector<Unit*> restored;
    QVector<Unit*> benchUnits;
    QVector<Unit*> boardUnits;
    QVector<Unit*> enemyUnits;

    for (const QJsonValue& v : root.value("boardUnits").toArray()) {
        Unit* u = deserializeUnit(v.toObject());
        boardUnits.append(u);
        restored.append(u);
    }
    for (const QJsonValue& v : root.value("benchUnits").toArray()) {
        Unit* u = deserializeUnit(v.toObject());
        u->setPosition(QPoint(-1, -1));
        benchUnits.append(u);
        restored.append(u);
    }
    for (const QJsonValue& v : root.value("enemyUnits").toArray()) {
        Unit* u = deserializeUnit(v.toObject());
        enemyUnits.append(u);
        restored.append(u);
    }

    game.installLoadedUnits(restored, boardUnits, benchUnits, enemyUnits);
    return true;
}

bool SaveManager::loadFromFile(const QString& path, Game& game)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    try {
        return applyToGame(doc.object(), game);
    } catch (...) {
        return false;
    }
}
