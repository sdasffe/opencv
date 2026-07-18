#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QString>

namespace StyleLoader {

/** 从 Qt 资源加载全局 QSS；失败时返回内置兜底样式 */
QString loadAppStyle();

} // namespace StyleLoader

#endif // STYLELOADER_H
