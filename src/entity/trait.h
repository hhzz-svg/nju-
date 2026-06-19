#ifndef ENTITY_TRAIT_H
#define ENTITY_TRAIT_H

#include <QString>

enum class Trait {
    Warrior,
    Ranger,
    Mage,
    Guardian,
    Assassin,
    Healer
};

inline QString traitName(Trait trait)
{
    switch (trait) {
    case Trait::Warrior: return "Warrior";
    case Trait::Ranger: return "Ranger";
    case Trait::Mage: return "Mage";
    case Trait::Guardian: return "Guardian";
    case Trait::Assassin: return "Assassin";
    case Trait::Healer: return "Healer";
    }
    return "Unknown";
}

#endif