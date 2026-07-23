/**
 * @file styleloader.cpp
 * @brief 加载浅色/深色 QSS；优先读磁盘 styles/，避免未重编资源时看不到改动
 */

#include "styleloader.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace StyleLoader {

namespace {

/** @brief 以 UTF-8 读取指定路径的文本文件内容 */
QString readFileUtf8(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString(); // 打开失败返回空串
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8"); // Qt5 编码设置
#else
    in.setEncoding(QStringConverter::Utf8); // Qt6 编码设置
#endif
    return in.readAll(); // 读取全部 QSS 文本
}

/** @brief 解析 QSS 文件路径：优先磁盘 styles/，找不到再用 Qt 资源 :/styles/ */
QString resolveThemePath(const QString &fileName)
{
    QDir dir(QCoreApplication::applicationDirPath()); // 从 exe 目录开始向上查找
    for (int i = 0; i < 8; ++i) {
        const QString candidate = dir.filePath(QStringLiteral("styles/") + fileName); // 候选路径
        if (QFile::exists(candidate))
            return candidate; // 找到磁盘文件
        if (!dir.cdUp())
            break; // 无法继续向上
    }
    const QString cwd = QDir::current().filePath(QStringLiteral("styles/") + fileName); // 再试当前工作目录
    if (QFile::exists(cwd))
        return cwd;
    return QStringLiteral(":/styles/") + fileName; // 回退到 Qt 资源路径
}

/** @brief 浅色主题内置兜底 QSS（文件均不可读时使用） */
QString fallbackLight()
{
    return QStringLiteral(
        "QWidget#Widget { background-color: #F0F0F0; color: #2B2B2B; }"
        "QGraphicsView#graphicsView { background-color: #2B2B2B; }"
        "QPushButton#btnApply { background-color: #3574F0; color: white; border: none; }");
}

/** @brief 深色主题内置兜底 QSS（文件均不可读时使用） */
QString fallbackDark()
{
    return QStringLiteral(
        "QWidget#Widget { background-color: #1E1F22; color: #DFE1E5; }"
        "QGraphicsView#graphicsView { background-color: #141517; }"
        "QPushButton#btnApply { background-color: #3574F0; color: white; border: none; }");
}

} // namespace

/** @brief 按主题 id 加载完整 QSS 字符串 */
QString loadTheme(const QString &themeId)
{
    const bool dark = (themeId.compare(QLatin1String(ThemeDark), Qt::CaseInsensitive) == 0); // 判断是否深色
    const QString fileName = dark ? QStringLiteral("theme_dark.qss")
                                  : QStringLiteral("theme_light.qss"); // 对应文件名
    const QString path = resolveThemePath(fileName); // 解析实际路径
    const QString qss = readFileUtf8(path);         // 读取主题 QSS
    if (!qss.isEmpty())
        return qss; // 成功读取主题文件
    return dark ? fallbackDark() : fallbackLight(); // 文件不可读时用内置兜底
}

/** @brief 兼容旧接口：加载默认浅色主题 */
QString loadAppStyle()
{
    return loadTheme(QLatin1String(ThemeLight)); // 委托 loadTheme
}

} // namespace StyleLoader
