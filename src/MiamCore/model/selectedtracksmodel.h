#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QStringList>

#include "miamcore_global.h"
#include "model/trackdao.h"

/// Forward declaration
class SqlDatabase;

/**
 * \brief		The SelectedTracksModel class
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SelectedTracksModel
{
public:
	virtual ~SelectedTracksModel();

	virtual QStringList selectedTracks() = 0;

	virtual void updateSelectedTracks() = 0;
};

#endif // SELECTEDTRACKSMODEL_H
