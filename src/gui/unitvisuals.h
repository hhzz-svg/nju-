#ifndef GUI_UNITVISUALS_H
#define GUI_UNITVISUALS_H

#include <QString>

inline QString displayNameForUnit(const QString& name)
{
    if (name == QStringLiteral("Knight"))  return QStringLiteral("骑士");
    if (name == QStringLiteral("Archer"))  return QStringLiteral("弓手");
    if (name == QStringLiteral("Mage"))    return QStringLiteral("法师");
    if (name == QStringLiteral("Priest"))  return QStringLiteral("牧师");
    if (name == QStringLiteral("Stalker")) return QStringLiteral("潜行者");
    if (name == QStringLiteral("Paladin")) return QStringLiteral("圣骑士");
    if (name == QStringLiteral("Goblin"))  return QStringLiteral("哥布林");
    if (name == QStringLiteral("Brute"))   return QStringLiteral("蛮兵");
    if (name == QStringLiteral("Raider"))  return QStringLiteral("突袭兵");
    return name;
}

inline QString unitSpriteRelativePath(const QString& name)
{
    if (name == QStringLiteral("Knight"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_1/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    if (name == QStringLiteral("Archer"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_01/PNG Sequences/Idle/Satyr_01_Idle_000.png");
    if (name == QStringLiteral("Mage"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_02/PNG Sequences/Idle/Satyr_02_Idle_000.png");
    if (name == QStringLiteral("Priest"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_03/PNG Sequences/Idle/Satyr_03_Idle_000.png");
    if (name == QStringLiteral("Stalker"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_2/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    if (name == QStringLiteral("Paladin"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_3/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    if (name == QStringLiteral("Goblin"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_03/PNG Sequences/Attacking/Satyr_03_Attacking_000.png");
    if (name == QStringLiteral("Brute"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_3/PNG/PNG Sequences/Running/0_Reaper_Man_Running_000.png");
    if (name == QStringLiteral("Raider"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_2/PNG/PNG Sequences/Running/0_Reaper_Man_Running_000.png");
    return QString();
}

#endif // GUI_UNITVISUALS_H
