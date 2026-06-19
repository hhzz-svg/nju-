#include "game.h"
#include "savemanager.h"
#include "entity/equipment.h"
#include "entity/unit.h"
#include "gui/griditem.h"
#include "gui/unititem.h"
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <QRandomGenerator>
#include <QTimer>
#include <QtMath>

namespace {
constexpr qreal kZGrid = 0.0;
constexpr qreal kZUnit = 1.0;
constexpr qreal kZDraggingUnit = 2.0;

QString displayTraitName(Trait trait)
{
    switch (trait) {
    case Trait::Warrior:  return QStringLiteral("战士");
    case Trait::Ranger:   return QStringLiteral("游侠");
    case Trait::Mage:     return QStringLiteral("法师");
    case Trait::Guardian: return QStringLiteral("守护");
    case Trait::Assassin: return QStringLiteral("刺客");
    case Trait::Healer:   return QStringLiteral("治疗");
    }
    return QStringLiteral("未知");
}

QString phaseName(GamePhase p)
{
    switch (p) {
    case GamePhase::Prep:    return QStringLiteral("准备");
    case GamePhase::Combat:  return QStringLiteral("战斗");
    case GamePhase::Resolve: return QStringLiteral("结算");
    }
    return QStringLiteral("?");
}
}

Game::Game(QObject* parent)
    : QObject(parent)
    , m_scene(new QGraphicsScene(this))
    , m_dragActive(false)
    , m_activeUnitId(-1)
    , m_sourceGrid(-1, -1)
    , m_rows(Board::ROWS)
    , m_cols(Board::COLS)
    , m_radius(38.0)
    , m_rowSpacing(58.0)
    , m_phase(GamePhase::Prep)
    , m_combatTimer(new QTimer(this))
    , m_effectTimer(new QTimer(this))
    , m_round(1)
    , m_gameOver(false)
{
    connect(m_combatTimer, &QTimer::timeout,
            this, &Game::updateCombat);
    connect(m_effectTimer, &QTimer::timeout,
            this, &Game::tickEffects);
    m_effectTimer->start(60);
}

Game::~Game()
{
    qDeleteAll(m_units);
    m_units.clear();
}

void Game::initialize()
{
    createStarterUnitsIfNeeded();
    generateEnemiesForRound();
    buildScene();

    
    const QPoint playerPositions[] = {
        QPoint(2, 7), QPoint(4, 7), QPoint(6, 7)
    };
    int idx = 0;
    for (Unit* u : m_units) {
        if (u && u->owner() == Owner::PlayerCtrl && idx < 3) {
            m_board.addUnit(u, playerPositions[idx++]);
        }
    }

    syncFromBoard();
    emit stateChanged();
}

void Game::reset()
{
    m_combatTimer->stop();
    m_phase = GamePhase::Prep;
    m_board.clear();

    
    QVector<Unit*> survivors;
    for (Unit* u : m_units) {
        if (!u) continue;
        if (u->owner() == Owner::EnemyCtrl) {
            delete u;
            continue;
        }
        u->resetForCombat();
        survivors.append(u);
    }
    m_units = survivors;

    
    QVector<Unit*> deployed;
    for (Unit* u : m_units) {
        if (!u) continue;
        if (m_player.bench().contains(u)) continue;
        deployed.append(u);
    }

    int row = 7, col = 0;
    for (Unit* u : deployed) {
        while (row >= 4) {
            if (!m_board.hasUnitAt(QPoint(col, row))) {
                m_board.addUnit(u, QPoint(col, row));
                break;
            }
            ++col;
            if (col >= Board::COLS) { col = 0; --row; }
        }
    }

    generateEnemiesForRound();

    syncFromBoard();
    emit stateChanged();
}

