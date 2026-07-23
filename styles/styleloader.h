#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QString>

/**
 * @file styleloader.h
 * @brief 全局 QSS 主题加载（浅色 / 深色，IDEA 风格）
 *
 * main 启动或设置菜单切换时：
 *   qApp->setStyleSheet(StyleLoader::loadTheme(themeId));
 */

namespace StyleLoader {

/** 主题 id：与资源文件名对应 */
inline constexpr const char *ThemeLight = "light";
inline constexpr const char *ThemeDark = "dark";

/**
 * @brief 按主题加载 QSS
 * @param themeId "light" 或 "dark"；其它值回退 light
 */
QString loadTheme(const QString &themeId);

/** @brief 兼容旧调用：默认浅色 */
QString loadAppStyle();

} // namespace StyleLoader

#endif // STYLELOADER_H
