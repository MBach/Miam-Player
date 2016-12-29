#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QUrl>

#include "miamcore_global.h"
#include "model/trackdao.h"

/**
 * \brief		The SelectedTracksModel class
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SelectedTracksModel
{
public:
	virtual ~SelectedTracksModel();

	virtual QList<QUrl> selectedTracks() = 0;

	virtual void updateSelectedTracks() = 0;
};

#endif // SELECTEDTRACKSMODEL_H
