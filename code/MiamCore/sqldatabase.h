#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QSqlDatabase>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY SqlDatabase : public QSqlDatabase
{

public:
	explicit SqlDatabase();

	virtual ~SqlDatabase() {}
};

#endif // SQLDATABASE_H
