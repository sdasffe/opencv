#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @file resizablerectitem.h
 * @brief 可调整大小的矩形 ROI 图元 —— QGraphicsView 上的交互范例
 *
 * 【在整条链路中的位置】
 *   用户在 QGraphicsView 上绘制/调整 ROI
 *   → ResizableRectItem 提供矩形几何与拖拽交互
 *   → Widget 读取 rect()/pos() 等写入 RoiInfo
 *   → ImageProcessor 把 RoiInfo 传给各 Block::process
 *   → RoiProcess::makeMask 按同一几何生成 OpenCV mask
 *
 * 【学习本文件可掌握】
 *   1. QGraphicsItem 坐标系：item 本地坐标 vs scene 坐标
 *   2. 鼠标事件链：hover → press → move → release
 *   3. 手柄命中检测：把矩形边/角扩成可点击热区
 *   4. 缩放 vs 移动：移动可委托基类 ItemIsMovable，缩放需自实现 setRect
 *   5. itemChange / prepareGeometryChange：几何变化时通知 Scene 重绘
 *
 * 【与 ResizableRotatedRectItem 等的关系】
 *   本类是最简单的轴对齐矩形 ROI；旋转矩形需额外处理 angle 与四角旋转
 *
 * @see roi/roiinfo.h          ROI 数据结构
 * @see utils/roiprocess.cpp   mask 生成（与 rect 几何对应）
 */

/**
 * @brief 可调整大小的矩形 ROI
 *
 * 继承自 QGraphicsRectItem，在标准矩形绘制之上增加：
 *   1. 拖拽内部 → 移动整个矩形（复用 ItemIsMovable）
 *   2. 拖拽 8 个手柄（4 角 + 4 边）→ 缩放
 *   3. 选中或缩放过程中绘制蓝色手柄
 *   4. 悬停时根据手柄类型切换光标（↔ ↕ 等）
 */
class ResizableRectItem : public QGraphicsRectItem
{
public:
    /**
     * @brief 鼠标命中区域类型
     *
     * None：矩形外或未识别
     * TopLeft..Right：八个缩放手柄
     * Move：矩形内部（整体平移）
     */
    enum HandleType {
        None, TopLeft, TopRight, BottomLeft, BottomRight,
        Top, Bottom, Left, Right, Move
    };

    /**
     * @brief 构造 ROI 矩形
     *
     * @param x,y,width,height  item 本地坐标系下的矩形（通常父项为 scene 或 pixmap 容器）
     * @param parent            父 QGraphicsItem，变换会继承
     *
     * 谁调用：Widget 在用户选择「矩形 ROI」并拖拽创建时
     */
    explicit ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                               QGraphicsItem *parent = nullptr);

protected:
    /** 绘制矩形虚线边框 + 选中时的 8 手柄 */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    /** 左键按下：记录命中手柄、起始 scene 坐标、原始 rect */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    /** 拖动：根据手柄类型更新 rect；Move 交给基类处理 pos */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    /** 释放：结束缩放、恢复缓存、刷新 scene */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    /** 悬停移动：更新 resize 光标 */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    /** 悬停离开：恢复箭头光标 */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    /** 选中状态变化时 update() 以显示/隐藏手柄 */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    /** @brief 判断本地坐标 pos 落在哪个手柄/内部/外部 */
    HandleType getHandleAtPos(const QPointF &pos);
    /** @brief 根据命中类型设置 Qt 标准 resize 光标 */
    void updateCursor(const QPointF &pos);
    /** @brief 在矩形角点与中点绘制 8 个白色填充小方块 */
    void drawHandles(QPainter *painter);

    HandleType m_handleType;   ///< 当前按下/拖动的手柄类型
    QPointF m_mousePressPos;   ///< 按下时鼠标在 scene 中的位置（缩放时用 scene 增量）
    QRectF m_originalRect;     ///< 按下瞬间的 rect，缩放基准
    bool m_resizing;           ///< true 表示正在拖手柄缩放（非 Move）

    static constexpr qreal HANDLE_SIZE = 8.0;   ///< 手柄绘制边长（像素，item 坐标）
    static constexpr qreal HANDLE_MARGIN = 4.0; ///< 预留：可扩展热区与绘制尺寸差
};

#endif // RESIZABLERECTITEM_H
