#ifndef RESIZABLEELLIPSEITEM_H
#define RESIZABLEELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>

/**
 * @file resizableellipseitem.h
 * @brief 可调大小的椭圆 ROI；几何写入 RoiInfo::rect（外接矩形）
 *
 * 交互：内部拖移、四边手柄调宽高；命中用椭圆方程。坐标系=图像像素。
 */

class ResizableEllipseItem : public QGraphicsEllipseItem
{
public:
    /** 四边缩放手柄 + 内部 Move（无角手柄） */
    enum HandleType {
        None,                                                          // 未命中手柄
        Top, Bottom, Left, Right,                                      // 四边中点手柄
        Move                                                           // 椭圆内部：平移
    };

    /** @param x,y,w,h item 本地外接矩形 */
    explicit ResizableEllipseItem(qreal x, qreal y, qreal width, qreal height,
                                  QGraphicsItem *parent = nullptr);    // 创建可选中、可悬停的椭圆

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;                              // 画椭圆轮廓；选中时画手柄
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;    // 记录按下点与当前 rect
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;     // 按手柄类型缩放或平移
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;  // 结束拖动，清 m_resizing
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;     // 悬停换光标
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;    // 离开恢复箭头
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override; // 选中态刷新手柄

private:
    HandleType getHandleAtPos(const QPointF &pos);                     // 命中测试：手柄优先，再判椭圆内
    void updateCursor(const QPointF &pos);                             // 按命中类型设 Size*/SizeAll
    void drawHandles(QPainter *painter);                               // 绘制四边小方块手柄

    HandleType m_handleType;                                           // 当前拖动模式
    QPointF m_mousePressPos;                                           // 按下时场景/本地坐标
    QRectF m_originalRect;                                             // 按下时的外接矩形快照
    bool m_resizing;                                                   // 是否正在拖动手柄

    static constexpr qreal HANDLE_SIZE = 8.0;                          // 手柄视觉边长（像素）
};

#endif // RESIZABLEELLIPSEITEM_H
