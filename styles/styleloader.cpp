/**
 * @file styleloader.cpp
 * @brief 从 Qt 资源或内置字符串加载 QSS
 *
 * 【资源路径】
 *   :/styles/app.qss —— 在 .qrc 中注册，编译进可执行文件
 *
 * 【Qt5/Qt6 差异】
 *   QTextStream 编码：Qt5 用 setCodec("UTF-8")，Qt6 用 QStringConverter::Utf8
 *   保证中文 QSS 注释与选择器正常读取
 */

#include "styleloader.h"

#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace StyleLoader {

/**
 * @brief 加载全局 QSS
 *
 * 流程：
 *   1. 打开资源文件 ReadOnly | Text
 *   2. 设置 UTF-8，readAll 读入
 *   3. 失败则返回 QStringLiteral 内置兜底（主窗口、GraphicsView、按钮配色）
 */
QString loadAppStyle()
{
    QFile file(QStringLiteral(":/styles/app.qss"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        in.setCodec("UTF-8");
#else
        in.setEncoding(QStringConverter::Utf8);
#endif
        return in.readAll();
    }

    // 兜底：资源未打包或路径错误时仍能显示基本 UI
    return QStringLiteral(
        "QWidget#Widget { background-color: #E8ECF1; }"
        "QGraphicsView#graphicsView { background-color: #1A1D23; border-radius: 10px; }"
        "QPushButton#pushButton, QPushButton#pushButton_3 {"
        "  background-color: #0F766E; color: white; border: none;"
        "  border-radius: 8px; padding: 8px 14px; }"
        "QPushButton#deltete {"
        "  background-color: #DC2626; color: white; border: none;"
        "  border-radius: 8px; padding: 8px 14px; }"
        );
}

} // namespace StyleLoader
