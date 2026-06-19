#include "board.h"
#include <algorithm>

Board::Board()
    : m_cells(ROWS * COLS, nullptr)
{}

void Board::addUnit(Unit* unit, const QPoint& pos)
{
    const int idx = indexOf(pos);
    if (!unit || idx < 0 || m_cells[idx]) {
        return;
    }

    m_cells[idx] = unit;
    m_unitToPosition[unit] = pos;
    unit->setPosition(pos);
}

void Board::removeUnit(Unit* unit)
{
    if (!unit || !m_unitToPosition.contains(unit)) {
        return;
    }

    const int idx = indexOf(m_unitToPosition.value(unit));
    if (idx >= 0) {
        m_cells[idx] = nullptr;
    }
    m_unitToPosition.remove(unit);
}

Unit* Board::getUnitAt(const QPoint& pos) const
{
    const int idx = indexOf(pos);
    return idx < 0 ? nullptr : m_cells[idx];
}

bool Board::hasUnitAt(const QPoint& pos) const
{
    return getUnitAt(pos) != nullptr;
}

bool Board::isValidPosition(const QPoint& pos) const
{
    return pos.x() >= 0 && pos.x() < COLS && pos.y() >= 0 && pos.y() < ROWS;
}

bool Board::isPlayerHalf(const QPoint& pos) const
{
    return pos.y() >= ROWS / 2;
}
bool Board::isEnemyHalf(const QPoint& pos) const
{
    return pos.y() < ROWS / 2;
}

bool Board::moveUnit(Unit* unit, const QPoint& target)
{
    if (!unit || !isValidPosition(target) || hasUnitAt(target)) {
        return false;
    }
    if (!m_unitToPosition.contains(unit)) {
        return false;
    }

    const QPoint oldPos = m_unitToPosition.value(unit);
    const int oldIdx = indexOf(oldPos);
    const int newIdx = indexOf(target);
    if (oldIdx < 0 || newIdx < 0) {
        return false;
    }

    m_cells[oldIdx] = nullptr;
    m_cells[newIdx] = unit;
    m_unitToPosition[unit] = target;
    unit->setPosition(target);
    return true;
}

void Board::clear()
{
    std::fill(m_cells.begin(), m_cells.end(), nullptr);
    m_unitToPosition.clear();
}

int Board::indexOf(const QPoint& pos) const
{
    if (!isValidPosition(pos)) {
        return -1;
    }
    return pos.y() * COLS + pos.x();
}

