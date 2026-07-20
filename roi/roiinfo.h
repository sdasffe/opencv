#ifndef ROIINFO_H
#define ROIINFO_H

#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QtMath>

/**
 * @brief 统一 ROI 描述（图像像素坐标系，与场景坐标一致）
 *
 * shape == None 表示全图处理。
 *
 * 谁产生：Widget::getCurrentRoiInfo() 从场景图元读取。
 * 谁消费：ImageProcessor::setRoi → 各 Block::process → RoiProcess::makeMask/apply。
 *
 * 三种形状用不同字段：
 *   - Rect / Ellipse → rect（轴对齐外接矩形）
 *   - RotatedRect    → center + size + angleDeg
 */
struct RoiInfo
{
    enum class Shape {
        None,
        Rect,
        Ellipse,
        RotatedRect
    };

    Shape shape = Shape::None;

    /** 轴对齐矩形 / 椭圆外接矩形（图像坐标） */
    QRectF rect;

    /** 旋转矩形：中心、宽高、角度（度，逆时针，与 Qt rotation 一致） */
    QPointF center;
    QSizeF size;
    qreal angleDeg = 0.0;

    bool isEmpty() const { return shape == Shape::None; }

    /** 外包络矩形，便于裁剪到图像范围 */
    QRectF boundingRect() const
    {
        switch (shape) {
        case Shape::Rect:
        case Shape::Ellipse:
            return rect.normalized();
        case Shape::RotatedRect: {
            const qreal rad = qDegreesToRadians(angleDeg);
            const qreal c = qCos(rad);
            const qreal s = qSin(rad);
            const qreal hw = size.width() / 2.0;
            const qreal hh = size.height() / 2.0;
            const QPointF corners[4] = {
                QPointF(-hw, -hh), QPointF(hw, -hh),
                QPointF(hw, hh), QPointF(-hw, hh)
            };
            qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
            for (const QPointF &p : corners) {
                const QPointF r(p.x() * c - p.y() * s, p.x() * s + p.y() * c);
                const QPointF w = center + r;
                minX = qMin(minX, w.x());
                minY = qMin(minY, w.y());
                maxX = qMax(maxX, w.x());
                maxY = qMax(maxY, w.y());
            }
            return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
        }
        default:
            return QRectF();
        }
    }
};

#endif // ROIINFO_H
