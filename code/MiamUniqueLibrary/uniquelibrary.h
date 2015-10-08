#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QSet>
#include <QWidget>

#include "miamuniquelibrary_global.h"
#include "model/sqldatabase.h"

#include "ui_uniquelibrary.h"

/**
* \brief
* \details
* \author      Matthieu Bachelier
* \copyright   GNU General Public License v3
*/
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget, public Ui::UniqueLibrary
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

	QStandardItem *_currentTrack;

public:
	explicit UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent = 0);

private slots:
	void playSingleTrack(const QModelIndex &index);

	void skipBackward();

	void skipForward();

	void toggleShuffle();
};

#endif // UNIQUELIBRARY_H
