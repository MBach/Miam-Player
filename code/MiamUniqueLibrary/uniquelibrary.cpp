#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include <QStandardItemModel>
#include <library/jumptowidget.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include "uniquelibraryitemdelegate.h"

#include <QtDebug>

UniqueLibrary::UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent)
	: QWidget(parent)
	, _mediaPlayer(mediaPlayer)
{
	setupUi(this);
	library->setItemDelegate(new UniqueLibraryItemDelegate(library->jumpToWidget(), library->model()->proxy()));
	library->setSelectionBehavior(QAbstractItemView::SelectRows);
	library->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &SearchBar::aboutToStartSearch, [=](const QString &text) {
		library->model()->proxy()->findMusic(text);
	});
	connect(library, &ListView::doubleClicked, this, &UniqueLibrary::playSingleTrack);

	/*connect(_mediaPlayer, &MediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
		qDebug() << "mediaStatusChanged" << status;
	});*/
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		qDebug() << "mediaStatusChanged" << state;
	});
}

void UniqueLibrary::playSingleTrack(const QModelIndex &index)
{
	QStandardItem *item = library->model()->itemFromIndex(library->model()->proxy()->mapToSource(index));
	if (item && item->type() == Miam::IT_Track) {
		_mediaPlayer->playMediaContent(QMediaContent(QUrl::fromLocalFile(item->data(Miam::DF_URI).toString())));
		item->setData(true, Miam::DF_Highlighted);
	}
}
