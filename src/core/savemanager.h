#ifndef CORE_SAVEMANAGER_H
#define CORE_SAVEMANAGER_H

#include "entity/unit.h"
#include <QJsonObject>
#include <QString>
#include <QVector>

class Game;
class Player;
class Board;

class SaveManager
{
public:
    static bool saveToFile(const QString& path, const Game& game);
    static bool loadFromFile(const QString& path, Game& game);

    static QJsonObject serializeGame(const Game& game);
    static bool applyToGame(const QJsonObject& root, Game& game);

private:
    static QJsonObject serializeUnit(const Unit* unit);
    static Unit* deserializeUnit(const QJsonObject& obj);
};

#endif
