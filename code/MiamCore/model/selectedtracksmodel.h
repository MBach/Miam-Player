#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QStringList>

#include "miamcore_global.h"

class LibrarySqlModel;

class MIAMCORE_LIBRARY SelectedTracksModel
{
public:
	virtual ~SelectedTracksModel();

	virtual QStringList selectedTracks() = 0;

	virtual void updateSelectedTracks() = 0;

	virtual void init(LibrarySqlModel *sqlModel) = 0;
};

#endif // SELECTEDTRACKSMODEL_H
