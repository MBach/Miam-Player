#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QStringList>

#include "miamcore_global.h"
#include "model/trackdao.h"

class SqlDatabase;

class MIAMCORE_LIBRARY SelectedTracksModel
{
public:
	virtual ~SelectedTracksModel();

	virtual QStringList selectedTracks() = 0;

	virtual void updateSelectedTracks() = 0;

	virtual void init(SqlDatabase *db) = 0;
};

#endif // SELECTEDTRACKSMODEL_H
