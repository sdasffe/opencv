#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @file resizablerectitem.h
 * @brief 可调大小的轴对齐矩形 ROI；几何写入 RoiInfo::rect
 *
 * 交互：内部拖移、8 手柄缩放。Widget → RoiInfo → RoiProcess::makeMask。
 */

class ResizableRectItem : public QGraphicsRectItem
{
public:
    /** 四角 + 四边缩放手柄 + 内部 Move */
    enum HandleType {
        None,                                                          // 未命中
        TopLeft, TopRight, BottomLeft, BottomRight,                    // 四角对角缩放
        Top, Bottom, Left, Right,                                      // 四边单向缩放
        Move                                                           // 矩形内部：平移
    };

    /** @param x,y,w,h item 本地矩形 */
    explicit ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
                               QGraphicsItem *parent = nullptr);       // 创建可选中、可悬停的虚线红矩形

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;                              // 画矩形；选中时画 8 手柄
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;    // 记录手柄与原始 rect
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;     // 按手柄缩放或基类平移
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;  // 结束缩放，恢复缓存
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;     // 悬停换光标
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;    // 离开恢复箭头
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override; // 选中刷新手柄

private:
    HandleType getHandleAtPos(const QPointF &pos);                     // 命中：角→边→内部
    void updateCursor(const QPointF &pos);                             // 按命中设 Size* 光标
    void drawHandles(QPainter *painter);                               // 绘制 8 个白底蓝边小方块

    HandleType m_handleType;                                           // 当前拖动模式
    QPointF m_mousePressPos;                                           // 按下时 scene 坐标
    QRectF m_originalRect;                                             // 按下时的 rect 快照
    bool m_resizing;                                                   // 是否正在拖动手柄

    static constexpr qreal HANDLE_SIZE = 8.0;                          // 手柄视觉边长
    static constexpr qreal HANDLE_MARGIN = 4.0;                        // 热区相对视觉的外扩余量
};

#endif // RESIZABLERECTITEM_H
