#include "resizablerectitem.h"
#include <QGraphicsView>             // 图形视图
#include <QStyleOptionGraphicsItem>  // 图形项绘制选项
#include <QGraphicsScene>            // 场景

// ========== 构造函数 ==========
ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                                     QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)  // 调用父类构造，设置矩形位置和大小
    , m_handleType(None)       // 初始没有激活的手柄
    , m_resizing(false)        // 初始不在调整状态
{
    // ========== 设置图形项的交互标志 ==========
    setFlags(QGraphicsItem::ItemIsMovable |           // 允许移动
             QGraphicsItem::ItemIsSelectable |        // 允许选中
             QGraphicsItem::ItemIsFocusable |         // 允许获得焦点
             QGraphicsItem::ItemSendsGeometryChanges); // 几何变化时发送通知（重要！）

    setAcceptHoverEvents(true);  // 启用悬停事件（用于光标跟随变化）

    // 启用缓存优化绘制（减少重复绘制开销）
    setCacheMode(DeviceCoordinateCache);

    // ========== 设置矩形边框样式 ==========
    QPen pen;
    pen.setColor(Qt::red);       // 红色边框
    pen.setWidth(2);             // 线宽2像素
    pen.setStyle(Qt::DashLine);  // 虚线样式
    setPen(pen);
}

// ========== 判断鼠标位置对应的手柄类型 ==========
// 按优先级从高到低检测：四个角 → 四条边 → 矩形内部 → 无
ResizableRectItem::HandleType ResizableRectItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();  // 获取矩形区域

    // 手柄的实际检测区域比视觉大4像素，降低点击难度
    const qreal handleSize = HANDLE_SIZE + 4;

    // ---------- 检测四个角手柄 ----------
    // 每个角的检测区域是一个以角点为中心的正方形

    // 左上角：矩形左上角向左上偏移
    QRectF topLeft(rect.topLeft() - QPointF(handleSize, handleSize),
                   QSizeF(handleSize * 2, handleSize * 2));
    if (topLeft.contains(pos)) return TopLeft;

    // 右上角：矩形右上角向右上偏移
    QRectF topRight(rect.topRight() - QPointF(handleSize, handleSize),
                    QSizeF(handleSize * 2, handleSize * 2));
    if (topRight.contains(pos)) return TopRight;

    // 左下角：矩形左下角向左下偏移
    QRectF bottomLeft(rect.bottomLeft() - QPointF(handleSize, handleSize),
                      QSizeF(handleSize * 2, handleSize * 2));
    if (bottomLeft.contains(pos)) return BottomLeft;

    // 右下角：矩形右下角向右下偏移
    QRectF bottomRight(rect.bottomRight() - QPointF(handleSize, handleSize),
                       QSizeF(handleSize * 2, handleSize * 2));
    if (bottomRight.contains(pos)) return BottomRight;

    // ---------- 检测四条边中间手柄 ----------
    // 每条边的检测区域是在边外侧的一个矩形条

    // 上边中间：矩形上方，水平居中
    QRectF top(rect.topLeft() + QPointF(handleSize, -handleSize),
               QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (top.contains(pos)) return Top;

    // 下边中间：矩形下方，水平居中
    QRectF bottom(rect.bottomLeft() + QPointF(handleSize, -handleSize),
                  QSizeF(rect.width() - 2*handleSize, handleSize * 2));
    if (bottom.contains(pos)) return Bottom;

    // 左边中间：矩形左侧，垂直居中
    QRectF left(rect.topLeft() + QPointF(-handleSize, handleSize),
                QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (left.contains(pos)) return Left;

    // 右边中间：矩形右侧，垂直居中
    QRectF right(rect.topRight() + QPointF(-handleSize, handleSize),
                 QSizeF(handleSize * 2, rect.height() - 2*handleSize));
    if (right.contains(pos)) return Right;

    // ---------- 检测矩形内部（移动操作） ----------
    // 只有前面都没命中，且鼠标在矩形内部时，才判定为移动
    if (rect.contains(pos)) return Move;

    // 不在任何可交互区域
    return None;
}

// ========== 根据手柄类型更新光标样式 ==========
void ResizableRectItem::updateCursor(const QPointF &pos)
{
    HandleType handle = getHandleAtPos(pos);  // 先判断位置对应的手柄

    switch (handle) {
    case TopLeft:       // 左上角 → ↖↘ 方向光标
    case BottomRight:   // 右下角 → ↖↘ 方向光标（同一对角线）
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:      // 右上角 → ↙↗ 方向光标
    case BottomLeft:    // 左下角 → ↙↗ 方向光标（同一对角线）
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Top:           // 上边 → ↕ 垂直光标
    case Bottom:        // 下边 → ↕ 垂直光标
        setCursor(Qt::SizeVerCursor);
        break;
    case Left:          // 左边 → ↔ 水平光标
    case Right:         // 右边 → ↔ 水平光标
        setCursor(Qt::SizeHorCursor);
        break;
    case Move:          // 矩形内部 → 十字移动光标
        setCursor(Qt::SizeAllCursor);
        break;
    default:            // 矩形外部 → 普通箭头
        setCursor(Qt::ArrowCursor);
        break;
    }
}

// ========== 鼠标悬停移动事件 ==========
// 鼠标在矩形上移动时，实时更新光标样式
void ResizableRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) {  // 调整过程中不改变光标（保持拖拽时的光标）
        updateCursor(event->pos());
    }
    QGraphicsRectItem::hoverMoveEvent(event);  // 调用父类处理
}