void Game::generateEnemiesForRound()
{
    const int enemyCount = qMin(8, 1 + (m_round + 1) / 2);
    const double scale = 1.0 + 0.2 * ((m_round - 1) / 3);

    for (int i = 0; i < enemyCount; ++i) {
        UnitStats stats;
        const int variant = i % 3;
        if (variant == 0) {
            stats.maxHp = static_cast<int>(220 * scale);
            stats.atk = static_cast<int>(24 * scale);
            stats.range = 1;
            stats.maxMana = 60;
        } else if (variant == 1) {
            stats.maxHp = static_cast<int>(140 * scale);
            stats.atk = static_cast<int>(22 * scale);
            stats.range = 3;
            stats.maxMana = 50;
        } else {
            stats.maxHp = static_cast<int>(170 * scale);
            stats.atk = static_cast<int>(28 * scale);
            stats.range = 1;
            stats.maxMana = 70;
        }
        stats.hp = stats.maxHp;

        const QString name = (variant == 0) ? "Brute"
                          : (variant == 1) ? "Goblin"
                                           : "Brute";
        const Trait t = (variant == 1) ? Trait::Ranger : Trait::Warrior;
        Unit* enemy = new Unit(name, Owner::EnemyCtrl, stats, {t});
        m_units.append(enemy);

        
        const int col = i % Board::COLS;
        const int row = i / Board::COLS;
        if (!m_board.hasUnitAt(QPoint(col, row))) {
            m_board.addUnit(enemy, QPoint(col, row));
        }
    }

    buildScene();
}

void Game::startCombat()
{
    if (m_phase != GamePhase::Prep || m_gameOver) {
        return;
    }

    m_synergy.applyBuffs(m_units);
    m_phase = GamePhase::Combat;
    for (Unit* unit : m_units) {
        if (unit && unit->isAlive()) {
            unit->setState(UnitState::Idle);
        }
    }

    m_combatTimer->start(500);
    emit stateChanged();
}

void Game::updateCombat()
{
    if (m_phase != GamePhase::Combat) {
        return;
    }

    m_combat.update(0.5, m_board, m_units);
    spawnAttackEffects();        // 把这一帧打出来的攻击转成场景里的连线和飘字
    m_combat.clearRecentEvents();
    syncFromBoard();

    if (m_combat.isCombatFinished(m_units)) {
        resolveCombat();
    }

    emit stateChanged();
}

void Game::spawnAttackEffects()
{
    const std::vector<CombatEvent>& events = m_combat.recentEvents();
    if (events.empty() || !m_scene) {
        return;
    }

    for (const CombatEvent& ev : events) {
        if (!m_board.isValidPosition(ev.attackerPos) ||
            !m_board.isValidPosition(ev.targetPos)) {
            continue;
        }

        const QPointF p1 = gridToWorld(ev.attackerPos.y(), ev.attackerPos.x());
        const QPointF p2 = gridToWorld(ev.targetPos.y(),   ev.targetPos.x());

        // B2：攻击者→目标 一条琥珀色短线
        QGraphicsLineItem* line = new QGraphicsLineItem(QLineF(p1, p2));
        QPen pen(QColor(233, 180, 76, 230));
        pen.setWidth(3);
        pen.setCapStyle(Qt::RoundCap);
        line->setPen(pen);
        line->setZValue(3.0);
        m_scene->addItem(line);

        VisualEffect lineFx;
        lineFx.item = line;
        lineFx.kind = 0;
        lineFx.msLeft = 240;
        lineFx.totalMs = 240;
        m_effects.push_back(lineFx);

        
        QGraphicsSimpleTextItem* text = new QGraphicsSimpleTextItem(
            QString::number(ev.damage));
        QFont f = text->font();
        f.setBold(true);
        f.setPointSize(11);
        text->setFont(f);
        text->setBrush(QColor(255, 220, 220));
        text->setPen(QPen(QColor(60, 0, 0), 0.5));
        const QRectF tb = text->boundingRect();
        text->setPos(p2.x() - tb.width() / 2.0, p2.y() - 36.0);
        text->setZValue(5.0);
        m_scene->addItem(text);

        VisualEffect textFx;
        textFx.item = text;
        textFx.kind = 1;
        textFx.msLeft = 700;
        textFx.totalMs = 700;
        textFx.vy = -1.2;
        m_effects.push_back(textFx);
    }
}

