#include "resizableellipseitem.h"
#include <QGraphicsView>             // 图形视图
#include <QStyleOptionGraphicsItem>  // 图形项绘制选项
#include <QGraphicsScene>            // 场景

// ========== 构造函数 ==========
ResizableEllipseItem::ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                           QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent)  // 调用父类构造，设置椭圆外接矩形
    , m_handleType(None)       // 初始没有激活的手柄
    , m_resizing(false)        // 初始不在调整状态
{
    // ========== 设置图形项的交互标志 ==========
    setFlags(QGraphicsItem::ItemIsMovable |           // 允许移动
             QGraphicsItem::ItemIsSelectable |        // 允许选中
             QGraphicsItem::ItemIsFocusable |         // 允许获得焦点
             QGraphicsItem::ItemSendsGeometryChanges); // 几何变化时发送通知（重要！）

    setAcceptHoverEvents(true);  // 启用悬停事件（用于光标跟随变化）

    // ========== 设置椭圆边框样式 ==========
    QPen pen;
    pen.setColor(Qt::blue);      // 蓝色边框
    pen.setWidth(2);             // 线宽2像素
    pen.setStyle(Qt::DashLine);  // 虚线样式
    setPen(pen);
}

// ========== 判断鼠标位置对应的手柄（只有上下左右） ==========
// 按优先级检测：四条边中间 → 椭圆内部 → 无
ResizableEllipseItem::HandleType ResizableEllipseItem::getHandleAtPos(const QPointF &pos)
{
    QRectF rect = this->rect();  // 获取椭圆外接矩形
    const qreal handleSize = HANDLE_SIZE + 4;  // 手柄检测区域比视觉大4像素，降低点击难度

    // ---------- 检测四条边中间手柄 ----------
    // 每个手柄的检测区域是一个以边中点为中心的正方形

    // 上边中间：水平居中，垂直在矩形上方
    QRectF top(rect.center().x() - handleSize, rect.top() - handleSize,
               handleSize * 2, handleSize * 2);
    if (top.contains(pos)) return Top;

    // 下边中间：水平居中，垂直在矩形下方
    QRectF bottom(rect.center().x() - handleSize, rect.bottom() - handleSize,
                  handleSize * 2, handleSize * 2);
    if (bottom.contains(pos)) return Bottom;

    // 左边中间：垂直居中，水平在矩形左侧
    QRectF left(rect.left() - handleSize, rect.center().y() - handleSize,
                handleSize * 2, handleSize * 2);
    if (left.contains(pos)) return Left;

    // 右边中间：垂直居中，水平在矩形右侧
    QRectF right(rect.right() - handleSize, rect.center().y() - handleSize,
                 handleSize * 2, handleSize * 2);
    if (right.contains(pos)) return Right;

    // ---------- 检测是否在椭圆内部（用于移动） ----------
    // 使用椭圆方程判断：点(x,y)满足 (x-x₀)²/a² + (y-y₀)²/b² ≤ 1 则在椭圆内
    if (rect.contains(pos)) {  // 先在外接矩形内（快速排除）
        QPointF center = rect.center();  // 椭圆中心点
        qreal a = rect.width() / 2.0;    // 半长轴（x方向半径）
        qreal b = rect.height() / 2.0;   // 半短轴（y方向半径）
        if (a > 0 && b > 0) {
            // 归一化坐标，代入椭圆方程
            qreal dx = (pos.x() - center.x()) / a;
            qreal dy = (pos.y() - center.y()) / b;
            if (dx*dx + dy*dy <= 1.0) {
                return Move;  // 在椭圆内部，执行移动操作
            }
        }
    }

    // 不在任何可交互区域
    return None;
}