// ========== 鼠标离开事件 ==========
// 鼠标离开矩形区域时，恢复默认箭头光标
void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) {  // 调整过程中不改变光标
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsRectItem::hoverLeaveEvent(event);
}

// ========== 鼠标按下事件（开始拖拽） ==========
void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {  // 只响应左键
        m_handleType = getHandleAtPos(event->pos());  // 判断按下的位置

        if (m_handleType != None) {  // 按在有效交互区域
            m_resizing = true;                    // 进入调整状态
            m_mousePressPos = event->scenePos();  // 记录按下的场景坐标（用于计算拖拽距离）
            m_originalRect = rect();              // 记录原始矩形（用于计算新形状）

            // 拖动时禁用缓存，避免显示旧内容产生拖影
            setCacheMode(NoCache);

            if (m_handleType == Move) {
                // 移动操作：交给父类 QGraphicsItem 的内置移动机制处理
                QGraphicsRectItem::mousePressEvent(event);
            }
            return;  // 事件已处理
        }
    }
    QGraphicsRectItem::mousePressEvent(event);  // 非有效区域，正常传递
}

// ========== 鼠标移动事件（实时调整大小） ==========
void ResizableRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 只在调整大小状态且不是移动操作时才自定义处理
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        // 计算鼠标相对按下位置的偏移量
        QPointF delta = event->scenePos() - m_mousePressPos;
        QRectF newRect = m_originalRect;  // 从原始矩形出发计算

        // 根据手柄类型，调整矩形不同的边
        switch (m_handleType) {
        case TopLeft:      // 拖拽左上角 → 同时移动上边和左边
            newRect.setTopLeft(m_originalRect.topLeft() + delta);
            break;
        case TopRight:     // 拖拽右上角 → 同时移动上边和右边
            newRect.setTopRight(m_originalRect.topRight() + delta);
            break;
        case BottomLeft:   // 拖拽左下角 → 同时移动下边和左边
            newRect.setBottomLeft(m_originalRect.bottomLeft() + delta);
            break;
        case BottomRight:  // 拖拽右下角 → 同时移动下边和右边
            newRect.setBottomRight(m_originalRect.bottomRight() + delta);
            break;
        case Top:          // 拖拽上边中间 → 只移动上边（只改变y和高度）
            newRect.setTop(m_originalRect.top() + delta.y());
            break;
        case Bottom:       // 拖拽下边中间 → 只移动下边
            newRect.setBottom(m_originalRect.bottom() + delta.y());
            break;
        case Left:         // 拖拽左边中间 → 只移动左边（只改变x和宽度）
            newRect.setLeft(m_originalRect.left() + delta.x());
            break;
        case Right:        // 拖拽右边中间 → 只移动右边
            newRect.setRight(m_originalRect.right() + delta.x());
            break;
        default:
            break;
        }

        // ---------- 限制最小尺寸（防止矩形太小或反向） ----------
        // 宽度最小20像素
        if (newRect.width() < 20) {
            if (m_handleType == Left || m_handleType == TopLeft || m_handleType == BottomLeft)
                newRect.setLeft(newRect.right() - 20);   // 拖拽左边时固定右边
            else
                newRect.setRight(newRect.left() + 20);    // 拖拽右边时固定左边
        }

        // 高度最小20像素
        if (newRect.height() < 20) {
            if (m_handleType == Top || m_handleType == TopLeft || m_handleType == TopRight)
                newRect.setTop(newRect.bottom() - 20);   // 拖拽上边时固定下边
            else
                newRect.setBottom(newRect.top() + 20);    // 拖拽下边时固定上边
        }

        // 通知场景几何即将变化（重要！避免拖影）
        prepareGeometryChange();
        setRect(newRect);  // 应用新矩形
        emit rectChanged();  // ✅ 通知矩形改变
        return;  // 事件已处理
    }

    QGraphicsRectItem::mouseMoveEvent(event);  // 移动操作交给父类
    if (m_resizing && m_handleType == Move) {
        emit rectChanged();  // ✅ 移动时也通知
    }
}