void Game::tickEffects()
{
    if (m_effects.empty()) {
        return;
    }

    constexpr int dt = 60;
    auto it = m_effects.begin();
    while (it != m_effects.end()) {
        it->msLeft -= dt;
        if (it->msLeft <= 0) {
            if (it->item && m_scene) {
                m_scene->removeItem(it->item);
            }
            delete it->item;
            it = m_effects.erase(it);
            continue;
        }

        if (it->kind == 1 && it->item) {
            
            it->item->moveBy(0.0, it->vy);
            const double rate = static_cast<double>(it->msLeft) / it->totalMs;
            it->item->setOpacity(qMax(0.0, qMin(1.0, rate)));
        } else if (it->kind == 0 && it->item) {
        
            const double rate = static_cast<double>(it->msLeft) / it->totalMs;
            it->item->setOpacity(qMax(0.0, qMin(1.0, rate * 1.6)));
        }

        ++it;
    }
}

void Game::resolveCombat()
{
    m_combatTimer->stop();
    m_phase = GamePhase::Resolve;

    const Owner winner = m_combat.winner(m_units);
    if (winner == Owner::PlayerCtrl) {
        m_player.addGold(5 + m_round);
        m_player.addExp(2);
    } else {
        m_player.loseHp(3 + m_round);
        m_player.addGold(2);
        if (!m_player.isAlive()) {
            m_gameOver = true;
            emit gameOver(false);
            emit stateChanged();
            return;
        }
    }

    ++m_round;
    reset();
}

void Game::handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF&)
{
    if (m_phase != GamePhase::Prep || m_gameOver) {
        return;
    }

    m_dragActive = true;
    m_activeUnitId = unitId;
    m_sourceGrid = sourceGrid;

    UnitItem* item = findUnitItem(unitId);
    if (item) {
        item->setZValue(kZDraggingUnit);
    }
}

void Game::handleDragMoved(int unitId, const QPoint&, const QPointF& scenePos)
{
    if (m_phase != GamePhase::Prep || !m_dragActive) {
        return;
    }

    clearGridHighlights();

    const QPoint target = hitGridAt(scenePos);
    GridItem* targetItem = findGridItem(target);
    if (!targetItem) {
        return;
    }

    targetItem->setHoverActive(true);

    if (canApplyDrop(unitId, m_sourceGrid, target)) {
        targetItem->setDropActive(true);
    }
}

void Game::handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos)
{
    if (m_phase != GamePhase::Prep || !m_dragActive) {
        return;
    }

    const QPoint target = hitGridAt(scenePos);

    clearGridHighlights();
    if (canApplyDrop(unitId, sourceGrid, target)) {
        applyDrop(unitId, target);
    } else if (!m_board.isValidPosition(target)) {
        sendUnitToBench(unitId);
    }

    UnitItem* item = findUnitItem(m_activeUnitId);
    if (item) {
        item->setZValue(kZUnit);
    }

    m_dragActive = false;
    m_activeUnitId = -1;
    m_sourceGrid = QPoint(-1, -1);

    syncFromBoard();
}

void Game::createStarterUnitsIfNeeded()
{
    if (!m_units.isEmpty()) {
        return;
    }

    UnitStats warrior;
    warrior.maxHp = 220; warrior.hp = 220; warrior.atk = 28; warrior.range = 1; warrior.maxMana = 60;

    UnitStats ranger;
    ranger.maxHp = 140; ranger.hp = 140; ranger.atk = 22; ranger.range = 3; ranger.maxMana = 50;

    UnitStats mage;
    mage.maxHp = 120; mage.hp = 120; mage.atk = 16; mage.range = 3; mage.maxMana = 40;

    Unit* knight = new Unit("Knight", Owner::PlayerCtrl, warrior, {Trait::Warrior, Trait::Guardian});
    knight->addEquipment(makeEquipment(EquipmentType::Sword));

    Unit* archer = new Unit("Archer", Owner::PlayerCtrl, ranger, {Trait::Ranger});
    archer->addEquipment(makeEquipment(EquipmentType::Tear));

    Unit* mageUnit = new Unit("Mage", Owner::PlayerCtrl, mage, {Trait::Mage});
    mageUnit->addEquipment(makeEquipment(EquipmentType::Staff));

    m_units.append(knight);
    m_units.append(archer);
    m_units.append(mageUnit);
}

