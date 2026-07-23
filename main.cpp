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

#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>

/**
 * @brief 程序入口
 * @return 事件循环退出码（一般为 0）
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);                                      // 必须最先创建；后续控件依赖它
    a.setApplicationName(QString::fromUtf8(AppConfig::APP_NAME_ZH)); // 应用显示名（中文）
    a.setApplicationVersion(QString::fromUtf8(AppConfig::APP_VERSION)); // 版本号
    a.setOrganizationName(QStringLiteral("Image Processing Toolbox")); // QSettings 组织名

    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        a.setStyle(fusion);                                          // Fusion 才能充分吃到 QSS

    AppLogger::init();                                               // 创建 logs/ 并打开当日日志文件
    AppLogger::info(QStringLiteral("程序启动"));
    QObject::connect(&a, &QCoreApplication::aboutToQuit, []() {
        AppLogger::info(QStringLiteral("程序退出"));                 // 关窗退出前打一条尾日志
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
