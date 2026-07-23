/**
 * @file resizablerectitem.cpp
 * @brief 轴对齐矩形 ROI 交互实现
 *
 * 拖角/拖边改 rect；拖内部走基类 ItemIsMovable 改 pos。
 * 缩放用 scene 增量；静止 DeviceCoordinateCache，缩放中 NoCache。
 */

#include "resizablerectitem.h"
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

/** @brief 创建可选中、可悬停的虚线红矩形 */
ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                                     QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)
    , m_handleType(None)
    , m_resizing(false)
{
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);               // 可移可选，几何变更通知
    setAcceptHoverEvents(true);                                      // 悬停换光标
    setCacheMode(DeviceCoordinateCache);                             // 静止时缓存，减重绘

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);                                      // 虚线框，与图像内容区分
    setPen(pen);
}

/** @brief 命中测试：角优先，再边，再内部 Move */
ResizableRectItem::HandleType ResizableRectItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();
    const qreal handleSize = HANDLE_SIZE + 4;                        // 热区略大于视觉手柄

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

    QRectF top(rect.topLeft() + QPointF(handleSize, -handleSize),
               QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (top.contains(pos)) return Top;                               // 顶边（避开角热区）

    QRectF bottom(rect.bottomLeft() + QPointF(handleSize, -handleSize),
                  QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (bottom.contains(pos)) return Bottom;

    QRectF left(rect.topLeft() + QPointF(-handleSize, handleSize),
                QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (left.contains(pos)) return Left;

    QRectF right(rect.topRight() + QPointF(-handleSize, handleSize),
                 QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (right.contains(pos)) return Right;

    if (rect.contains(pos)) return Move;                             // 内部：平移
    return None;
}

/** @brief 按命中手柄切换对角/单向/四向光标 */
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

/** @brief 悬停时刷新光标（拖动中不改） */
void ResizableRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) updateCursor(event->pos());
    QGraphicsRectItem::hoverMoveEvent(event);
}

/** @brief 离开图元恢复箭头光标 */
void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) setCursor(Qt::ArrowCursor);
    QGraphicsRectItem::hoverLeaveEvent(event);
}

/** @brief 左键按下：记录手柄与原始 rect；Move 交给基类 */
void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressPos = event->scenePos();                     // scene 基准，move 用 delta
            m_originalRect = rect();                                 // 缩放基准
            setCacheMode(NoCache);                                   // 动态改 rect 避免缓存错位
            if (m_handleType == Move)
                QGraphicsRectItem::mousePressEvent(event);           // 内部拖移走基类
            return;                                                  // 手柄不交给基类，避免冲突
        }
    }
    QGraphicsRectItem::mousePressEvent(event);
}

/** @brief 拖手柄改 rect；最小 20×20；Move 交给基类 */
void ResizableRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        QPointF delta = event->scenePos() - m_mousePressPos;         // scene 增量
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

        if (newRect.width() < 20) {                                  // 最小宽度
            if (m_handleType == Left || m_handleType == TopLeft || m_handleType == BottomLeft)
                newRect.setLeft(newRect.right() - 20);
            else
                newRect.setRight(newRect.left() + 20);
        }
        if (newRect.height() < 20) {                                 // 最小高度
            if (m_handleType == Top || m_handleType == TopLeft || m_handleType == TopRight)
                newRect.setTop(newRect.bottom() - 20);
            else
                newRect.setBottom(newRect.top() + 20);
        }

        prepareGeometryChange();                                     // 通知 scene 几何将变
        setRect(newRect);
        return;
    }
    QGraphicsRectItem::mouseMoveEvent(event);                        // Move：基类改 pos
}

/** @brief 松手结束缩放，恢复缓存并刷新场景 */
void ResizableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_handleType = None;
        setCacheMode(DeviceCoordinateCache);                         // 静止再开缓存
        update();
        if (scene()) scene()->update();
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
}

/** @brief 画矩形；选中或缩放中再画 8 手柄 */
void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    if (isSelected() || m_resizing) drawHandles(painter);
}

/** @brief 在四角与四边中点画白底蓝边小方块 */
void ResizableRectItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);          // 小方块像素对齐更清晰
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
        painter->drawRect(h);                                        // 白填充
        painter->setPen(handlePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(h);                                        // 蓝边框
    }
    painter->restore();
}

/** @brief 选中态变化时重绘以显示/隐藏手柄 */
QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) update();
    return QGraphicsRectItem::itemChange(change, value);
}
