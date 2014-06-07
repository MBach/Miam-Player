#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QStringList>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY SelectedTracksModel
{
public:
	virtual QStringList selectedTracks() const = 0;
};

#endif // SELECTEDTRACKSMODEL_H
