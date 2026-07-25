#ifndef DEMOCORE_GLOBAL_H
#define DEMOCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DEMOCORE_LIBRARY)
#  define DEMOCORE_EXPORT Q_DECL_EXPORT
#else
#  define DEMOCORE_EXPORT Q_DECL_IMPORT
#endif

#endif
