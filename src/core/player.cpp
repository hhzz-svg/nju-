#include "player.h"
#include <algorithm>

bool Player::spendGold(int amount)
{
    if (amount < 0 || m_gold < amount) {
        return false;
    }
    m_gold -= amount;
    return true;
}

void Player::addGold(int amount)
{
    if (amount > 0) {
        m_gold += amount;
    }
}

void Player::addExp(int amount)
{
    if (amount <= 0) {
        return;
    }

    m_exp += amount;
    while (m_exp >= m_expToNext && m_level < 8) {
        m_exp -= m_expToNext;
        ++m_level;
        m_expToNext = std::min(20, m_expToNext + 2);
    }
    if (m_level >= 8) {
        m_exp = 0;
    }
}

bool Player::buyExp(int goldCost, int expGain)
{
    if (m_level >= 8) {
        return false;
    }
    if (!spendGold(goldCost)) {
        return false;
    }
    addExp(expGain);
    return true;
}

void Player::loseHp(int amount)
{
    if (amount <= 0) {
        return;
    }
    m_hp = std::max(0, m_hp - amount);
}

bool Player::addToBench(Unit* unit)
{
    if (!unit || benchFull()) {
        return false;
    }
    m_bench.append(unit);
    return true;
}

void Player::removeFromBench(Unit* unit)
{
    m_bench.removeAll(unit);
}

bool Player::benchFull() const
{
    return m_bench.size() >= 8;
}
