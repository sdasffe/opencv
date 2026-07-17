#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>          // Qt 矩形图形项基类
#include <QGraphicsSceneMouseEvent>   // 场景鼠标事件（包含坐标信息）
#include <QPainter>                   // 绘图类
#include <QCursor>                    // 光标样式
#include <QPen>                       // 画笔（颜色、线宽、样式）

/**
 * @brief 可调整大小的矩形类
 *
 * 继承自 QGraphicsRectItem，支持以下功能：
 * 1. 拖拽内部区域移动矩形
 * 2. 拖拽4个角调整大小（同时改变宽高）
 * 3. 拖拽4条边中间调整大小（单独改变宽或高）
 * 4. 选中时显示8个调整手柄
 * 5. 悬停时根据位置自动切换光标样式
 */
class ResizableRectItem : public QGraphicsRectItem
{
public:
    // ========== 手柄类型枚举 ==========
    // 定义鼠标在矩形上可能的位置，每种位置对应不同的操作
    enum HandleType {
        None,         // 不在任何手柄上（矩形外部）
        TopLeft,      // 左上角手柄 → 同时调整宽高，↖方向
        TopRight,     // 右上角手柄 → 同时调整宽高，↗方向
        BottomLeft,   // 左下角手柄 → 同时调整宽高，↙方向
        BottomRight,  // 右下角手柄 → 同时调整宽高，↘方向
        Top,          // 上边中间手柄 → 只调整高度，↑方向
        Bottom,       // 下边中间手柄 → 只调整高度，↓方向
        Left,         // 左边中间手柄 → 只调整宽度，←方向
        Right,        // 右边中间手柄 → 只调整宽度，→方向
        Move          // 矩形内部 → 移动整个矩形
    };

    /**
     * @brief 构造函数
     * @param x      矩形左上角的 x 坐标
     * @param y      矩形左上角的 y 坐标
     * @param width  矩形宽度
     * @param height 矩形高度
     * @param parent 父图形项（可选，默认为 nullptr）
     */
    explicit ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                               QGraphicsItem *parent = nullptr);

protected:
    // ========== Qt 事件重写 ==========

    /**
     * @brief 绘制矩形和调整手柄
     * 先绘制矩形本身，选中时额外绘制8个手柄
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /**
     * @brief 鼠标按下事件
     * 记录按下位置和原始矩形，判断是移动还是缩放操作
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
     * 实时更新矩形的大小或位置
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件
     * 结束调整，恢复默认状态
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标悬停移动事件
     * 根据鼠标位置动态切换光标样式（如双向箭头、四向箭头等）
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 鼠标离开事件
     * 鼠标离开矩形时恢复默认箭头光标
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 图形项状态变化事件
     * 当选中状态或位置改变时，强制更新场景避免拖影
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

signals:  // 添加信号
    void rectChanged();  // 矩形位置或大小改变时发出

private:
    // ========== 私有辅助方法 ==========

    /**
     * @brief 根据鼠标位置判断当前激活的手柄类型
     * @param pos 鼠标在图形项本地坐标系中的位置
     * @return 对应的手柄类型枚举值
     */
    HandleType getHandleAtPos(const QPointF &pos);

    /**
     * @brief 根据手柄类型更新光标样式
     * @param pos 鼠标位置（用于判断手柄类型）
     *
     * 映射关系：
     * - 左上/右下角 → SizeFDiagCursor（↖↘）
     * - 右上/左下角 → SizeBDiagCursor（↗↙）
     * - 上/下边     → SizeVerCursor（↕）
     * - 左/右边     → SizeHorCursor（↔）
     * - 矩形内部    → SizeAllCursor（十字箭头）
     * - 矩形外部    → ArrowCursor（普通箭头）
     */
    void updateCursor(const QPointF &pos);

    /**
     * @brief 绘制8个调整手柄
     * @param painter 绘图对象
     *
     * 绘制位置：
     * - 4个角：矩形四个顶点外侧
     * - 4条边：每条边的中点外侧
     * 样式：白色填充 + 蓝色边框的小正方形
     */
    void drawHandles(QPainter *painter);

    // ========== 私有成员变量 ==========

    HandleType m_handleType;   // 当前激活的手柄类型（鼠标按下时确定）
    QPointF m_mousePressPos;   // 鼠标按下时的场景坐标（用于计算拖拽距离）
    QRectF m_originalRect;     // 鼠标按下时矩形的原始形状（用于计算新形状）
    bool m_resizing;           // 是否正在调整大小（true=缩放中，false=移动或空闲）

    // ========== 常量 ==========
    static constexpr qreal HANDLE_SIZE = 8.0;   // 手柄显示大小（像素）
    static constexpr qreal HANDLE_MARGIN = 4.0;  // 手柄检测容差（扩大点击区域）
};

#endif // RESIZABLERECTITEM_H