// ========== 鼠标释放事件（结束调整） ==========
void ResizableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {  // 之前在调整状态
        m_resizing = false;   // 退出调整状态
        m_handleType = None;  // 清除激活的手柄

        // 恢复缓存以提高后续性能
        setCacheMode(DeviceCoordinateCache);

        // 强制更新，确保最终状态正确显示
        update();
        if (scene()) {
            scene()->update();  // 更新整个场景，清除可能的拖影
        }
        emit rectChanged();  // ✅ 通知矩形改变
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
}

// ========== 绘制矩形和手柄 ==========
void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    // 先绘制矩形本身（红色虚线框）
    QGraphicsRectItem::paint(painter, option, widget);

    // 如果被选中或正在调整大小，额外绘制8个手柄
    if (isSelected() || m_resizing) {
        drawHandles(painter);
    }
}

// ========== 绘制8个调整手柄 ==========
// 手柄位置示意：
//   ■──────────■──────────■
//   │                     │
//   ■                     ■
//   │      矩形区域       │
//   ■                     ■
//   │                     │
//   ■──────────■──────────■
void ResizableRectItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();  // 获取矩形区域
    painter->save();  // 保存绘图状态，便于恢复

    // 关闭抗锯齿，手柄边缘更清晰锐利
    painter->setRenderHint(QPainter::Antialiasing, false);

    // 手柄边框：蓝色实线
    QPen handlePen(Qt::blue, 1, Qt::SolidLine);

    // ========== 四个角的手柄（以角点为中心的小正方形） ==========
    QVector<QRectF> cornerHandles = {
        // 左上角：角点向左上偏移半个手柄大小
        QRectF(rect.topLeft() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        // 右上角
        QRectF(rect.topRight() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        // 左下角
        QRectF(rect.bottomLeft() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE)),
        // 右下角
        QRectF(rect.bottomRight() - QPointF(HANDLE_SIZE/2, HANDLE_SIZE/2),
               QSizeF(HANDLE_SIZE, HANDLE_SIZE))
    };

    // ========== 四条边中间的手柄 ==========
    QVector<QRectF> edgeHandles = {
        // 上边中间：水平居中，垂直紧贴矩形上边
        QRectF(rect.center().x() - HANDLE_SIZE/2, rect.top() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        // 下边中间
        QRectF(rect.center().x() - HANDLE_SIZE/2, rect.bottom() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        // 左边中间：垂直居中，水平紧贴矩形左边
        QRectF(rect.left() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE),
        // 右边中间
        QRectF(rect.right() - HANDLE_SIZE/2, rect.center().y() - HANDLE_SIZE/2,
               HANDLE_SIZE, HANDLE_SIZE)
    };

    // ========== 逐个绘制8个手柄 ==========
    // 每个手柄：白色填充 + 蓝色边框
    for (const QRectF &handle : cornerHandles + edgeHandles) {
        // 第一步：画白色填充
        painter->setPen(Qt::NoPen);     // 无边框
        painter->setBrush(Qt::white);   // 白色填充
        painter->drawRect(handle);

        // 第二步：画蓝色边框（覆盖在白色填充上）
        painter->setPen(handlePen);     // 蓝色实线边框
        painter->setBrush(Qt::NoBrush); // 透明填充
        painter->drawRect(handle);
    }

    painter->restore();  // 恢复绘图状态
}

// ========== 图形项状态变化事件 ==========
QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        // 选中状态改变时重绘（显示/隐藏手柄）
        update();
    }
    return QGraphicsRectItem::itemChange(change, value);
}