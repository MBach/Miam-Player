#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QSet>
#include <QWidget>

#include "miamuniquelibrary_global.hpp"
#include "model/sqldatabase.h"

#include "ui_uniquelibrary.h"

/**
 * \brief		The UniqueLibrary class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget, public Ui::UniqueLibrary
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;
	QStandardItem *_currentTrack;
	UniqueLibraryItemModel *_model;
	MiamSortFilterProxyModel *_proxy;

public:
	explicit UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent = 0);

private slots:
	bool playSingleTrack(const QModelIndex &index);

	void skipBackward();

	void skipForward();

	void toggleShuffle();
};

#endif // UNIQUELIBRARY_H
