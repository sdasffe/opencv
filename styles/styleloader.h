#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QString>

/**
 * @file styleloader.h
 * @brief 全局 QSS 样式加载 —— 应用启动时的视觉主题入口
 *
 * 【在整条链路中的位置】
 *   main.cpp 或 Widget 构造早期：
 *     qApp->setStyleSheet(StyleLoader::loadAppStyle());
 *
 * 【职责】
 *   1. 优先从 Qt 资源 :/styles/app.qss 读取完整主题
 *   2. 资源缺失时返回内置兜底样式，保证界面仍可基本使用
 *
 * 【为什么不用 QWidget::setStyleSheet 分散设置】
 *   集中 QSS 便于统一暗色画布、按钮、处理块等风格，改一处全局生效
 */

namespace StyleLoader {

/**
 * @brief 加载应用全局样式表
 *
 * 谁调用：main 或 Widget 初始化
 * 返回：完整 QSS 字符串，可直接传给 QApplication::setStyleSheet
 *
 * 失败策略：文件打不开时不抛异常，返回硬编码的最小样式（背景色、按钮色等）
 */
QString loadAppStyle();

} // namespace StyleLoader

#endif // STYLELOADER_H
