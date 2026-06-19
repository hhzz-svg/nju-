#ifndef CORE_GAME_H
#define CORE_GAME_H

#include "board.h"
#include "combat.h"
#include "player.h"
#include "shop.h"
#include "synergy.h"
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QVector>
#include <unordered_map>
#include <vector>

class QGraphicsScene;
class QGraphicsItem;
class GridItem;
class QTimer;
class Unit;
class UnitItem;

// A short-lived visual effect (attack line or floating damage text).
struct VisualEffect {
    QGraphicsItem* item = nullptr;
    int kind = 0;
    int msLeft = 0;
    int totalMs = 0;
    double vy = 0.0;
};

class Game : public QObject
{
    Q_OBJECT

public:
    explicit Game(QObject* parent = nullptr);
    ~Game();

    void initialize();
    void reset();
    void startCombat();

    QGraphicsScene* scene() const { return m_scene; }
    Player* player() { return &m_player; }
    Player& playerRef() { return m_player; }
    Shop* shop() { return &m_shop; }
    GamePhase phase() const { return m_phase; }
    int round() const { return m_round; }
    QString statusText() const;
    QString synergyText() const;
    QString equipmentText() const;
    QString benchText() const;
    QString playerStatsText() const;

    const QVector<Unit*>& allUnits() const { return m_units; }

    void handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);

    bool buyShopUnit(int index);
    bool addUnitToBench(Unit* unit);
    bool deployBenchUnit(int benchIndex);
    bool sendUnitToBench(int unitId);
    void refreshShop();
    void tryMergeUnits();
    bool buyExperience();

    bool saveToFile(const QString& path);
    bool loadFromFile(const QString& path);

    void clearAllUnits();
    void setRound(int r) { m_round = r; }
    void installLoadedUnits(const QVector<Unit*>& all,
                            const QVector<Unit*>& boardUnits,
                            const QVector<Unit*>& benchUnits,
                            const QVector<Unit*>& enemyUnits);

signals:
    void stateChanged();
    void gameOver(bool playerWon);

private slots:
    void updateCombat();
    void tickEffects();

private:
    void spawnAttackEffects();
    void createStarterUnitsIfNeeded();
    void generateEnemiesForRound();
    void resolveCombat();
    Unit* findUnitById(int unitId) const;
    GridItem* findGridItem(const QPoint& gridPos) const;
    UnitItem* findUnitItem(int unitId) const;
    void clearGridHighlights();
    bool canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const;
    void applyDrop(int unitId, const QPoint& target);
    void buildScene();
    void syncFromBoard();
    int countDeployedPlayerUnits() const;

    QPointF gridToWorld(int row, int col) const;
    QPoint worldToGrid(const QPointF& world) const;
    QPoint hitGridAt(const QPointF& world) const;
    QPolygonF cellHexPolygon(int row, int col) const;

    Board m_board;
    QVector<Unit*> m_units;

    QGraphicsScene* m_scene;
    std::vector<GridItem*> m_gridItems;
    std::vector<UnitItem*> m_unitItems;

    bool m_dragActive;
    int m_activeUnitId;
    QPoint m_sourceGrid;
    std::unordered_map<int, UnitItem*> m_unitItemById;

    int m_rows;
    int m_cols;
    qreal m_radius;
    qreal m_rowSpacing;

    Player m_player;
    Shop m_shop;

    GamePhase m_phase;
    QTimer* m_combatTimer;
    QTimer* m_effectTimer;
    std::vector<VisualEffect> m_effects;
    CombatSystem m_combat;
    SynergySystem m_synergy;
    int m_round;
    bool m_gameOver;
};

#endif // CORE_GAME_H
