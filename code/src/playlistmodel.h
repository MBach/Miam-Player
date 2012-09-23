#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QStandardItemModel>

class PlaylistModel : public QStandardItemModel
{
	Q_OBJECT
public:
	explicit PlaylistModel(QObject *parent = 0);
	
signals:
	
public slots:
	
};

#endif // PLAYLISTMODEL_H
