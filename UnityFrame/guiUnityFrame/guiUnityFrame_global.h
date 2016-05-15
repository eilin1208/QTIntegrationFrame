#ifndef GUIUNITYFRAME_GLOBAL_H
#define GUIUNITYFRAME_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GUIUNITYFRAME_LIBRARY)
#  define GUI_API_EXPORT Q_DECL_EXPORT
#else
#  define GUI_API_EXPORT Q_DECL_IMPORT
#endif

#endif // GUIUNITYFRAME_GLOBAL_H
