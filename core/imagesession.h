#ifndef IMAGESESSION_H
#define IMAGESESSION_H

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QHash>
#include <QString>
#include "../roi/roiinfo.h"

/**
 * @brief 单张图片的处理会话（内存 + 可落盘）
 *
 * 换图时 Widget 保存/恢复：处理链 JSON + 多 ROI 几何。
 * 键为图片绝对路径，见 Widget::m_sessions。
 */
struct ImageSession
{
    QJsonArray chain;      ///< 各 BaseBlock::saveParams() 序列
    QList<RoiInfo> rois;   ///< 场景 ROI 几何快照（多 ROI 并集参与算法）

    /** @brief 序列化为 JSON 对象（blocks + rois 字段） */
    QJsonObject toJson() const
    {
        QJsonObject o;
        o.insert(QStringLiteral("blocks"), chain);
        QJsonArray arr;
        for (const RoiInfo &r : rois)
            arr.append(r.toJson());
        o.insert(QStringLiteral("rois"), arr);
        return o;
    }

    /** @brief 从 JSON 对象反序列化；缺失字段用空数组/列表 */
    static ImageSession fromJson(const QJsonObject &o)
    {
        ImageSession s;
        s.chain = o.value(QStringLiteral("blocks")).toArray();
        const QJsonArray arr = o.value(QStringLiteral("rois")).toArray();
        for (const QJsonValue &v : arr)
            s.rois.append(RoiInfo::fromJson(v.toObject()));
        return s;
    }
};

/** @brief 将会话哈希表打包为落盘 JSON（含 version 与 sessions 键） */
inline QJsonObject sessionsToJson(const QHash<QString, ImageSession> &map)
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    QJsonObject sessions;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        sessions.insert(it.key(), it.value().toJson());
    root.insert(QStringLiteral("sessions"), sessions);
    return root;
}

/** @brief 从落盘 JSON 恢复会话哈希表 */
inline QHash<QString, ImageSession> sessionsFromJson(const QJsonObject &root)
{
    QHash<QString, ImageSession> map;
    const QJsonObject sessions = root.value(QStringLiteral("sessions")).toObject();
    for (auto it = sessions.begin(); it != sessions.end(); ++it)
        map.insert(it.key(), ImageSession::fromJson(it.value().toObject()));
    return map;
}

#endif // IMAGESESSION_H
