/**
 * @file applogger.cpp
 * @brief AppLogger 实现 —— 线程安全追加写 UTF-8 文本日志
 *
 * 格式：hh:mm:ss.zzz | LEVEL | 事件 | [细节]
 */

#include "applogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QStringConverter>

namespace {

QMutex g_mutex;       // 保护 g_logPath / g_inited 及 QFile 写入的互斥锁
QString g_logPath;    // 当前日志文件绝对路径
bool g_inited = false; // 是否已完成 init()

/** @brief 将日志级别枚举转换为固定宽度文本标签 */
QString levelTag(AppLogger::Level level)
{
    switch (level) {
    case AppLogger::Level::Warn:  return QStringLiteral("WARN");  // 警告级别
    case AppLogger::Level::Error: return QStringLiteral("ERROR"); // 错误级别
    default:                      return QStringLiteral("INFO");  // 默认信息级别
    }
}

} // namespace

/** @brief 初始化日志系统：创建目录、确定当日日志路径并写入会话头 */
void AppLogger::init()
{
    QMutexLocker lock(&g_mutex); // 加锁，防止并发重复初始化
    if (g_inited)
        return; // 已初始化则直接返回

    const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/logs"); // 日志目录与 exe 同目录
    QDir().mkpath(dir); // 确保 logs/ 目录存在
    const QString name = QStringLiteral("app_%1.log")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"))); // 按日期命名
    g_logPath = dir + QLatin1Char('/') + name; // 拼接完整日志路径
    g_inited = true; // 标记初始化完成

    QFile f(g_logPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return; // 无法打开文件则静默失败

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8); // 统一 UTF-8 编码
    out << QStringLiteral("========== 会话开始 %1 ==========\n")
               .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"))); // 会话起始时间
    out << QStringLiteral("INFO | 日志文件 | %1\n").arg(QDir::toNativeSeparators(g_logPath)); // 记录日志路径
    out << QStringLiteral("INFO | 应用 | %1 %2\n")
               .arg(QCoreApplication::applicationName(),
                    QCoreApplication::applicationVersion()); // 记录应用名与版本
    out.flush(); // 立即落盘
}

/** @brief 返回当前日志文件的绝对路径 */
QString AppLogger::currentLogPath()
{
    QMutexLocker lock(&g_mutex); // 加锁读取共享路径
    return g_logPath;
}

/** @brief 写入 Info 级别日志 */
void AppLogger::info(const QString &event, const QString &detail)
{
    log(Level::Info, event, detail); // 委托统一入口
}

/** @brief 写入 Warn 级别日志 */
void AppLogger::warn(const QString &event, const QString &detail)
{
    log(Level::Warn, event, detail); // 委托统一入口
}

/** @brief 写入 Error 级别日志 */
void AppLogger::error(const QString &event, const QString &detail)
{
    log(Level::Error, event, detail); // 委托统一入口
}

/** @brief 统一写日志：格式化时间戳、级别、事件与可选细节后追加到文件 */
void AppLogger::log(Level level, const QString &event, const QString &detail)
{
    if (!g_inited)
        init(); // 允许首次写日志时自动初始化

    QMutexLocker lock(&g_mutex); // 串行化写文件，避免多线程行交错
    if (g_logPath.isEmpty())
        return; // 路径无效则放弃写入

    QFile f(g_logPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return; // 打开失败则放弃写入

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8); // UTF-8 输出

    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz")); // 当前时间戳
    if (detail.isEmpty())
        out << QStringLiteral("%1 | %2 | %3\n").arg(ts, levelTag(level), event); // 无细节行
    else
        out << QStringLiteral("%1 | %2 | %3 | %4\n").arg(ts, levelTag(level), event, detail); // 含细节行
    out.flush(); // 立即落盘
}
