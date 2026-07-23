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

QString readFileUtf8(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    return in.readAll();
}

/** 从 exe 向上找 styles/<fileName>，找不到再用 :/styles/ */
QString resolveThemePath(const QString &fileName)
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; ++i) {
        const QString candidate = dir.filePath(QStringLiteral("styles/") + fileName);
        if (QFile::exists(candidate))
            return candidate;
        if (!dir.cdUp())
            break;
    }
    const QString cwd = QDir::current().filePath(QStringLiteral("styles/") + fileName);
    if (QFile::exists(cwd))
        return cwd;
    return QStringLiteral(":/styles/") + fileName;
}

QString fallbackLight()
{
    return QStringLiteral(
        "QWidget#Widget { background-color: #F0F0F0; color: #2B2B2B; }"
        "QGraphicsView#graphicsView { background-color: #2B2B2B; }"
        "QPushButton#btnApply { background-color: #3574F0; color: white; border: none; }");
}

QString fallbackDark()
{
    return QStringLiteral(
        "QWidget#Widget { background-color: #1E1F22; color: #DFE1E5; }"
        "QGraphicsView#graphicsView { background-color: #141517; }"
        "QPushButton#btnApply { background-color: #3574F0; color: white; border: none; }");
}

} // namespace

QString loadTheme(const QString &themeId)
{
    const bool dark = (themeId.compare(QLatin1String(ThemeDark), Qt::CaseInsensitive) == 0);
    const QString fileName = dark ? QStringLiteral("theme_dark.qss")
                                  : QStringLiteral("theme_light.qss");
    const QString path = resolveThemePath(fileName);
    const QString qss = readFileUtf8(path);
    if (!qss.isEmpty())
        return qss;
    const QString legacy = readFileUtf8(QStringLiteral(":/styles/app.qss"));
    if (!legacy.isEmpty())
        return legacy;
    return dark ? fallbackDark() : fallbackLight();
}

QString loadAppStyle()
{
    return loadTheme(QLatin1String(ThemeLight));
}

} // namespace StyleLoader
