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
 * 产生：Widget::getAllRoiInfo()；消费：ImageProcessor::setRois → Block → RoiProcess。
 * Rect/Ellipse 用 rect；RotatedRect 用 center + size + angleDeg。
 */
struct RoiInfo
{
    enum class Shape {
        None,                                                          // 无效 / 占位
        Rect,                                                          // 轴对齐矩形
        Ellipse,                                                       // 椭圆（存外接矩形）
        RotatedRect                                                    // 旋转矩形
    };

    Shape shape = Shape::None;                                         // 当前形状类型

    QRectF rect;                                                       // Rect/Ellipse：轴对齐外接矩形（图像坐标）

    QPointF center;                                                    // RotatedRect：中心点
    QSizeF size;                                                       // RotatedRect：宽高
    qreal angleDeg = 0.0;                                              // RotatedRect：角度（度，逆时针，同 Qt）

    bool isEmpty() const { return shape == Shape::None; }              // 是否无效项

    bool operator==(const RoiInfo &o) const                            // 形状与几何全等（角度模糊比较）
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
                && qFuzzyCompare(angleDeg + 1.0, o.angleDeg + 1.0);    // 避免 -0/0 浮点问题
        }
        return false;
    }
    bool operator!=(const RoiInfo &o) const { return !(*this == o); }

    /** @brief 外包络轴对齐矩形，便于裁剪到图像范围 */
    QRectF boundingRect() const
    {
        switch (shape) {
        case Shape::Rect:
        case Shape::Ellipse:
            return rect.normalized();                                  // 直接用外接矩形
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
                const QPointF r(p.x() * c - p.y() * s, p.x() * s + p.y() * c); // 旋转
                const QPointF w = center + r;                          // 平移到世界坐标
                minX = qMin(minX, w.x());
                minY = qMin(minY, w.y());
                maxX = qMax(maxX, w.x());
                maxY = qMax(maxY, w.y());
            }
            return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));   // AABB
        }
        default:
            return QRectF();
        }
    }

    /** @brief 序列化为 JSON，供会话落盘 / 处理链导出 */
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

    /** @brief 从 JSON 反序列化；未知 shape 返回 None */
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
