#ifndef SELECTEDTRACKSMODEL_H
#define SELECTEDTRACKSMODEL_H

#include <QItemSelectionModel>
#include <QStringList>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY SelectedTracksModel : public QObject
{
	Q_OBJECT
private:
	QItemSelectionModel *_itemSelectionModel;

public:
	explicit SelectedTracksModel(QItemSelectionModel *itemSelectionModel);

	QStringList selectedTracks() const;
};

#endif // SELECTEDTRACKSMODEL_H
