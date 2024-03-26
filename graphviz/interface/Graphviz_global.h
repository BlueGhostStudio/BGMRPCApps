#ifndef GRAPHVIZ_GLOBAL_H
#define GRAPHVIZ_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GRAPHVIZ_LIBRARY)
#define GRAPHVIZ_EXPORT Q_DECL_EXPORT
#else
#define GRAPHVIZ_EXPORT Q_DECL_IMPORT
#endif

#endif  // GRAPHVIZ_GLOBAL_H
