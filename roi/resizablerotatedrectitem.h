#ifndef RESIZABLEROTATEDRECTITEM_H
#define RESIZABLEROTATEDRECTITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @file resizablerotatedrectitem.h
 * @brief 可旋转矩形 ROI —— 对应 RoiInfo::RotatedRect（center + size + angleDeg）
 *
 * 局部 m_rect 以原点为中心；scene 位置 = center，rotation = angleDeg。
 * Widget::getAllRoiInfo 读 mapToScene(localRect().center()) 与 rotation()。
 *
 * @see roi/roiinfo.h
 */

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

    /** 局部坐标矩形（中心在原点），供 Widget 导出 size / 中心 */
    QRectF localRect() const { return m_rect; }
    /** 改局部宽高并重设 transformOrigin 为矩形中心 */
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
