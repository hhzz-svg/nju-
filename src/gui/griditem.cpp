#include "griditem.h"
#include <QGraphicsSceneHoverEvent>
#include <QLinearGradient>
#include <QPainter>

GridItem::GridItem(int row, int col, const QPolygonF& polygon, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_row(row)
    , m_col(col)
    , m_polygon(polygon)
    , m_bounds(polygon.boundingRect())
    , m_baseColor(QColor(60, 60, 80))
    , m_hoverActive(false)
    , m_dropActive(false)
    , m_pointerHover(false)
{
    setAcceptHoverEvents(true);
}

QRectF GridItem::boundingRect() const
{
    return m_bounds.adjusted(-2.0, -2.0, 2.0, 2.0);
}

void GridItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    
    QColor topColor    = m_baseColor.lighter(118);
    QColor bottomColor = m_baseColor.darker(115);
    QColor borderColor = m_baseColor.darker(160);

    if (m_dropActive) {
        
        topColor    = QColor(238, 200, 110);
        bottomColor = QColor(196, 142, 60);
        borderColor = QColor(255, 220, 130);
    } else if (m_hoverActive || m_pointerHover) {
        topColor    = m_baseColor.lighter(140);
        bottomColor = m_baseColor.lighter(110);
    }

    QLinearGradient grad(m_bounds.topLeft(), m_bounds.bottomLeft());
    grad.setColorAt(0.0, topColor);
    grad.setColorAt(1.0, bottomColor);

    painter->setPen(QPen(borderColor, 1.4));
    painter->setBrush(grad);
    painter->drawPolygon(m_polygon);

    
    painter->setPen(QPen(QColor(255, 255, 255, 22), 1.0));
    painter->setBrush(Qt::NoBrush);
    QPolygonF inner;
    inner.reserve(m_polygon.size());
    const QPointF c = m_bounds.center();
    for (const QPointF& p : m_polygon) {
        inner.append(QPointF(p.x() * 0.92 + c.x() * 0.08,
                             p.y() * 0.92 + c.y() * 0.08));
    }
    painter->drawPolygon(inner);

    if (m_hoverActive || m_pointerHover) {
        // 悬停加一圈琥珀色描边
        painter->setPen(QPen(QColor(233, 180, 76), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(m_polygon);
    }
}

QPoint GridItem::gridPos() const
{
    return QPoint(m_col, m_row);
}

void GridItem::setBaseColor(const QColor& color)
{
    m_baseColor = color;
    update();
}

void GridItem::setHoverActive(bool active)
{
    if (m_hoverActive == active) {
        return;
    }
    m_hoverActive = active;
    update();
}

void GridItem::setDropActive(bool active)
{
    if (m_dropActive == active) {
        return;
    }
    m_dropActive = active;
    update();
}

void GridItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    if (!m_pointerHover) {
        m_pointerHover = true;
        update();
    }
}

void GridItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    if (m_pointerHover) {
        m_pointerHover = false;
        update();
    }
}
