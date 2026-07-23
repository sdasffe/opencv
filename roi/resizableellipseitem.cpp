/**
 * @file resizableellipseitem.cpp
 * @brief 椭圆 ROI 交互实现
 *
 * 与矩形版差异：getHandleAtPos 用 (dx/a)²+(dy/b)²≤1 判断内部移动；
 * 缩放仅四边；paint 时去掉 State_Selected 避免 Qt 默认选中框干扰虚线椭圆。
 */

#include "resizableellipseitem.h"
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

/** @brief 创建可选中、可悬停的虚线蓝椭圆 */
ResizableEllipseItem::ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                           QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent)
    , m_handleType(None)
    , m_resizing(false)
{
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);               // 可移可选，几何变更通知
    setAcceptHoverEvents(true);                                      // 悬停换光标

    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);                                      // 虚线轮廓，与矩形 ROI 区分
    setPen(pen);
}

/** @brief 先测四边热区，再用椭圆方程区分内部（Move）与外部（None） */
ResizableEllipseItem::HandleType ResizableEllipseItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();                                      // 本地外接矩形
    const qreal handleSize = HANDLE_SIZE + 4;                        // 热区略大于视觉手柄

    QRectF top(rect.center().x() - handleSize, rect.top() - handleSize,
               handleSize * 2, handleSize * 2);
    if (top.contains(pos)) return Top;                               // 顶边中点手柄

    QRectF bottom(rect.center().x() - handleSize, rect.bottom() - handleSize,
                  handleSize * 2, handleSize * 2);
    if (bottom.contains(pos)) return Bottom;                         // 底边

    QRectF left(rect.left() - handleSize, rect.center().y() - handleSize,
                handleSize * 2, handleSize * 2);
    if (left.contains(pos)) return Left;                             // 左边

    QRectF right(rect.right() - handleSize, rect.center().y() - handleSize,
                 handleSize * 2, handleSize * 2);
    if (right.contains(pos)) return Right;                           // 右边

    if (rect.contains(pos)) {                                        // 外接 rect 内再判椭圆
        QPointF center = rect.center();
        qreal a = rect.width() / 2.0;                                // 半长轴
        qreal b = rect.height() / 2.0;                               // 半短轴
        if (a > 0 && b > 0) {
            qreal dx = (pos.x() - center.x()) / a;
            qreal dy = (pos.y() - center.y()) / b;
            if (dx*dx + dy*dy <= 1.0) return Move;                   // 椭圆内：平移
        }
    }
    return None;                                                     // 四角空白等：不抢事件
}

/** @brief 按命中手柄切换 SizeVer / SizeHor / SizeAll */
void ResizableEllipseItem::updateCursor(const QPointF &pos)
{
    HandleType handle = getHandleAtPos(pos);
    switch (handle) {
    case Top: case Bottom: setCursor(Qt::SizeVerCursor); break;      // 上下调高
    case Left: case Right: setCursor(Qt::SizeHorCursor); break;      // 左右调宽
    case Move: setCursor(Qt::SizeAllCursor); break;                  // 内部拖移
    default: setCursor(Qt::ArrowCursor); break;
    }
}

/** @brief 悬停时刷新光标（拖动中不改） */
void ResizableEllipseItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) updateCursor(event->pos());
    QGraphicsEllipseItem::hoverMoveEvent(event);
}

/** @brief 离开图元恢复箭头光标 */
void ResizableEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) setCursor(Qt::ArrowCursor);
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

/** @brief 左键按下：记录手柄与原始 rect；Move 交给基类拖移 */
void ResizableEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());                 // 判定当前热区
        if (m_handleType != None) {
            m_resizing = true;                                       // 进入拖动手柄态
            m_mousePressPos = event->scenePos();                     // 场景坐标基准
            m_originalRect = rect();                                 // 按下时外接矩形快照
            if (m_handleType == Move)
                QGraphicsEllipseItem::mousePressEvent(event);        // 内部拖移走基类
            return;
        }
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

/** @brief 拖边缩放；最小宽高 20；Move 交给基类 */
void ResizableEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        QPointF delta = event->scenePos() - m_mousePressPos;         // 相对按下点位移
        QRectF newRect = m_originalRect;

        switch (m_handleType) {
        case Top:    newRect.setTop(m_originalRect.top() + delta.y()); break;
        case Bottom: newRect.setBottom(m_originalRect.bottom() + delta.y()); break;
        case Left:   newRect.setLeft(m_originalRect.left() + delta.x()); break;
        case Right:  newRect.setRight(m_originalRect.right() + delta.x()); break;
        default: break;
        }

        if (newRect.width() < 20) {                                  // 最小宽度
            if (m_handleType == Left)
                newRect.setLeft(newRect.right() - 20);
            else
                newRect.setRight(newRect.left() + 20);
        }
        if (newRect.height() < 20) {                                 // 最小高度
            if (m_handleType == Top)
                newRect.setTop(newRect.bottom() - 20);
            else
                newRect.setBottom(newRect.top() + 20);
        }

        prepareGeometryChange();                                     // 通知 scene 几何将变
        setRect(newRect);                                            // 更新外接矩形 → 椭圆变形
        scene()->update(scene()->sceneRect());                       // 全场景刷，清拖影
        return;
    }
    QGraphicsEllipseItem::mouseMoveEvent(event);
}

/** @brief 松手结束缩放，刷新场景 */
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

/** @brief 画虚线椭圆；去掉默认选中框，改画四边手柄 */
void ResizableEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget)
{
    QStyleOptionGraphicsItem opt = *option;
    opt.state &= ~QStyle::State_Selected;                            // 去掉 Qt 默认矩形选框
    QGraphicsEllipseItem::paint(painter, &opt, widget);              // 只画椭圆轮廓
    if (isSelected() || m_resizing) drawHandles(painter);            // 选中/拖动时显示手柄
}

/** @brief 在四边中点画白底蓝边小方块 */
void ResizableEllipseItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);          // 手柄像素对齐更清晰
    QPen handlePen(Qt::blue, 1, Qt::SolidLine);

    QVector<QRectF> handles;
    handles.append(QRectF(rect.center().x() - HANDLE_SIZE/2, rect.top() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));                // 上
    handles.append(QRectF(rect.center().x() - HANDLE_SIZE/2, rect.bottom() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));                // 下
    handles.append(QRectF(rect.left() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));                // 左
    handles.append(QRectF(rect.right() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
                          HANDLE_SIZE, HANDLE_SIZE));                // 右

    for (const QRectF &h : handles) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(h);                                        // 白填充
        painter->setPen(handlePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(h);                                        // 蓝边框
    }
    painter->restore();
}

/** @brief 位置或选中态变化时刷新场景 */
QVariant ResizableEllipseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged || change == ItemSelectedHasChanged) {
        scene()->update(scene()->sceneRect());
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}
