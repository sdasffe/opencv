#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @brief 可调整大小的矩形 ROI
 *
 * 继承自 QGraphicsRectItem，支持：
 * 1. 拖拽内部移动矩形
 * 2. 4角 + 4边共8个手柄缩放
 * 3. 选中时显示手柄，悬停自动切换光标
 */
class ResizableRectItem : public QGraphicsRectItem
{
public:
    enum HandleType {
        None, TopLeft, TopRight, BottomLeft, BottomRight,
        Top, Bottom, Left, Right, Move
    };

    explicit ResizableRectItem(qreal x, qreal y, qreal width, qreal height,
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
    static constexpr qreal HANDLE_MARGIN = 4.0;
};

#endif // RESIZABLERECTITEM_H