Unit* Game::findUnitById(int unitId) const
{
    for (Unit* unit : m_units) {
        if (unit && unit->id() == unitId) {
            return unit;
        }
    }
    return nullptr;
}

GridItem* Game::findGridItem(const QPoint& gridPos) const
{
    for (GridItem* item : m_gridItems) {
        if (item && item->gridPos() == gridPos) {
            return item;
        }
    }
    return nullptr;
}

UnitItem* Game::findUnitItem(int unitId) const
{
    auto it = m_unitItemById.find(unitId);
    if (it == m_unitItemById.end()) {
        return nullptr;
    }
    return it->second;
}

void Game::clearGridHighlights()
{
    for (GridItem* item : m_gridItems) {
        if (!item) continue;
        item->setHoverActive(false);
        item->setDropActive(false);
    }
}

int Game::countDeployedPlayerUnits() const
{
    int n = 0;
    for (Unit* u : m_units) {
        if (!u || u->owner() != Owner::PlayerCtrl) continue;
        if (m_player.bench().contains(u)) continue;
        if (!m_board.isValidPosition(u->position())) continue;
        ++n;
    }
    return n;
}

bool Game::canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const
{
    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl) {
        return false;
    }

    if (!m_board.isValidPosition(source) || !m_board.isValidPosition(target)) {
        return false;
    }

    if (!m_board.isPlayerHalf(source) || !m_board.isPlayerHalf(target)) {
        return false;
    }

    if (source == target || m_board.hasUnitAt(target)) {
        return false;
    }

    return m_board.getUnitAt(source) == unit;
}

void Game::applyDrop(int unitId, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    if (!unit) return;

    m_board.removeUnit(unit);
    m_board.addUnit(unit, target);
}

void Game::buildScene()
{
    
    m_effects.clear();
    m_scene->clear();
    m_gridItems.clear();
    m_unitItems.clear();
    m_unitItemById.clear();

    QRectF totalBounds;
    bool first = true;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QPolygonF poly = cellHexPolygon(row, col);
            GridItem* gridItem = new GridItem(row, col, poly);
            gridItem->setZValue(kZGrid);
            
            gridItem->setBaseColor(row < Board::ROWS / 2
                                       ? QColor(78, 52, 62)
                                       : QColor(54, 60, 92));

            m_scene->addItem(gridItem);
            m_gridItems.push_back(gridItem);

            const QRectF bounds = gridItem->boundingRect();
            totalBounds = first ? bounds : totalBounds.united(bounds);
            first = false;
        }
    }

    for (Unit* unit : m_units) {
        UnitItem* unitItem = new UnitItem(unit);
        unitItem->setZValue(kZUnit);
        m_scene->addItem(unitItem);
        m_unitItems.push_back(unitItem);
        m_unitItemById[unit->id()] = unitItem;

        connect(unitItem, &UnitItem::dragStarted,
                this, &Game::handleDragStarted);
        connect(unitItem, &UnitItem::dragMoved,
                this, &Game::handleDragMoved);
        connect(unitItem, &UnitItem::dragDropped,
                this, &Game::handleDropCommand);
    }

    m_scene->setSceneRect(totalBounds.adjusted(-40, -40, 40, 40));
}

void Game::syncFromBoard()
{
    clearGridHighlights();

    for (UnitItem* item : m_unitItems) {
        if (!item || !item->unit()) continue;

        Unit* unit = item->unit();
        const QPoint pos = unit->position();
        if (!unit->isAlive() || !m_board.isValidPosition(pos) || m_board.getUnitAt(pos) != unit) {
            item->setVisible(false);
            item->update();
            continue;
        }

        item->setVisible(true);
        item->setGridPos(pos);
        item->setPos(gridToWorld(pos.y(), pos.x()));
        item->setZValue(kZUnit);
        item->update();
    }
}

QPointF Game::gridToWorld(int row, int col) const
{
    const qreal colSpacing = m_radius * qSqrt(3.0);
    const qreal xOffset = (row % 2 == 0) ? colSpacing * 0.5 : 0.0;
    const qreal x = xOffset + col * colSpacing;
    const qreal y = row * m_rowSpacing;
    return QPointF(x, y);
}

