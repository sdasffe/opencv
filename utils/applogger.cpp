/**
 * @file applogger.cpp
 * @brief AppLogger 实现 —— 线程安全追加写 UTF-8 文本日志
 *
 * 格式：hh:mm:ss.zzz | LEVEL | 事件 | [细节]
 * 多线程下通过全局互斥锁串行化写文件，避免行交错。
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

QMutex g_mutex;       ///< 保护 g_logPath / g_inited 及 QFile 写入
QString g_logPath;
bool g_inited = false;

/** 日志级别 → 固定宽度标签 */
QString levelTag(AppLogger::Level level)
{
    switch (level) {
    case AppLogger::Level::Warn:  return QStringLiteral("WARN");
    case AppLogger::Level::Error: return QStringLiteral("ERROR");
    default:                      return QStringLiteral("INFO");
    }
}

} // namespace

void AppLogger::init()
{
    QMutexLocker lock(&g_mutex);
    if (g_inited)
        return;

    // 日志与 exe 同目录，便于打包后用户直接找到
    const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/logs");
    QDir().mkpath(dir);
    const QString name = QStringLiteral("app_%1.log")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd")));
    g_logPath = dir + QLatin1Char('/') + name;
    g_inited = true;

    QFile f(g_logPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << QStringLiteral("========== 会话开始 %1 ==========\n")
               .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")));
    out << QStringLiteral("INFO | 日志文件 | %1\n").arg(QDir::toNativeSeparators(g_logPath));
    out << QStringLiteral("INFO | 应用 | %1 %2\n")
               .arg(QCoreApplication::applicationName(),
                    QCoreApplication::applicationVersion());
    out.flush();
}

QString AppLogger::currentLogPath()
{
    QMutexLocker lock(&g_mutex);
    return g_logPath;
}

void AppLogger::info(const QString &event, const QString &detail)
{
    log(Level::Info, event, detail);
}

void AppLogger::warn(const QString &event, const QString &detail)
{
    log(Level::Warn, event, detail);
}

void AppLogger::error(const QString &event, const QString &detail)
{
    log(Level::Error, event, detail);
}

void AppLogger::log(Level level, const QString &event, const QString &detail)
{
    // 允许在 main 未显式 init 时首次写日志自动初始化
    if (!g_inited)
        init();

    QMutexLocker lock(&g_mutex);
    if (g_logPath.isEmpty())
        return;

    QFile f(g_logPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);

    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    if (detail.isEmpty())
        out << QStringLiteral("%1 | %2 | %3\n").arg(ts, levelTag(level), event);
    else
        out << QStringLiteral("%1 | %2 | %3 | %4\n").arg(ts, levelTag(level), event, detail);
    out.flush();
}