// ========== 根据手柄类型更新光标样式 ==========
void ResizableEllipseItem::updateCursor(const QPointF &pos)
{
    HandleType handle = getHandleAtPos(pos);  // 先判断位置对应的手柄

    switch (handle) {
    case Top:           // 上边 → ↕ 垂直调整光标
    case Bottom:        // 下边 → ↕ 垂直调整光标
        setCursor(Qt::SizeVerCursor);
        break;
    case Left:          // 左边 → ↔ 水平调整光标
    case Right:         // 右边 → ↔ 水平调整光标
        setCursor(Qt::SizeHorCursor);
        break;
    case Move:          // 椭圆内部 → 十字移动光标
        setCursor(Qt::SizeAllCursor);
        break;
    default:            // 椭圆外部 → 普通箭头
        setCursor(Qt::ArrowCursor);
        break;
    }
}

// ========== 鼠标悬停移动事件 ==========
// 鼠标在椭圆上移动时，实时更新光标样式
void ResizableEllipseItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) {  // 调整过程中不改变光标（保持拖拽时的光标）
        updateCursor(event->pos());
    }
    QGraphicsEllipseItem::hoverMoveEvent(event);  // 调用父类处理
}

// ========== 鼠标离开事件 ==========
// 鼠标离开椭圆区域时，恢复默认箭头光标
void ResizableEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_resizing) {  // 调整过程中不改变光标
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

// ========== 鼠标按下事件（开始拖拽） ==========
void ResizableEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {  // 只响应左键
        m_handleType = getHandleAtPos(event->pos());  // 判断按下的位置

        if (m_handleType != None) {  // 按在有效交互区域
            m_resizing = true;                    // 进入调整状态
            m_mousePressPos = event->scenePos();  // 记录按下的场景坐标（用于计算拖拽距离）
            m_originalRect = rect();              // 记录原始外接矩形（用于计算新形状）

            if (m_handleType == Move) {
                // 移动操作：交给父类 QGraphicsItem 的内置移动机制处理
                QGraphicsEllipseItem::mousePressEvent(event);
            }
            return;  // 事件已处理
        }
    }
    QGraphicsEllipseItem::mousePressEvent(event);  // 非有效区域，正常传递
}

// ========== 鼠标移动事件（单独调整宽度或高度） ==========
void ResizableEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 只在调整大小状态且不是移动操作时才自定义处理
    if (m_resizing && m_handleType != None && m_handleType != Move) {
        // 计算鼠标相对按下位置的偏移量
        QPointF delta = event->scenePos() - m_mousePressPos;
        QRectF newRect = m_originalRect;  // 从原始外接矩形出发计算

        // 根据手柄类型，只调整对应的边
        switch (m_handleType) {
        case Top:
            // 拖拽上边 → 只调整上边界（只改变y和高度，宽度不变）
            newRect.setTop(m_originalRect.top() + delta.y());
            break;
        case Bottom:
            // 拖拽下边 → 只调整下边界
            newRect.setBottom(m_originalRect.bottom() + delta.y());
            break;
        case Left:
            // 拖拽左边 → 只调整左边界（只改变x和宽度，高度不变）
            newRect.setLeft(m_originalRect.left() + delta.x());
            break;
        case Right:
            // 拖拽右边 → 只调整右边界
            newRect.setRight(m_originalRect.right() + delta.x());
            break;
        default:
            break;
        }

        // ---------- 限制最小尺寸（防止椭圆太小或反向） ----------
        // 宽度最小20像素
        if (newRect.width() < 20) {
            if (m_handleType == Left)
                newRect.setLeft(newRect.right() - 20);   // 拖拽左边时固定右边
            else
                newRect.setRight(newRect.left() + 20);    // 拖拽右边时固定左边
        }

        // 高度最小20像素
        if (newRect.height() < 20) {
            if (m_handleType == Top)
                newRect.setTop(newRect.bottom() - 20);   // 拖拽上边时固定下边
            else
                newRect.setBottom(newRect.top() + 20);    // 拖拽下边时固定上边
        }

        // 通知场景几何即将变化（重要！避免拖影）
        prepareGeometryChange();
        setRect(newRect);  // 应用新的外接矩形

        // 强制更新场景，消除拖影
        scene()->update(scene()->sceneRect());

        return;  // 事件已处理
    }

    QGraphicsEllipseItem::mouseMoveEvent(event);  // 移动操作交给父类
}

