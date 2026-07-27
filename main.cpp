/**
 * @file main.cpp
 * @brief 程序入口：创建应用、日志、字体、主题，显示主窗口并进入事件循环
 *
 * 业务逻辑在 core/widget.cpp 与 core/imageprocessor.cpp。
 */

#include "core/widget.h"
#include "config/appconfig.h"
#include "styles/styleloader.h"
#include "utils/applogger.h"
#include "blocksdk/pluginmanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QImageReader>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

/** 可能存放算法插件的目录（exe 旁 + 向上查找 bin/debug|release/plugins） */
static QStringList candidatePluginDirs()
{
    QStringList dirs;
    const auto add = [&dirs](const QString &path) {
        const QString n = QDir::cleanPath(path);
        if (!n.isEmpty() && QDir(n).exists() && !dirs.contains(n))
            dirs.append(n);
    };

    const QString appDir = QCoreApplication::applicationDirPath();
    add(QDir(appDir).filePath(QStringLiteral("plugins")));

    QDir walk(appDir);
    for (int i = 0; i < 8 && walk.cdUp(); ++i) {
        add(walk.filePath(QStringLiteral("bin/debug/plugins")));
        add(walk.filePath(QStringLiteral("bin/release/plugins")));
        add(walk.filePath(QStringLiteral("plugins")));
    }
    return dirs;
}

/** 让插件能找到同级目录的 blocksdk.dll */
static void prepareDllSearchPath(const QString &pluginDir)
{
    const QString binDir = QFileInfo(pluginDir).absolutePath(); // .../plugins 的上一级
    QCoreApplication::addLibraryPath(binDir);
#ifdef Q_OS_WIN
    SetDllDirectoryW(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(binDir).utf16()));
    const QByteArray oldPath = qgetenv("PATH");
    qputenv("PATH", QFile::encodeName(QDir::toNativeSeparators(binDir)) + ';' + oldPath);
#endif
}

/**
 * @brief 程序入口
 * @return 事件循环退出码（一般为 0）
 */
int main(int argc, char *argv[])
{
    // 须在任何读图之前设置：否则超大图（如 10000×10000）会被 Qt6 默认 256MB 上限拒载
    QImageReader::setAllocationLimit(AppConfig::IMAGE_ALLOCATION_LIMIT_MB);

    QApplication a(argc, argv);                                      // 必须最先创建；后续控件依赖它
    a.setApplicationName(QString::fromUtf8(AppConfig::APP_NAME_ZH)); // 应用显示名（中文）
    a.setApplicationVersion(QString::fromUtf8(AppConfig::APP_VERSION)); // 版本号
    a.setOrganizationName(QStringLiteral("Image Processing Toolbox")); // QSettings 组织名

    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        a.setStyle(fusion);                                          // Fusion 才能充分吃到 QSS

    AppLogger::init();
    AppLogger::info(QStringLiteral("程序启动"));
    AppLogger::info(QStringLiteral("可执行文件"), QCoreApplication::applicationFilePath());
    AppLogger::info(QStringLiteral("图像内存上限"),
                    AppConfig::IMAGE_ALLOCATION_LIMIT_MB == 0
                        ? QStringLiteral("不限制")
                        : QStringLiteral("%1 MB").arg(AppConfig::IMAGE_ALLOCATION_LIMIT_MB));

    // 算法来自 plugins/*.dll（优先 exe 旁；开发时兼容 Qt Creator 旧影子目录）
    QString usedPluginDir;
    int loaded = 0;
    for (const QString &dir : candidatePluginDirs()) {
        prepareDllSearchPath(dir);
        loaded = PluginManager::instance().loadFromDirectory(dir);
        AppLogger::info(QStringLiteral("尝试插件目录"),
                        QStringLiteral("%1 → %2").arg(dir).arg(loaded));
        if (loaded > 0) {
            usedPluginDir = dir;
            break;
        }
    }
    if (loaded <= 0) {
        const QString hint = QStringLiteral(
            "未加载到算法插件。\n\n"
            "请运行：\n  D:/Qt/project/opencv/bin/debug/opencv.exe\n"
            "并确认旁边有：\n  bin/debug/plugins/block_*.dll\n\n"
            "Qt Creator 请选择运行目标为 opencv_app，不要跑旧的 build/.../opencv.exe。");
        AppLogger::warn(QStringLiteral("算法插件"), QStringLiteral("加载失败"));
        QMessageBox::warning(nullptr, QStringLiteral("提示"), hint);
    } else {
        AppLogger::info(QStringLiteral("算法插件"),
                        QStringLiteral("目录=%1 加载=%2").arg(usedPluginDir).arg(loaded));
    }

    QObject::connect(&a, &QCoreApplication::aboutToQuit, []() {
        AppLogger::info(QStringLiteral("程序退出"));
    });

    QFont appFont(QStringLiteral("Microsoft YaHei UI"), 10);         // 优先雅黑 UI
    if (appFont.exactMatch() == false)
        appFont = QFont(QStringLiteral("Microsoft YaHei"), 10);      // 没有则回退普通雅黑
    a.setFont(appFont);                                              // 全局默认字体

    {
        QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
        const QString theme = settings.value(QStringLiteral("ui/theme"),
                                             QLatin1String(StyleLoader::ThemeLight)).toString(); // 读上次主题
        a.setStyleSheet(StyleLoader::loadTheme(theme));              // 启动即套用浅色/深色 QSS
    }

    Widget w;                                                        // 主窗口：画布、菜单、处理链、ROI
    w.show();                                                        // 显示窗口
    AppLogger::info(QStringLiteral("主窗口已显示"));

    return QApplication::exec();                                     // 进入事件循环直到用户退出
}
