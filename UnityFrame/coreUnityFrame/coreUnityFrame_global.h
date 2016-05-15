#ifndef COREUNITYFRAME_GLOBAL_H
#define COREUNITYFRAME_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COREUNITYFRAME_LIBRARY)
#  define API_EXPORT Q_DECL_EXPORT
#else
#  define API_EXPORT Q_DECL_IMPORT
#endif

#ifdef Q_OS_WIN
#   define PATH_LIST_SEPARATOR ";"
#else
#   define PATH_LIST_SEPARATOR ":"
#endif

#endif // COREUNITYFRAME_GLOBAL_H
