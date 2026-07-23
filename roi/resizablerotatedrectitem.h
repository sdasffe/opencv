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
 */

class ResizableRotatedRectItem : public QGraphicsObject
{
public:
    /** 四角缩放 + 顶部旋转 + 内部平移 */
    enum HandleType {
        None,                                                          // 未命中
        TopLeft, TopRight, BottomLeft, BottomRight,                    // 四角缩放
        Rotate,                                                        // 顶部旋转手柄
        Move                                                           // 矩形内部平移
    };

    /** @param width,height 初始局部宽高（会 ≥ MIN_SIZE） */
    explicit ResizableRotatedRectItem(qreal width, qreal height,
                                      QGraphicsItem *parent = nullptr); // 创建可旋转矩形图元

    QRectF boundingRect() const override;                              // 含手柄外扩的包围盒
    QPainterPath shape() const override;                               // 精确命中路径
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;                              // 画虚线矩形与手柄

    QRectF localRect() const { return m_rect; }                        // 局部居中矩形，供导出 size
    void setLocalSize(qreal width, qreal height);                      // 改宽高并重设旋转原点

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;    // 记录手柄与原始几何
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;     // 旋转或对角缩放
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;  // 松手后重新居中 m_rect
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;     // 悬停换光标
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;    // 离开恢复箭头
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override; // 选中刷新

private:
    HandleType getHandleAtPos(const QPointF &pos) const;               // 命中：旋转→角→内部
    void updateCursor(const QPointF &pos);                             // 按手柄设光标
    void drawHandles(QPainter *painter) const;                         // 四角方块 + 旋转圆点
    QPointF handleCenter(HandleType type) const;                       // 各手柄局部中心点

    QRectF m_rect;                                                     // 以原点为中心的局部矩形
    HandleType m_handleType = None;                                    // 当前拖动模式
    QPointF m_mousePressScenePos;                                      // 按下时场景坐标
    QPointF m_mousePressItemPos;                                       // 按下时局部坐标
    QRectF m_originalRect;                                             // 按下时局部矩形快照
    qreal m_originalRotation = 0.0;                                    // 按下时旋转角（度）
    bool m_resizing = false;                                           // 是否正在拖动手柄

    static constexpr qreal HANDLE_SIZE = 8.0;                          // 缩放手柄边长
    static constexpr qreal ROTATE_OFFSET = 28.0;                       // 旋转点相对顶边的偏移
    static constexpr qreal MIN_SIZE = 20.0;                            // 最小宽/高
};

#endif // RESIZABLEROTATEDRECTITEM_H
