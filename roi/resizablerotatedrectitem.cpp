/**
 * @file resizablerotatedrectitem.cpp
 * @brief 旋转矩形 ROI：局部居中几何 + scene 位姿
 *
 * 缩放时对角固定一点；旋转手柄用 scene 坐标 atan2 算角度。
 * mouseRelease 后把 m_rect 重新居中到局部原点，pos 设为 scene 中心，便于与 RoiInfo 互转。
 */

#include "resizablerotatedrectitem.h"

#include <QtMath>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

ResizableRotatedRectItem::ResizableRotatedRectItem(qreal width, qreal height,
                                                   QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setLocalSize(width, height);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

void ResizableRotatedRectItem::setLocalSize(qreal width, qreal height)
{
    prepareGeometryChange();
    const qreal w = qMax(width, MIN_SIZE);
    const qreal h = qMax(height, MIN_SIZE);
    m_rect = QRectF(-w / 2.0, -h / 2.0, w, h);
    setTransformOriginPoint(m_rect.center());
    update();
}

/** 含旋转手柄与四角热区的包围盒，供 scene 脏区与命中范围 */
QRectF ResizableRotatedRectItem::boundingRect() const
{
    const qreal pad = HANDLE_SIZE + ROTATE_OFFSET + 4;
    return m_rect.adjusted(-pad, -pad, pad, pad);
}

QPainterPath ResizableRotatedRectItem::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);
    const qreal hs = HANDLE_SIZE + 4;
    for (HandleType t : {TopLeft, TopRight, BottomLeft, BottomRight, Rotate}) {
        const QPointF c = handleCenter(t);
        path.addRect(QRectF(c.x() - hs, c.y() - hs, hs * 2, hs * 2));
    }
    return path;
}

QPointF ResizableRotatedRectItem::handleCenter(HandleType type) const
{
    switch (type) {
    case TopLeft:     return m_rect.topLeft();
    case TopRight:    return m_rect.topRight();
    case BottomLeft:  return m_rect.bottomLeft();
    case BottomRight: return m_rect.bottomRight();
    case Rotate:      return QPointF(m_rect.center().x(), m_rect.top() - ROTATE_OFFSET);
    default:          return QPointF();
    }
}

ResizableRotatedRectItem::HandleType
ResizableRotatedRectItem::getHandleAtPos(const QPointF &pos) const
{
    const qreal hs = HANDLE_SIZE + 4;
    auto hit = [&](HandleType t) {
        const QPointF c = handleCenter(t);
        return QRectF(c.x() - hs, c.y() - hs, hs * 2, hs * 2).contains(pos);
    };

    if (hit(Rotate)) return Rotate;
    if (hit(TopLeft)) return TopLeft;
    if (hit(TopRight)) return TopRight;
    if (hit(BottomLeft)) return BottomLeft;
    if (hit(BottomRight)) return BottomRight;
    if (m_rect.contains(pos)) return Move;
    return None;
}

void ResizableRotatedRectItem::updateCursor(const QPointF &pos)
{
    switch (getHandleAtPos(pos)) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Rotate:
        setCursor(Qt::CrossCursor);
        break;
    case Move:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void ResizableRotatedRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing)
        updateCursor(event->pos());
    QGraphicsObject::hoverMoveEvent(event);
}

void ResizableRotatedRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing)
        setCursor(Qt::ArrowCursor);
    QGraphicsObject::hoverLeaveEvent(event);
}

void ResizableRotatedRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressScenePos = event->scenePos();
            m_mousePressItemPos = event->pos();
            m_originalRect = m_rect;
            m_originalRotation = rotation();
            if (m_handleType == Move)
                QGraphicsObject::mousePressEvent(event);
            event->accept();
            return;
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

void ResizableRotatedRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_resizing || m_handleType == None || m_handleType == Move) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    // 旋转：鼠标相对 scene 中心的方向角 +90° 使 0° 对应矩形「顶边朝上」
    if (m_handleType == Rotate) {
        const QPointF centerScene = mapToScene(m_rect.center());
        const QPointF v = event->scenePos() - centerScene;
        const qreal angle = qRadiansToDegrees(qAtan2(v.y(), v.x())) + 90.0;
        setTransformOriginPoint(m_rect.center());
        setRotation(angle);
        event->accept();
        return;
    }

    // 拖角缩放：对角顶点 fixed 不动，当前鼠标位置为对角点
    QPointF fixed;
    switch (m_handleType) {
    case TopLeft:     fixed = m_originalRect.bottomRight(); break;
    case TopRight:    fixed = m_originalRect.bottomLeft(); break;
    case BottomLeft:  fixed = m_originalRect.topRight(); break;
    case BottomRight: fixed = m_originalRect.topLeft(); break;
    default: break;
    }

    QRectF newRect = QRectF(fixed, event->pos()).normalized();
    if (newRect.width() < MIN_SIZE) {
        if (event->pos().x() < fixed.x())
            newRect.setLeft(fixed.x() - MIN_SIZE);
        else
            newRect.setRight(fixed.x() + MIN_SIZE);
    }
    if (newRect.height() < MIN_SIZE) {
        if (event->pos().y() < fixed.y())
            newRect.setTop(fixed.y() - MIN_SIZE);
        else
            newRect.setBottom(fixed.y() + MIN_SIZE);
    }

    prepareGeometryChange();
    m_rect = newRect;
    setTransformOriginPoint(m_rect.center());
    update();
    event->accept();
}

void ResizableRotatedRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_handleType = None;
        // 松手后把矩形重新居中到局部原点，避免长期偏移
        const QPointF centerScene = mapToScene(m_rect.center());
        const qreal w = m_rect.width();
        const qreal h = m_rect.height();
        prepareGeometryChange();
        m_rect = QRectF(-w / 2.0, -h / 2.0, w, h);
        setTransformOriginPoint(m_rect.center());
        setPos(centerScene);
        update();
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

void ResizableRotatedRectItem::paint(QPainter *painter,
                                     const QStyleOptionGraphicsItem *option,
                                     QWidget *widget)
{
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(245, 158, 11), 2, Qt::DashLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_rect);

    painter->setPen(QPen(QColor(245, 158, 11), 1, Qt::SolidLine));
    painter->drawLine(QPointF(m_rect.center().x(), m_rect.top()), handleCenter(Rotate));

    if ((option->state & QStyle::State_Selected) || m_resizing)
        drawHandles(painter);
}

void ResizableRotatedRectItem::drawHandles(QPainter *painter) const
{
    painter->save();
    const QPen handlePen(QColor(37, 99, 235), 1);

    auto drawBox = [&](const QPointF &c) {
        const QRectF h(c.x() - HANDLE_SIZE / 2, c.y() - HANDLE_SIZE / 2,
                       HANDLE_SIZE, HANDLE_SIZE);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(h);
        painter->setPen(handlePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(h);
    };

    drawBox(handleCenter(TopLeft));
    drawBox(handleCenter(TopRight));
    drawBox(handleCenter(BottomLeft));
    drawBox(handleCenter(BottomRight));

    const QPointF rc = handleCenter(Rotate);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(245, 158, 11));
    painter->drawEllipse(rc, HANDLE_SIZE / 2 + 1, HANDLE_SIZE / 2 + 1);
    painter->setPen(handlePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rc, HANDLE_SIZE / 2 + 1, HANDLE_SIZE / 2 + 1);

    painter->restore();
}

QVariant ResizableRotatedRectItem::itemChange(GraphicsItemChange change,
                                              const QVariant &value)
{
    if (change == ItemSelectedHasChanged)
        update();
    return QGraphicsObject::itemChange(change, value);
}
