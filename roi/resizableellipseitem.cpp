#include "resizableellipseitem.h"
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

ResizableEllipseItem::ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                           QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent)
    , m_handleType(None)
    , m_resizing(false)
{
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
}

ResizableEllipseItem::HandleType ResizableEllipseItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();
    const qreal handleSize = HANDLE_SIZE + 4;

    // 四条边
    QRectF top(rect.center().x() - handleSize, rect.top() - handleSize,
               handleSize * 2, handleSize * 2);
    if (top.contains(pos)) return Top;

    QRectF bottom(rect.center().x() - handleSize, rect.bottom() - handleSize,
                  handleSize * 2, handleSize * 2);
    if (bottom.contains(pos)) return Bottom;

    QRectF left(rect.left() - handleSize, rect.center().y() - handleSize,
                handleSize * 2, handleSize * 2);
    if (left.contains(pos)) return Left;

    QRectF right(rect.right() - handleSize, rect.center().y() - handleSize,
                 handleSize * 2, handleSize * 2);
    if (right.contains(pos)) return Right;

    // 椭圆内部（椭圆方程判断）
    if (rect.contains(pos)) {
        QPointF center = rect.center();
        qreal a = rect.width() / 2.0;
        qreal b = rect.height() / 2.0;
        if (a > 0 && b > 0) {
            qreal dx = (pos.x() - center.x()) / a;
            qreal dy = (pos.y() - center.y()) / b;
            if (dx*dx + dy*dy <= 1.0) return Move;
        }
    }
    return None;
}

void ResizableEllipseItem::updateCursor(const QPointF &pos)
{
    HandleType handle = getHandleAtPos(pos);
    switch (handle) {
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;
    case Move: setCursor(Qt::SizeAllCursor); break;
    default: setCursor(Qt::ArrowCursor); break;
    }
}

void ResizableEllipseItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) updateCursor(event->pos());
    QGraphicsEllipseItem::hoverMoveEvent(event);
}

void ResizableEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) setCursor(Qt::ArrowCursor);
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

void ResizableEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressPos = event->scenePos();
            m_originalRect = rect();
            if (m_handleType == Move)
                QGraphicsEllipseItem::mousePressEvent(event);
            return;
        }
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

void ResizableEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        QPointF delta = event->scenePos() - m_mousePressPos;
        QRectF newRect = m_originalRect;

        switch (m_handleType) {
        case Top:    newRect.setTop(m_originalRect.top() + delta.y()); break;
        case Bottom: newRect.setBottom(m_originalRect.bottom() + delta.y()); break;
        case Left:   newRect.setLeft(m_originalRect.left() + delta.x()); break;
        case Right:  newRect.setRight(m_originalRect.right() + delta.x()); break;
        default: break;
        }

        // 最小尺寸限制
        if (newRect.width() < 20) {
            if (m_handleType == Left)
                newRect.setLeft(newRect.right() - 20);
            else
                newRect.setRight(newRect.left() + 20);
        }
        if (newRect.height() < 20) {
            if (m_handleType == Top)
                newRect.setTop(newRect.bottom() - 20);
            else
                newRect.setBottom(newRect.top() + 20);
        }

        prepareGeometryChange();
        setRect(newRect);
        scene()->update(scene()->sceneRect());
        return;
    }
    QGraphicsEllipseItem::mouseMoveEvent(event);
}

void ResizableEllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_handleType = None;
        update();
        scene()->update(scene()->sceneRect());
    }
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void ResizableEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget)
{
    QStyleOptionGraphicsItem opt = *option;
    opt.state &= ~QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &opt, widget);
    if (isSelected() || m_resizing) drawHandles(painter);
}

void ResizableEllipseItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    QPen handlePen(Qt::blue, 1, Qt::SolidLine);

    QVector<QRectF> handles;
    handles.append(QRectF(rect.center().x() - HANDLE_SIZE/2, rect.top() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));
    handles.append(QRectF(rect.center().x() - HANDLE_SIZE/2, rect.bottom() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));
    handles.append(QRectF(rect.left() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));
    handles.append(QRectF(rect.right() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));

    for (const QRectF &h : handles) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(h);
        painter->setPen(handlePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(h);
    }
    painter->restore();
}

QVariant ResizableEllipseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged || change == ItemSelectedHasChanged) {
        scene()->update(scene()->sceneRect());
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}
