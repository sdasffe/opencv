#ifndef ROIINFO_H
#define ROIINFO_H

#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QJsonObject>
#include <QtMath>

/**
 * @brief 统一 ROI 描述（图像像素坐标系，与场景坐标一致）
 *
 * shape == None 表示无效项；空列表表示全图处理。
 *
 * 谁产生：Widget::getAllRoiInfo() 从场景图元读取。
 * 谁消费：ImageProcessor::setRois → 各 Block::process → RoiProcess::makeMask/apply。
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

    bool operator==(const RoiInfo &o) const
    {
        if (shape != o.shape)
            return false;
        switch (shape) {
        case Shape::None:
            return true;
        case Shape::Rect:
        case Shape::Ellipse:
            return rect == o.rect;
        case Shape::RotatedRect:
            return center == o.center && size == o.size
                && qFuzzyCompare(angleDeg + 1.0, o.angleDeg + 1.0);
        }
        return false;
    }
    bool operator!=(const RoiInfo &o) const { return !(*this == o); }

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

    /** 序列化为 JSON，供会话落盘 / 处理链导出（字段见各 shape 分支） */
    QJsonObject toJson() const
    {
        QJsonObject o;
        switch (shape) {
        case Shape::Rect:
            o.insert(QStringLiteral("shape"), QStringLiteral("rect"));
            o.insert(QStringLiteral("x"), rect.x());
            o.insert(QStringLiteral("y"), rect.y());
            o.insert(QStringLiteral("w"), rect.width());
            o.insert(QStringLiteral("h"), rect.height());
            break;
        case Shape::Ellipse:
            o.insert(QStringLiteral("shape"), QStringLiteral("ellipse"));
            o.insert(QStringLiteral("x"), rect.x());
            o.insert(QStringLiteral("y"), rect.y());
            o.insert(QStringLiteral("w"), rect.width());
            o.insert(QStringLiteral("h"), rect.height());
            break;
        case Shape::RotatedRect:
            o.insert(QStringLiteral("shape"), QStringLiteral("rotatedRect"));
            o.insert(QStringLiteral("cx"), center.x());
            o.insert(QStringLiteral("cy"), center.y());
            o.insert(QStringLiteral("w"), size.width());
            o.insert(QStringLiteral("h"), size.height());
            o.insert(QStringLiteral("angle"), angleDeg);
            break;
        default:
            o.insert(QStringLiteral("shape"), QStringLiteral("none"));
            break;
        }
        return o;
    }

    /** 从 JSON 反序列化；未知 shape 或 "none" 时返回无效项（shape == None） */
    static RoiInfo fromJson(const QJsonObject &o)
    {
        RoiInfo info;
        const QString shape = o.value(QStringLiteral("shape")).toString();
        if (shape == QLatin1String("rect")) {
            info.shape = Shape::Rect;
            info.rect = QRectF(o.value(QStringLiteral("x")).toDouble(),
                               o.value(QStringLiteral("y")).toDouble(),
                               o.value(QStringLiteral("w")).toDouble(),
                               o.value(QStringLiteral("h")).toDouble());
        } else if (shape == QLatin1String("ellipse")) {
            info.shape = Shape::Ellipse;
            info.rect = QRectF(o.value(QStringLiteral("x")).toDouble(),
                               o.value(QStringLiteral("y")).toDouble(),
                               o.value(QStringLiteral("w")).toDouble(),
                               o.value(QStringLiteral("h")).toDouble());
        } else if (shape == QLatin1String("rotatedRect")) {
            info.shape = Shape::RotatedRect;
            info.center = QPointF(o.value(QStringLiteral("cx")).toDouble(),
                                  o.value(QStringLiteral("cy")).toDouble());
            info.size = QSizeF(o.value(QStringLiteral("w")).toDouble(),
                               o.value(QStringLiteral("h")).toDouble());
            info.angleDeg = o.value(QStringLiteral("angle")).toDouble();
        }
        return info;
    }
};

#endif // ROIINFO_H
