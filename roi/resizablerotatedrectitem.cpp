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

/** @brief 按宽高创建局部居中矩形，开启可选中/可悬停 */
ResizableRotatedRectItem::ResizableRotatedRectItem(qreal width, qreal height,
                                                   QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setLocalSize(width, height);                                     // 初始化 m_rect 居中于原点
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

/** @brief 改局部宽高（≥MIN_SIZE），transformOrigin 设为矩形中心 */
void ResizableRotatedRectItem::setLocalSize(qreal width, qreal height)
{
    prepareGeometryChange();
    const qreal w = qMax(width, MIN_SIZE);
    const qreal h = qMax(height, MIN_SIZE);
    m_rect = QRectF(-w / 2.0, -h / 2.0, w, h);                       // 始终以原点为中心
    setTransformOriginPoint(m_rect.center());                        // 旋转绕中心
    update();
}

/** @brief 含旋转手柄与四角热区的包围盒，供 scene 脏区与命中 */
QRectF ResizableRotatedRectItem::boundingRect() const
{
    const qreal pad = HANDLE_SIZE + ROTATE_OFFSET + 4;               // 预留手柄+旋转点外扩
    return m_rect.adjusted(-pad, -pad, pad, pad);
}

/** @brief 精确命中：矩形本体 + 各手柄热区 */
QPainterPath ResizableRotatedRectItem::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);                                            // 本体
    const qreal hs = HANDLE_SIZE + 4;
    for (HandleType t : {TopLeft, TopRight, BottomLeft, BottomRight, Rotate}) {
        const QPointF c = handleCenter(t);
        path.addRect(QRectF(c.x() - hs, c.y() - hs, hs * 2, hs * 2)); // 手柄可点区域
    }
    return path;
}

/** @brief 各手柄中心（局部坐标）；Rotate 在顶边上方 ROTATE_OFFSET */
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

/** @brief 命中测试：旋转点优先，再四角，再内部 Move */
ResizableRotatedRectItem::HandleType
ResizableRotatedRectItem::getHandleAtPos(const QPointF &pos) const
{
    const qreal hs = HANDLE_SIZE + 4;
    auto hit = [&](HandleType t) {
        const QPointF c = handleCenter(t);
        return QRectF(c.x() - hs, c.y() - hs, hs * 2, hs * 2).contains(pos);
    };

    if (hit(Rotate)) return Rotate;                                  // 旋转手柄最优先
    if (hit(TopLeft)) return TopLeft;
    if (hit(TopRight)) return TopRight;
    if (hit(BottomLeft)) return BottomLeft;
    if (hit(BottomRight)) return BottomRight;
    if (m_rect.contains(pos)) return Move;                           // 矩形内部：平移
    return None;
}

/** @brief 按手柄类型设置对角/交叉/四向光标 */
void ResizableRotatedRectItem::updateCursor(const QPointF &pos)
{
    switch (getHandleAtPos(pos)) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor); break;                       // 主对角缩放
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;                       // 副对角缩放
    case Rotate:
        setCursor(Qt::CrossCursor); break;                           // 旋转
    case Move:
        setCursor(Qt::SizeAllCursor); break;                         // 平移
    default:
        setCursor(Qt::ArrowCursor); break;
    }
}

/** @brief 悬停刷新光标 */
void ResizableRotatedRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing)
        updateCursor(event->pos());
    QGraphicsObject::hoverMoveEvent(event);
}

/** @brief 离开恢复箭头 */
void ResizableRotatedRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing)
        setCursor(Qt::ArrowCursor);
    QGraphicsObject::hoverLeaveEvent(event);
}

/** @brief 按下记录手柄、原始 rect/角度；Move 交给基类 */
void ResizableRotatedRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressScenePos = event->scenePos();                // 场景按下点
            m_mousePressItemPos = event->pos();                      // 局部按下点
            m_originalRect = m_rect;                                 // 缩放基准
            m_originalRotation = rotation();                         // 旋转基准（当前未用差分）
            if (m_handleType == Move)
                QGraphicsObject::mousePressEvent(event);             // 平移走基类
            event->accept();
            return;
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

/** @brief 旋转用 atan2；角缩放固定对角点，最小 MIN_SIZE */
void ResizableRotatedRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_resizing || m_handleType == None || m_handleType == Move) {
        QGraphicsObject::mouseMoveEvent(event);                      // Move/未激活交给基类
        return;
    }

    if (m_handleType == Rotate) {
        const QPointF centerScene = mapToScene(m_rect.center());     // 旋转中心（场景）
        const QPointF v = event->scenePos() - centerScene;
        const qreal angle = qRadiansToDegrees(qAtan2(v.y(), v.x())) + 90.0; // +90：顶边朝上为 0°
        setTransformOriginPoint(m_rect.center());
        setRotation(angle);
        event->accept();
        return;
    }

    QPointF fixed;                                                   // 对角固定点
    switch (m_handleType) {
    case TopLeft:     fixed = m_originalRect.bottomRight(); break;
    case TopRight:    fixed = m_originalRect.bottomLeft(); break;
    case BottomLeft:  fixed = m_originalRect.topRight(); break;
    case BottomRight: fixed = m_originalRect.topLeft(); break;
    default: break;
    }

    QRectF newRect = QRectF(fixed, event->pos()).normalized();       // 固定点↔鼠标构成新矩形
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
    setTransformOriginPoint(m_rect.center());                        // 缩放后仍绕中心转
    update();
    event->accept();
}

/** @brief 松手：局部矩形重新居中，pos 对齐 scene 中心 */
void ResizableRotatedRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_handleType = None;
        const QPointF centerScene = mapToScene(m_rect.center());     // 缩放后可能偏移的中心
        const qreal w = m_rect.width();
        const qreal h = m_rect.height();
        prepareGeometryChange();
        m_rect = QRectF(-w / 2.0, -h / 2.0, w, h);                   // 拉回原点居中
        setTransformOriginPoint(m_rect.center());
        setPos(centerScene);                                         // scene pos = 中心，对齐 RoiInfo
        update();
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

/** @brief 画琥珀色虚线矩形、旋转连线；选中时画手柄 */
void ResizableRotatedRectItem::paint(QPainter *painter,
                                     const QStyleOptionGraphicsItem *option,
                                     QWidget *widget)
{
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(245, 158, 11), 2, Qt::DashLine);                 // 琥珀色虚线本体
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_rect);

    painter->setPen(QPen(QColor(245, 158, 11), 1, Qt::SolidLine));
    painter->drawLine(QPointF(m_rect.center().x(), m_rect.top()), handleCenter(Rotate)); // 顶→旋转点

    if ((option->state & QStyle::State_Selected) || m_resizing)
        drawHandles(painter);
}

/** @brief 四角方块 + 顶部圆形旋转手柄 */
void ResizableRotatedRectItem::drawHandles(QPainter *painter) const
{
    painter->save();
    const QPen handlePen(QColor(37, 99, 235), 1);                    // 蓝色缩放手柄边

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
    painter->setBrush(QColor(245, 158, 11));                         // 琥珀色实心旋转点
    painter->drawEllipse(rc, HANDLE_SIZE / 2 + 1, HANDLE_SIZE / 2 + 1);
    painter->setPen(handlePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rc, HANDLE_SIZE / 2 + 1, HANDLE_SIZE / 2 + 1);

    painter->restore();
}

/** @brief 选中态变化时重绘手柄 */
QVariant ResizableRotatedRectItem::itemChange(GraphicsItemChange change,
                                              const QVariant &value)
{
    if (change == ItemSelectedHasChanged)
        update();
    return QGraphicsObject::itemChange(change, value);
}