// ========== 鼠标释放事件（结束调整） ==========
void ResizableEllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {  // 之前在调整状态
        m_resizing = false;   // 退出调整状态
        m_handleType = None;  // 清除激活的手柄

        // 强制重绘，确保最终状态正确显示
        update();                               // 重绘自身
        scene()->update(scene()->sceneRect());  // 更新整个场景，清除可能的拖影
    }
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

// ========== 绘制椭圆和手柄 ==========
void ResizableEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget)
{
    // 去掉默认的选中虚线框（QGraphicsItem 默认会在选中时绘制虚线矩形）
    QStyleOptionGraphicsItem opt = *option;         // 拷贝一份绘制选项
    opt.state &= ~QStyle::State_Selected;           // 清除选中状态标志位

    // 用修改后的选项绘制椭圆（不带默认选中框）
    QGraphicsEllipseItem::paint(painter, &opt, widget);

    // 如果被选中或正在调整大小，手动绘制4个手柄
    if (isSelected() || m_resizing) {
        drawHandles(painter);
    }
}

// ========== 绘制上下左右4个手柄 ==========
// 手柄位置示意：
//            ■  ← 上边中间
//   ┌─────────────────┐
//   │                 │
// ■ │      椭圆       │ ■  ← 左/右边中间
//   │                 │
//   └─────────────────┘
//            ■  ← 下边中间
void ResizableEllipseItem::drawHandles(QPainter *painter)
{
    QRectF rect = this->rect();  // 获取椭圆外接矩形
    painter->save();  // 保存绘图状态，便于恢复

    // 关闭抗锯齿，手柄边缘更清晰锐利
    painter->setRenderHint(QPainter::Antialiasing, false);

    // 手柄边框：蓝色实线
    QPen handlePen(Qt::blue, 1, Qt::SolidLine);

    // ========== 4个边中间的手柄位置 ==========
    QVector<QRectF> handles;

    // 上边中间：水平居中于矩形，垂直紧贴矩形上边外侧
    handles.append(QRectF(
        rect.center().x() - HANDLE_SIZE/2,  // x = 矩形中心x - 半个手柄宽
        rect.top() - HANDLE_SIZE/2,         // y = 矩形顶部 - 半个手柄高
        HANDLE_SIZE,                         // 宽度
        HANDLE_SIZE                          // 高度
        ));

    // 下边中间：水平居中，垂直紧贴矩形下边外侧
    handles.append(QRectF(
        rect.center().x() - HANDLE_SIZE/2,
        rect.bottom() - HANDLE_SIZE/2,
        HANDLE_SIZE,
        HANDLE_SIZE
        ));

    // 左边中间：垂直居中，水平紧贴矩形左边外侧
    handles.append(QRectF(
        rect.left() - HANDLE_SIZE/2,
        rect.center().y() - HANDLE_SIZE/2,
        HANDLE_SIZE,
        HANDLE_SIZE
        ));

    // 右边中间：垂直居中，水平紧贴矩形右边外侧
    handles.append(QRectF(
        rect.right() - HANDLE_SIZE/2,
        rect.center().y() - HANDLE_SIZE/2,
        HANDLE_SIZE,
        HANDLE_SIZE
        ));

    // ========== 逐个绘制4个手柄 ==========
    // 每个手柄：白色填充 + 蓝色边框
    for (const QRectF &handle : handles) {
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
QVariant ResizableEllipseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // 当位置改变或选中状态改变时，强制更新整个场景（避免拖影）
    if (change == ItemPositionHasChanged || change == ItemSelectedHasChanged) {
        scene()->update(scene()->sceneRect());
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}