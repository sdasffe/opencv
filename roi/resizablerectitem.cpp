#include "resizablerectitem.h"
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                                     QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)
    , m_handleType(None)
    , m_resizing(false)
{
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
}

ResizableRectItem::HandleType ResizableRectItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();
    const qreal handleSize = HANDLE_SIZE + 4;

    // 四个角
    QRectF topLeft(rect.topLeft() - QPointF(handleSize, handleSize),
                   QSizeF(handleSize * 2, handleSize * 2));
    if (topLeft.contains(pos)) return TopLeft;

    QRectF topRight(rect.topRight() - QPointF(handleSize, handleSize),
                    QSizeF(handleSize * 2, handleSize * 2));
    if (topRight.contains(pos)) return TopRight;

    QRectF bottomLeft(rect.bottomLeft() - QPointF(handleSize, handleSize),
                      QSizeF(handleSize * 2, handleSize * 2));
    if (bottomLeft.contains(pos)) return BottomLeft;

    QRectF bottomRight(rect.bottomRight() - QPointF(handleSize, handleSize),
                       QSizeF(handleSize * 2, handleSize * 2));
    if (bottomRight.contains(pos)) return BottomRight;

    // 四条边
    QRectF top(rect.topLeft() + QPointF(handleSize, -handleSize),
               QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (top.contains(pos)) return Top;

    QRectF bottom(rect.bottomLeft() + QPointF(handleSize, -handleSize),
                  QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (bottom.contains(pos)) return Bottom;

    QRectF left(rect.topLeft() + QPointF(-handleSize, handleSize),
                QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (left.contains(pos)) return Left;

    QRectF right(rect.topRight() + QPointF(-handleSize, handleSize),
                 QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (right.contains(pos)) return Right;

    // 内部
    if (rect.contains(pos)) return Move;
    return None;
}

void ResizableRectItem::updateCursor(const QPointF &pos)
{
    HandleType handle = getHandleAtPos(pos);
    switch (handle) {
    case TopLeft: case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case TopRight: case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;
    case Move: setCursor(Qt::SizeAllCursor); break;
    default: setCursor(Qt::ArrowCursor); break;
    }
}

void ResizableRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) updateCursor(event->pos());
    QGraphicsRectItem::hoverMoveEvent(event);
}

void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) setCursor(Qt::ArrowCursor);
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressPos = event->scenePos();
            m_originalRect = rect();
            setCacheMode(NoCache);
            if (m_handleType == Move)
                QGraphicsRectItem::mousePressEvent(event);
            return;
        }
    }
    QGraphicsRectItem::mousePressEvent(event);
}

void ResizableRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        QPointF delta = event->scenePos() - m_mousePressPos;
        QRectF newRect = m_originalRect;

        switch (m_handleType) {
        case TopLeft:    newRect.setTopLeft(m_originalRect.topLeft() + delta); break;
        case TopRight:   newRect.setTopRight(m_originalRect.topRight() + delta); break;
        case BottomLeft: newRect.setBottomLeft(m_originalRect.bottomLeft() + delta); break;
        case BottomRight:newRect.setBottomRight(m_originalRect.bottomRight() + delta); break;
        case Top:        newRect.setTop(m_originalRect.top() + delta.y()); break;
        case Bottom:     newRect.setBottom(m_originalRect.bottom() + delta.y()); break;
        case Left:       newRect.setLeft(m_originalRect.left() + delta.x()); break;
        case Right:      newRect.setRight(m_originalRect.right() + delta.x()); break;
        default: break;
        }

        // 最小尺寸限制
        if (newRect.width() < 20) {
            if (m_handleType == Left || m_handleType == TopLeft || m_handleType == BottomLeft)
                newRect.setLeft(newRect.right() - 20);
            else
                newRect.setRight(newRect.left() + 20);
        }
        if (newRect.height() < 20) {
            if (m_handleType == Top || m_handleType == TopLeft || m_handleType == TopRight)
                newRect.setTop(newRect.bottom() - 20);
            else
                newRect.setBottom(newRect.top() + 20);
        }

        prepareGeometryChange();
        setRect(newRect);
        return;
    }
    QGraphicsRectItem::mouseMoveEvent(event);
}

void ResizableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_handleType = None;
        setCacheMode(DeviceCoordinateCache);
        update();
        if (scene()) scene()->update();
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
}

void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    if (isSelected() || m_resizing) drawHandles(painter);
}

void ResizableRectItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    QPen handlePen(Qt::blue, 1, Qt::SolidLine);

    QVector<QRectF> cornerHandles = {
        QRectF(rect.topLeft() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        QRectF(rect.topRight() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        QRectF(rect.bottomLeft() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        QRectF(rect.bottomRight() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE))
    };
    QVector<QRectF> edgeHandles = {
        QRectF(rect.center().x() - HANDLE_SIZE/2, rect.top() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        QRectF(rect.center().x() - HANDLE_SIZE/2, rect.bottom() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        QRectF(rect.left() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        QRectF(rect.right() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE)
    };

    for (const QRectF &h : cornerHandles + edgeHandles) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(h);
        painter->setPen(handlePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(h);
    }
    painter->restore();
}

QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) update();
    return QGraphicsRectItem::itemChange(change, value);
}
