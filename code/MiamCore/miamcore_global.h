#ifndef MIAMCORE_GLOBAL_H
#define MIAMCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MIAM_PLUGIN
# define MIAMCORE_LIBRARY Q_DECL_EXPORT
#else
# define MIAMCORE_LIBRARY Q_DECL_IMPORT
#endif

#endif // MIAMCORE_GLOBAL_H
