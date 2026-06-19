#include "gui/unititem.h"
#include "entity/unit.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>

UnitItem::UnitItem(Unit* unit, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_unit(unit)
    , m_gridPos(-1, -1)
    , m_dragging(false)
    , m_spriteTried(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF UnitItem::boundingRect() const
{
    return QRectF(-42, -42, 84, 84);
}

void UnitItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    
    if (m_unit) {
        const int curHp = m_unit->hp();
        if (m_lastHp >= 0 && curHp < m_lastHp) {
            m_flashStartMs = QDateTime::currentMSecsSinceEpoch();
            
            QTimer::singleShot(60, this, [this](){ update(); });
            QTimer::singleShot(140, this, [this](){ update(); });
        }
        m_lastHp = curHp;
    }

    ensureSpriteLoaded();

    if (!m_sprite.isNull()) {
        const QRectF targetRect(-40, -40, 80, 80);
        painter->drawPixmap(targetRect, m_sprite, m_sprite.rect());
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(20, 20, 20, 110));
        painter->drawEllipse(QRectF(-14, 8, 28, 10));

        QPolygonF badge;
        badge << QPointF(0, -15)
              << QPointF(13, -7)
              << QPointF(13, 7)
              << QPointF(0, 15)
              << QPointF(-13, 7)
              << QPointF(-13, -7);

        painter->setPen(QPen(QColor(18, 18, 18), 1.5));
        painter->setBrush(QColor(100, 150, 200));
        painter->drawPolygon(badge);
    }

    if (!m_unit) {
        return;
    }

    
    if (m_flashStartMs > 0) {
        const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_flashStartMs;
        if (elapsed >= 0 && elapsed < 130) {
            const double rate = 1.0 - static_cast<double>(elapsed) / 130.0;
            const int alpha = static_cast<int>(180 * rate);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(255, 255, 255, alpha));
            painter->drawEllipse(QRectF(-38, -38, 76, 76));
        } else {
            m_flashStartMs = 0;
        }
    }

    const double hpRate = m_unit->maxHp() > 0
        ? static_cast<double>(m_unit->hp()) / m_unit->maxHp()
        : 0.0;
    const double manaRate = m_unit->maxMana() > 0
        ? static_cast<double>(m_unit->mana()) / m_unit->maxMana()
        : 0.0;

    const QColor hpColor = m_unit->owner() == Owner::PlayerCtrl
        ? QColor(80, 220, 110)
        : QColor(230, 80, 80);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(30, 30, 30, 180));
    painter->drawRect(QRectF(-34, 30, 68, 6));
    painter->setBrush(hpColor);
    painter->drawRect(QRectF(-34, 30, 68 * hpRate, 6));

    painter->setBrush(QColor(30, 30, 30, 180));
    painter->drawRect(QRectF(-34, 38, 68, 5));
    painter->setBrush(QColor(70, 150, 255));
    painter->drawRect(QRectF(-34, 38, 68 * manaRate, 5));

    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(7);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(QRectF(-38, -42, 76, 12), Qt::AlignCenter,
                      QString("%1*").arg(m_unit->star()));
}

void UnitItem::ensureSpriteLoaded() const
{
    if (m_spriteTried) {
        return;
    }

    m_spriteTried = true;
    const QString relativePath = spriteRelativePathForUnit();
    if (relativePath.isEmpty()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString roots[] = {
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath()
    };

    QPixmap pix;
    for (const QString& root : roots) {
        if (root.isEmpty()) {
            continue;
        }
        pix.load(root + "/" + relativePath);
        if (!pix.isNull()) {
            break;
        }
    }

    if (pix.isNull()) {
        return;
    }

    m_sprite = pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString UnitItem::spriteRelativePathForUnit() const
{
    if (!m_unit) {
        return QString();
    }

    const QString name = m_unit->name();
    if (name == QStringLiteral("Knight")) {
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_1/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    }
    if (name == QStringLiteral("Archer")) {
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_01/PNG Sequences/Idle/Satyr_01_Idle_000.png");
    }
    if (name == QStringLiteral("Mage")) {
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_02/PNG Sequences/Idle/Satyr_02_Idle_000.png");
    }
    if (name == QStringLiteral("Goblin")) {
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_03/PNG Sequences/Idle/Satyr_03_Idle_000.png");
    }
    if (name == QStringLiteral("Brute")) {
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_3/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    }

    if (m_unit->owner() == Owner::PlayerCtrl) {
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_2/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    }
    return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_03/PNG Sequences/Idle/Satyr_03_Idle_000.png");
}

int UnitItem::unitId() const
{
    return m_unit ? m_unit->id() : -1;
}

void UnitItem::setGridPos(const QPoint& gridPos)
{
    m_gridPos = gridPos;
}

void UnitItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    emit dragStarted(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    emit dragMoved(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        return;
    }

    m_dragging = false;
    emit dragDropped(unitId(), m_gridPos, event->scenePos());
    event->accept();
}
