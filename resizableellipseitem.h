#ifndef RESIZABLEELLIPSEITEM_H
#define RESIZABLEELLIPSEITEM_H

#include <QGraphicsEllipseItem>        // Qt 椭圆/圆形图形项基类
#include <QGraphicsSceneMouseEvent>    // 场景鼠标事件（包含坐标信息）
#include <QPainter>                    // 绘图类
#include <QCursor>                     // 光标样式
#include <QPen>                        // 画笔（颜色、线宽、样式）

/**
 * @brief 可调整大小的椭圆/圆形类
 *
 * 继承自 QGraphicsEllipseItem，支持以下功能：
 * 1. 拖拽内部区域移动椭圆
 * 2. 拖拽上下边中间手柄调整高度
 * 3. 拖拽左右边中间手柄调整宽度
 * 4. 选中时在上下左右显示4个调整手柄
 * 5. 悬停时根据位置自动切换光标样式
 *
 * 使用示例：
 * @code
 * // 创建直径100的圆形（宽高相等=正圆）
 * ResizableEllipseItem *circle = new ResizableEllipseItem(50, 50, 100, 100);
 * scene->addItem(circle);
 *
 * // 创建椭圆（宽高不等）
 * ResizableEllipseItem *ellipse = new ResizableEllipseItem(50, 50, 150, 100);
 * scene->addItem(ellipse);
 * @endcode
 */
class ResizableEllipseItem : public QGraphicsEllipseItem
{
public:
    // ========== 手柄类型枚举 ==========
    // 定义鼠标在椭圆上可能的位置，每种位置对应不同的操作
    enum HandleType {
        None,     // 不在任何手柄上（椭圆外部）
        Top,      // 上边中间手柄 → 只调整高度，↑↓方向拖拽
        Bottom,   // 下边中间手柄 → 只调整高度，↑↓方向拖拽
        Left,     // 左边中间手柄 → 只调整宽度，←→方向拖拽
        Right,    // 右边中间手柄 → 只调整宽度，←→方向拖拽
        Move      // 椭圆内部 → 移动整个椭圆
    };

    /**
     * @brief 构造函数
     * @param x      椭圆外接矩形左上角的 x 坐标
     * @param y      椭圆外接矩形左上角的 y 坐标
     * @param width  椭圆外接矩形宽度（正圆时等于直径）
     * @param height 椭圆外接矩形高度（正圆时等于直径）
     * @param parent 父图形项（可选，默认为 nullptr）
     *
     * 注意：Qt 中椭圆/圆形由外接矩形定义，width==height 时为正圆
     */
    explicit ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                  QGraphicsItem *parent = nullptr);

protected:
    // ========== Qt 事件重写 ==========

    /**
     * @brief 绘制椭圆和调整手柄
     * 先绘制椭圆本身，选中时额外绘制4个手柄（上下左右）
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /**
     * @brief 鼠标按下事件
     * 记录按下位置和原始外接矩形，判断是移动还是缩放操作
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
     * 实时更新椭圆外接矩形的大小或位置
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件
     * 结束调整，恢复默认状态
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标悬停移动事件
     * 根据鼠标位置动态切换光标样式
     * - 上/下边 → 垂直双向箭头
     * - 左/右边 → 水平双向箭头
     * - 椭圆内部 → 十字移动箭头
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 鼠标离开事件
     * 鼠标离开椭圆时恢复默认箭头光标
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 图形项状态变化事件
     * 当选中状态或位置改变时，强制更新场景避免拖影
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    // ========== 私有辅助方法 ==========

    /**
     * @brief 根据鼠标位置判断当前激活的手柄类型
     * @param pos 鼠标在图形项本地坐标系中的位置
     * @return 对应的手柄类型枚举值
     *
     * 检测优先级：四条边 → 椭圆内部 → 无
     * 椭圆内部使用椭圆方程判断：(x-x0)²/a² + (y-y0)²/b² <= 1
     */
    HandleType getHandleAtPos(const QPointF &pos);

    /**
     * @brief 根据手柄类型更新光标样式
     * @param pos 鼠标位置（用于判断手柄类型）
     *
     * 映射关系：
     * - 上/下边 → SizeVerCursor（↕ 垂直调整）
     * - 左/右边 → SizeHorCursor（↔ 水平调整）
     * - 椭圆内部 → SizeAllCursor（十字移动箭头）
     * - 椭圆外部 → ArrowCursor（普通箭头）
     */
    void updateCursor(const QPointF &pos);

    /**
     * @brief 绘制4个调整手柄（上下左右）
     * @param painter 绘图对象
     *
     * 手柄位置：
     *        ■  ← 上边中间
     *   ┌─────────┐
     * ■ │  椭圆   │ ■  ← 左/右边中间
     *   └─────────┘
     *        ■  ← 下边中间
     *
     * 样式：白色填充 + 蓝色边框的小正方形
     */
    void drawHandles(QPainter *painter);

    // ========== 私有成员变量 ==========

    HandleType m_handleType;   // 当前激活的手柄类型（鼠标按下时确定）
    QPointF m_mousePressPos;   // 鼠标按下时的场景坐标（用于计算拖拽距离）
    QRectF m_originalRect;     // 鼠标按下时椭圆外接矩形的原始形状（用于计算新形状）
    bool m_resizing;           // 是否正在调整大小（true=缩放中，false=移动或空闲）

    // ========== 常量 ==========
    static constexpr qreal HANDLE_SIZE = 8.0;  // 手柄显示大小（像素）
    // 注意：没有 HANDLE_MARGIN，检测容差直接内嵌在 getHandleAtPos 中（HANDLE_SIZE + 4）
};

#endif // RESIZABLEELLIPSEITEM_H