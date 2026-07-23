#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QString>

/**
 * @file styleloader.h
 * @brief 全局 QSS 主题加载（浅色 / 深色）
 *
 * main 启动或设置菜单切换时：qApp->setStyleSheet(StyleLoader::loadTheme(themeId))
 */
namespace StyleLoader {

inline constexpr const char *ThemeLight = "light"; // 浅色主题 id，对应 theme_light.qss
inline constexpr const char *ThemeDark = "dark";   // 深色主题 id，对应 theme_dark.qss

QString loadTheme(const QString &themeId); // 按主题 id 加载 QSS；"light" 或 "dark"，其它值回退 light
QString loadAppStyle();                    // 兼容旧调用：默认加载浅色主题

} // namespace StyleLoader

#endif // STYLELOADER_H
