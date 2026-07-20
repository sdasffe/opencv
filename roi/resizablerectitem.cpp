/**
 * @file resizablerectitem.cpp
 * @brief ResizableRectItem 实现 —— ROI 矩形图元的完整交互范例
 *
 * 【坐标系说明】
 *   - event->pos()：鼠标在 item 本地坐标
 *   - event->scenePos()：场景坐标；缩放时用 scene 增量，避免 item 旋转/缩放时 delta 错误
 *   - rect()：QGraphicsRectItem 的矩形，原点在 item (0,0)，一般不随「移动」改变数值，
 *             移动由 setPos() 改变 item 在 scene 中的位置
 *
 * 【缩放实现要点】
 *   拖角/拖边 → 改 rect 的 top/left/right/bottom
 *   拖内部     → 基类 ItemIsMovable 改 pos
 *
 * 【性能】
 *   静止时用 DeviceCoordinateCache；缩放中 NoCache 避免残影
 */

#include "resizablerectitem.h"
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

/**
 * @brief 构造函数：设置交互标志、外观与缓存策略
 */
ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                                     QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)
    , m_handleType(None)
    , m_resizing(false)
{
    // ItemIsMovable：内部拖拽时 Qt 自动改 pos（整体平移）
    // ItemIsSelectable：可选中，配合 paint 显示手柄
    // ItemIsFocusable：可接收键盘（本类未用，预留）
    // ItemSendsGeometryChanges：pos/rect 变化时触发 itemChange
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true); // 必须开启才能收到 hoverMove/hoverLeave

    // 设备坐标缓存：图元复杂或静态时减少重绘；缩放中会临时关闭
    setCacheMode(DeviceCoordinateCache);

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine); // 虚线框，与图像内容区分
    setPen(pen);
}

/**
 * @brief 命中测试：鼠标在 item 本地坐标 pos 处对应哪个手柄
 *
 * 策略：在每个角/边中心构造比 HANDLE_SIZE 更大的正方形热区（+4 像素），
 *       优先检测四角，再四边，最后 rect 内部为 Move
 *
 * 为什么热区比视觉手柄大：便于用户点中，符合常见图形编辑器体验
 */
ResizableRectItem::HandleType ResizableRectItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();
    const qreal handleSize = HANDLE_SIZE + 4; // 热区 12×12，大于绘制 8×8

    // ---------- 四个角（对角 resize 光标）----------
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

    // ---------- 四条边（单方向 resize）----------
    // 顶边：水平条，左右留出角热区避免与角冲突
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

    // ---------- 内部：整体移动 ----------
    if (rect.contains(pos)) return Move;
    return None;
}

/**
 * @brief 根据命中手柄设置鼠标光标形状
 *
 * 缩放过程中不更新（m_resizing 时 hover 仍可能触发，由 hoverMoveEvent  guard）
 */
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

/**
 * @brief 鼠标按下：区分「缩放」与「移动」
 *
 * Move：调用基类 mousePressEvent，由 Qt 处理拖拽改 pos
 * 手柄：记录 m_resizing、m_originalRect、m_mousePressPos，关闭缓存
 */
void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_handleType = getHandleAtPos(event->pos());
        if (m_handleType != None) {
            m_resizing = true;
            m_mousePressPos = event->scenePos(); // scene 坐标存基准，move 时用 delta
            m_originalRect = rect();             // 以按下时的 rect 为缩放基准
            setCacheMode(NoCache);               // 动态改 rect 时避免缓存错位
            if (m_handleType == Move)
                QGraphicsRectItem::mousePressEvent(event); // 交给基类启动移动
            return; // 缩放手柄不交给基类，避免冲突
        }
    }
    QGraphicsRectItem::mousePressEvent(event);
}

/**
 * @brief 鼠标移动：拖手柄改 rect
 *
 * delta = 当前 scene 位置 - 按下 scene 位置（与 view 缩放无关的正确增量）
 * 各手柄只改 rect 对应边/角；Qt 的 setTopLeft 等会自动 normalize 宽高为正
 *
 * 最小尺寸 20×20：防止 rect 翻折或缩成不可见
 */
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

        // 最小尺寸限制：根据拖拽方向固定对边，防止宽/高 < 20
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

        prepareGeometryChange(); // 通知 scene：boundingRect 即将变化，需重算
        setRect(newRect);
        return;
    }
    // Move 或非缩放：基类处理 pos 变化
    QGraphicsRectItem::mouseMoveEvent(event);
}

/**
 * @brief 鼠标释放：结束缩放会话
 *
 * 恢复 DeviceCoordinateCache；scene()->update() 确保视图刷新
 * Widget 可在 release 后读取 sceneBoundingRect() 更新 RoiInfo
 */
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

/**
 * @brief 绘制：先画矩形本身，选中或缩放中再画手柄
 *
 * option/widget 传给基类，保留 Qt 样式（选中高亮等）
 */
void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    if (isSelected() || m_resizing) drawHandles(painter);
}

/**
 * @brief 绘制 8 个手柄：角点 + 四边中点
 *
 * 每个手柄：白底实心 + 蓝色描边，与红色虚线框区分
 * Antialiasing 关闭：小矩形边缘更清晰
 */
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

/**
 * @brief 图元状态变化回调
 *
 * ItemSelectedHasChanged：选中/取消选中时重绘以显示或隐藏手柄
 * 其他 change 交给基类（如 ItemPositionHasChanged 等）
 */
QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) update();
    return QGraphicsRectItem::itemChange(change, value);
}
