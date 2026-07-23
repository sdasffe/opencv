/**
 * @file main.cpp
 * @brief 程序入口
 *
 * 运行过程概览：
 *   1. 创建 QApplication（Qt 事件循环容器）
 *   2. 设置全局字体、加载 QSS 样式
 *   3. 创建并显示主窗口 Widget
 *   4. 进入事件循环，等待用户操作（打开图、拖算法、调参、画 ROI 等）
 *
 * 真正的业务逻辑都在 core/widget.cpp 与 core/imageprocessor.cpp 中。
 */

#include "core/widget.h"
#include "config/appconfig.h"
#include "styles/styleloader.h"
#include "utils/applogger.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    // ---------- 1. 创建 Qt 应用对象 ----------
    // 必须最先创建；之后所有界面控件都依赖它
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromUtf8(AppConfig::APP_NAME_ZH));
    a.setApplicationVersion(QString::fromUtf8(AppConfig::APP_VERSION));
    a.setOrganizationName(QStringLiteral("Image Processing Toolbox"));

    // Fusion：Windows 原生样式会吞掉大量 QSS，IDE 质感必须走 Fusion
    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        a.setStyle(fusion);

    // ---------- 1b. 日志 ----------
    // 写入 debug/logs/，便于排查处理链与 ROI 问题
    AppLogger::init();
    AppLogger::info(QStringLiteral("程序启动"));
    QObject::connect(&a, &QCoreApplication::aboutToQuit, []() {
        AppLogger::info(QStringLiteral("程序退出"));
    });

    // ---------- 2. 全局字体 ----------
    // 优先微软雅黑 UI，没有则回退到微软雅黑，保证中文显示清晰
    QFont appFont(QStringLiteral("Microsoft YaHei UI"), 10);
    if (appFont.exactMatch() == false)
        appFont = QFont(QStringLiteral("Microsoft YaHei"), 10);
    a.setFont(appFont);

    // ---------- 3. 全局样式表（浅色 / 深色，IDE 风） ----------
    {
        QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
        const QString theme = settings.value(QStringLiteral("ui/theme"),
                                             QLatin1String(StyleLoader::ThemeLight)).toString();
        a.setStyleSheet(StyleLoader::loadTheme(theme));
    }

    // ---------- 4. 主窗口 ----------
    // Widget 内部会：搭画布、侧栏、处理链面板，并创建 ImageProcessor
    Widget w;
    w.show();
    AppLogger::info(QStringLiteral("主窗口已显示"));

    // ---------- 5. 事件循环 ----------
    // 阻塞在此，直到用户关闭窗口；期间所有点击/拖拽/信号槽都会在这里被分发
    return QApplication::exec();
}