QPoint Game::worldToGrid(const QPointF& world) const
{
    QPoint best(-1, -1);
    qreal bestDist = 1e18;

    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            const QPointF center = gridToWorld(row, col);
            const qreal dx = world.x() - center.x();
            const qreal dy = world.y() - center.y();
            const qreal d2 = dx * dx + dy * dy;
            if (d2 < bestDist) {
                bestDist = d2;
                best = QPoint(col, row);
            }
        }
    }

    return best;
}

QPoint Game::hitGridAt(const QPointF& world) const
{
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            if (cellHexPolygon(row, col).containsPoint(world, Qt::OddEvenFill)) {
                return QPoint(col, row);
            }
        }
    }

    return QPoint(-1, -1);
}

QPolygonF Game::cellHexPolygon(int row, int col) const
{
    const QPointF center = gridToWorld(row, col);
    QPolygonF poly;
    poly.reserve(6);

    for (int i = 0; i < 6; ++i) {
        const qreal angleDeg = 60.0 * i - 90.0;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        poly.append(QPointF(
            center.x() + m_radius * qCos(angleRad),
            center.y() + m_radius * qSin(angleRad)
        ));
    }

    return poly;
}

QString Game::statusText() const
{
    if (m_gameOver) {
        return QStringLiteral("游戏结束 — 玩家阵亡。点击「新开局」重新开始。");
    }
    return QStringLiteral("阶段：%1   回合：%2   金币：%3   等级：%4 (%5/%6)   人口：%7/%8   备战席：%9/8")
        .arg(phaseName(m_phase))
        .arg(m_round)
        .arg(m_player.gold())
        .arg(m_player.level())
        .arg(m_player.exp()).arg(m_player.expToNext())
        .arg(countDeployedPlayerUnits()).arg(m_player.boardCap())
        .arg(m_player.bench().size());
}

QString Game::playerStatsText() const
{
    return QStringLiteral("玩家血量：%1 / %2").arg(m_player.hp()).arg(m_player.maxHp());
}

QString Game::synergyText() const
{
    const QMap<Trait, int> counts = m_synergy.countTraits(m_units);
    QString text;

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        QString line = QStringLiteral("• %1：%2").arg(displayTraitName(it.key())).arg(it.value());
        const int n = it.value();
        bool active = false;
        if ((it.key() == Trait::Warrior || it.key() == Trait::Ranger
             || it.key() == Trait::Mage || it.key() == Trait::Guardian) && n >= 2) {
            active = true;
        }
        if (active) line += QStringLiteral("  ★");
        if (!text.isEmpty()) text += "\n";
        text += line;
    }

    return text.isEmpty() ? QStringLiteral("暂无羁绊") : text;
}

QString Game::equipmentText() const
{
    return QStringLiteral("铁剑：攻击 +15\n锁子甲：生命 +150\n急速手套：法力 +30 (加快放技能)\n蓝水晶：法力 +30");
}

QString Game::benchText() const
{
    QString text;
    int i = 0;
    for (Unit* u : m_player.bench()) {
        if (!u) continue;
        if (!text.isEmpty()) text += "\n";
        text += QStringLiteral("[%1] %2  %3★  HP %4")
            .arg(++i).arg(u->name()).arg(u->star()).arg(u->maxHp());
    }
    return text.isEmpty() ? QStringLiteral("（空）双击商店购买上场") : text;
}

bool Game::buyShopUnit(int index)
{
    if (m_gameOver) return false;
    const QVector<UnitTemplate>& offers = m_shop.offers();
    if (index < 0 || index >= offers.size()) {
        return false;
    }

    const int cost = offers.at(index).cost;
    if (m_player.benchFull() || !m_player.spendGold(cost)) {
        return false;
    }

    Unit* unit = m_shop.createUnitFromOffer(index, Owner::PlayerCtrl);
    if (!unit || !addUnitToBench(unit)) {
        delete unit;
        m_player.addGold(cost);
        return false;
    }

    return true;
}

bool Game::addUnitToBench(Unit* unit)
{
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_player.benchFull()) {
        return false;
    }

    m_units.append(unit);
    if (!m_player.addToBench(unit)) {
        m_units.removeAll(unit);
        return false;
    }

    tryMergeUnits();
    buildScene();
    syncFromBoard();
    emit stateChanged();
    return true;
}

