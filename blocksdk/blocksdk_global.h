#ifndef BLOCKSDK_GLOBAL_H
#define BLOCKSDK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BLOCKSDK_LIBRARY)
#  define BLOCKSDK_EXPORT Q_DECL_EXPORT
#else
#  define BLOCKSDK_EXPORT Q_DECL_IMPORT
#endif

#endif
