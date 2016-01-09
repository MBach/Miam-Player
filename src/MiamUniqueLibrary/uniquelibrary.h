#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QTranslator>
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
	UniqueLibraryFilterProxyModel *_proxy;

	QTranslator translator;

public:
	explicit UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

protected:
	virtual void changeEvent(QEvent *event) override;

private slots:
	bool playSingleTrack(const QModelIndex &index);

	void skipBackward();

	void skipForward();

	void toggleShuffle();
};

#endif // UNIQUELIBRARY_H
