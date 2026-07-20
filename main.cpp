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
#include "styles/styleloader.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    // ---------- 1. 创建 Qt 应用对象 ----------
    // 必须最先创建；之后所有界面控件都依赖它
    QApplication a(argc, argv);

    // ---------- 2. 全局字体 ----------
    // 优先微软雅黑 UI，没有则回退到微软雅黑，保证中文显示清晰
    QFont appFont(QStringLiteral("Microsoft YaHei UI"), 9);
    if (appFont.exactMatch() == false)
        appFont = QFont(QStringLiteral("Microsoft YaHei"), 9);
    a.setFont(appFont);

    // ---------- 3. 全局样式表 ----------
    // StyleLoader 从 resources.qrc 里读 app.qss 并应用
    a.setStyleSheet(StyleLoader::loadAppStyle());

    // ---------- 4. 主窗口 ----------
    // Widget 内部会：搭画布、侧栏、处理链面板，并创建 ImageProcessor
    Widget w;
    w.show();

    // ---------- 5. 事件循环 ----------
    // 阻塞在此，直到用户关闭窗口；期间所有点击/拖拽/信号槽都会在这里被分发
    return QApplication::exec();
}
