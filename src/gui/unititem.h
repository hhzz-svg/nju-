#ifndef GUI_ITEMS_UNITITEM_H
#define GUI_ITEMS_UNITITEM_H

#include <QGraphicsObject>
#include <QPoint>
#include <QPixmap>

class Unit;

class UnitItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit UnitItem(Unit* unit, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    Unit* unit() const { return m_unit; }
    int unitId() const;

    void setGridPos(const QPoint& gridPos);
    QPoint gridPos() const { return m_gridPos; }

signals:
    void dragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragDropped(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void ensureSpriteLoaded() const;
    QString spriteRelativePathForUnit() const;

    Unit* m_unit;
    QPoint m_gridPos;
    bool m_dragging;
    mutable QPixmap m_sprite;
    mutable bool m_spriteTried;

    // 被击白闪：检测到 hp 下降时记录起始时间，paint 时按已过的毫秒数算白色覆盖透明度
    mutable int m_lastHp = -1;
    mutable qint64 m_flashStartMs = 0;
};

#endif // GUI_ITEMS_UNITITEM_H
