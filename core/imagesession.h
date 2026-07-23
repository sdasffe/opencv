#ifndef IMAGESESSION_H
#define IMAGESESSION_H

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QHash>
#include <QString>
#include "../roi/roiinfo.h"

/**
 * @brief 单张图片的处理会话（内存结构 + 可 JSON 落盘）
 *
 * 换图时 Widget 按绝对路径保存/恢复：处理链参数 JSON + 多 ROI 几何快照。
 * 键为图片绝对路径，见 Widget::m_sessions 与 app_sessions.json。
 */
struct ImageSession
{
    QJsonArray chain;                                                // 各 BaseBlock::saveParams() 序列；顺序=执行顺序
    QList<RoiInfo> rois;                                             // 场景 ROI 几何快照；空=全图；算法侧做蒙版并集

    /** @brief 序列化为 JSON 对象（blocks + rois 字段） */
    QJsonObject toJson() const
    {
        QJsonObject o;                                               // 单张图会话根对象
        o.insert(QStringLiteral("blocks"), chain);                   // 处理链参数数组
        QJsonArray arr;                                              // ROI 列表 JSON 数组
        for (const RoiInfo &r : rois)
            arr.append(r.toJson());                                  // 逐个 ROI 转 JSON 对象
        o.insert(QStringLiteral("rois"), arr);                       // 写入 rois 键
        return o;
    }

    /** @brief 从 JSON 对象反序列化；缺失字段用空数组/空列表 */
    static ImageSession fromJson(const QJsonObject &o)
    {
        ImageSession s;                                              // 默认空链、空 ROI
        s.chain = o.value(QStringLiteral("blocks")).toArray();       // 读 blocks；无则空数组
        const QJsonArray arr = o.value(QStringLiteral("rois")).toArray(); // 读 rois 数组
        for (const QJsonValue &v : arr)
            s.rois.append(RoiInfo::fromJson(v.toObject()));          // 逐个还原 RoiInfo
        return s;
    }
};

/** @brief 将会话哈希表打包为落盘 JSON（含 version 与 sessions 键） */
inline QJsonObject sessionsToJson(const QHash<QString, ImageSession> &map)
{
    QJsonObject root;                                                // 文件根对象
    root.insert(QStringLiteral("version"), 1);                       // 格式版本；读盘时校验
    QJsonObject sessions;                                            // path → ImageSession JSON
    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        sessions.insert(it.key(), it.value().toJson());              // 键=图片绝对路径
    root.insert(QStringLiteral("sessions"), sessions);               // 写入 sessions 对象
    return root;
}

/** @brief 从落盘 JSON 恢复会话哈希表；缺失 sessions 则返回空 map */
inline QHash<QString, ImageSession> sessionsFromJson(const QJsonObject &root)
{
    QHash<QString, ImageSession> map;                                // 输出：path → 会话
    const QJsonObject sessions = root.value(QStringLiteral("sessions")).toObject(); // 取 sessions 子对象
    for (auto it = sessions.begin(); it != sessions.end(); ++it)
        map.insert(it.key(), ImageSession::fromJson(it.value().toObject())); // 逐条反序列化
    return map;
}

#endif // IMAGESESSION_H
