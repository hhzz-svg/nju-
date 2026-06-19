#ifndef ENTITY_EQUIPMENT_H
#define ENTITY_EQUIPMENT_H

#include <QString>

enum class EquipmentType {
    Sword,
    Armor,
    Staff,
    Tear
};

struct Equipment {
    EquipmentType type;
    QString name;
    int bonusHp = 0;
    int bonusAtk = 0;
    int bonusMana = 0;
};

inline Equipment makeEquipment(EquipmentType type)
{
    switch (type) {
    case EquipmentType::Sword:
        return {type, "Sword", 0, 6, 0};
    case EquipmentType::Armor:
        return {type, "Armor", 80, 0, 0};
    case EquipmentType::Staff:
        return {type, "Staff", 0, 8, 0};
    case EquipmentType::Tear:
        return {type, "Tear", 0, 0, 20};
    }
    return {type, "Unknown", 0, 0, 0};
}

#endif
