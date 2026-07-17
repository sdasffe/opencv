#ifndef RESIZABLEELLIPSEITEM_H
#define RESIZABLEELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

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
    enum HandleType {
        None, Top, Bottom, Left, Right, Move
    };

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
