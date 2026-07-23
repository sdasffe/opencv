#ifndef RESIZABLEELLIPSEITEM_H
#define RESIZABLEELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @file resizableellipseitem.h
 * @brief 可调整大小的椭圆 ROI —— 与 ResizableRectItem 同模式的多 ROI 图元
 *
 * 几何写入 RoiInfo::rect（外接矩形）；mask 由 RoiProcess 按椭圆方程生成。
 * 交互：内部拖拽移动、四边手柄单轴缩放；命中检测用椭圆方程区分内外。
 *
 * @see roi/roiinfo.h
 * @see roi/resizablerectitem.h  轴对齐矩形参考实现（文档更完整）
 */

/**
 * @brief 可调整大小的椭圆/圆形 ROI
 *
 * 继承自 QGraphicsEllipseItem，支持：
 * 1. 拖拽内部移动椭圆
 * 2. 上下左右4个手柄分别调整宽高
 * 3. 选中时显示手柄，悬停自动切换光标
 */
class ResizableEllipseItem : public QGraphicsEllipseItem
{
public:
    /** 四边缩放手柄 + 内部 Move；无角手柄（椭圆仅调宽高） */
    enum HandleType {
        None, Top, Bottom, Left, Right, Move
    };

    /** @param x,y,w,h  item 本地坐标系外接矩形（与 ResizableRectItem 一致） */
    explicit ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                  QGraphicsItem *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    HandleType getHandleAtPos(const QPointF &pos);
    void updateCursor(const QPointF &pos);
    void drawHandles(QPainter *painter);

    HandleType m_handleType;
    QPointF m_mousePressPos;
    QRectF m_originalRect;
    bool m_resizing;

    static constexpr qreal HANDLE_SIZE = 8.0;
};

#endif // RESIZABLEELLIPSEITEM_H
