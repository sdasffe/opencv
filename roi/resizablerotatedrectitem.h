#ifndef RESIZABLEROTATEDRECTITEM_H
#define RESIZABLEROTATEDRECTITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @brief 可旋转、可缩放的矩形 ROI
 *
 * 局部坐标系：矩形以原点为中心；通过 setPos / setRotation 放到场景中。
 * - 4 角手柄：缩放
 * - 顶部旋转手柄：绕中心旋转
 * - 内部拖拽：移动
 */
class ResizableRotatedRectItem : public QGraphicsObject
{
public:
    enum HandleType {
        None, TopLeft, TopRight, BottomLeft, BottomRight, Rotate, Move
    };

    explicit ResizableRotatedRectItem(qreal width, qreal height,
                                      QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    QRectF localRect() const { return m_rect; }
    void setLocalSize(qreal width, qreal height);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    HandleType getHandleAtPos(const QPointF &pos) const;
    void updateCursor(const QPointF &pos);
    void drawHandles(QPainter *painter) const;
    QPointF handleCenter(HandleType type) const;

    QRectF m_rect; // 以原点为中心的局部矩形
    HandleType m_handleType = None;
    QPointF m_mousePressScenePos;
    QPointF m_mousePressItemPos;
    QRectF m_originalRect;
    qreal m_originalRotation = 0.0;
    bool m_resizing = false;

    static constexpr qreal HANDLE_SIZE = 8.0;
    static constexpr qreal ROTATE_OFFSET = 28.0;
    static constexpr qreal MIN_SIZE = 20.0;
};

#endif // RESIZABLEROTATEDRECTITEM_H