bool Game::deployBenchUnit(int benchIndex)
{
    if (m_phase != GamePhase::Prep || m_gameOver) return false;
    if (benchIndex < 0 || benchIndex >= m_player.bench().size()) return false;
    if (countDeployedPlayerUnits() >= m_player.boardCap()) return false;

    Unit* u = m_player.bench().at(benchIndex);
    if (!u) return false;

    // find an empty slot on player half
    for (int row = Board::ROWS - 1; row >= Board::ROWS / 2; --row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QPoint p(col, row);
            if (!m_board.hasUnitAt(p)) {
                m_board.addUnit(u, p);
                m_player.removeFromBench(u);
                syncFromBoard();
                emit stateChanged();
                return true;
            }
        }
    }
    return false;
}

bool Game::sendUnitToBench(int unitId)
{
    if (m_phase != GamePhase::Prep || m_gameOver) return false;
    if (m_player.benchFull()) return false;
    Unit* u = findUnitById(unitId);
    if (!u || u->owner() != Owner::PlayerCtrl) return false;

    m_board.removeUnit(u);
    u->setPosition(QPoint(-1, -1));
    m_player.addToBench(u);
    syncFromBoard();
    emit stateChanged();
    return true;
}

void Game::tryMergeUnits()
{
    for (int i = 0; i < m_units.size(); ++i) {
        Unit* a = m_units.at(i);
        if (!a || a->owner() != Owner::PlayerCtrl || a->star() >= 3) {
            continue;
        }

        QVector<Unit*> same;
        for (Unit* b : m_units) {
            if (!b || b->owner() != Owner::PlayerCtrl) continue;
            if (b->name() == a->name() && b->star() == a->star()) same.append(b);
        }

        if (same.size() < 3) continue;

        Unit* upgraded = same.at(0);
        upgraded->setStar(upgraded->star() + 1);

        for (int k = 1; k < 3; ++k) {
            Unit* removed = same.at(k);
            m_board.removeUnit(removed);
            m_player.removeFromBench(removed);
            m_units.removeAll(removed);
            delete removed;
        }

        buildScene();
        syncFromBoard();
        emit stateChanged();
        return;
    }
}

void Game::refreshShop()
{
    if (m_gameOver) return;
    if (m_player.spendGold(2)) {
        m_shop.refresh();
        emit stateChanged();
    }
}

bool Game::buyExperience()
{
    if (m_gameOver) return false;
    const bool ok = m_player.buyExp(4, 4);
    if (ok) emit stateChanged();
    return ok;
}

bool Game::saveToFile(const QString& path)
{
    return SaveManager::saveToFile(path, *this);
}

bool Game::loadFromFile(const QString& path)
{
    const bool ok = SaveManager::loadFromFile(path, *this);
    if (ok) {
        m_gameOver = false;
        m_phase = GamePhase::Prep;
        buildScene();
        syncFromBoard();
        emit stateChanged();
    }
    return ok;
}

void Game::clearAllUnits()
{
    m_combatTimer->stop();
    m_board.clear();
    qDeleteAll(m_units);
    m_units.clear();
    m_player.clearBench();
}

void Game::installLoadedUnits(const QVector<Unit*>& all,
                              const QVector<Unit*>& boardUnits,
                              const QVector<Unit*>& benchUnits,
                              const QVector<Unit*>& enemyUnits)
{
    for (Unit* u : all) {
        m_units.append(u);
    }

    for (Unit* u : boardUnits) {
        const QPoint p = u->position();
        if (m_board.isValidPosition(p) && !m_board.hasUnitAt(p)) {
            m_board.addUnit(u, p);
        }
    }
    for (Unit* u : enemyUnits) {
        const QPoint p = u->position();
        if (m_board.isValidPosition(p) && !m_board.hasUnitAt(p)) {
            m_board.addUnit(u, p);
        }
    }
    for (Unit* u : benchUnits) {
        u->setPosition(QPoint(-1, -1));
        m_player.addToBench(u);
    }
}
