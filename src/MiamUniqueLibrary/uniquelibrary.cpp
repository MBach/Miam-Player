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
	, _currentTrack(nullptr)
{
	setupUi(this);
	library->setItemDelegate(new UniqueLibraryItemDelegate(library->jumpToWidget(), library->model()->proxy()));
	library->setSelectionBehavior(QAbstractItemView::SelectRows);
	library->setSelectionMode(QAbstractItemView::ExtendedSelection);

	_model = library->model();
	_proxy = library->model()->proxy();

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &SearchBar::aboutToStartSearch, [=](const QString &text) {
		library->model()->proxy()->findMusic(text);
	});
	connect(library, &ListView::doubleClicked, this, &UniqueLibrary::playSingleTrack);

	connect(skipBackwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipBackward);
	connect(seekBackwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(playButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::play);
	connect(stopButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::stop);
	connect(seekForwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekForward);
	connect(skipForwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipForward);
	connect(toggleShuffleButton, &MediaButton::clicked, this, &UniqueLibrary::toggleShuffle);

	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		if (_currentTrack) {
			if (state == QMediaPlayer::StoppedState) {
				_currentTrack->setData(false, Miam::DF_Highlighted);
			} else if (state == QMediaPlayer::PlayingState) {
				_currentTrack->setData(true, Miam::DF_Highlighted);
			}
		}
	});

	connect(_mediaPlayer, &MediaPlayer::positionChanged, this, [=](qint64 pos, qint64) {
		if (_currentTrack) {
			uint p = pos / 1000;
			_currentTrack->setData(p, Miam::DF_CurrentPosition);
		}
	});
}

bool UniqueLibrary::playSingleTrack(const QModelIndex &index)
{
	if (_currentTrack) {
		_currentTrack->setData(false, Miam::DF_Highlighted);
	}
	QStandardItem *item = _model->itemFromIndex(_proxy->mapToSource(index));
	if (item && item->type() == Miam::IT_Track) {
		_mediaPlayer->playMediaContent(QUrl::fromLocalFile(index.data(Miam::DF_URI).toString()));
		_currentTrack = item;
		return true;
	} else {
		return false;
	}
}

void UniqueLibrary::skipBackward()
{
	qDebug() << Q_FUNC_INFO << "not yet implemented";
	if (!_currentTrack) {
		return;
	}
}

void UniqueLibrary::skipForward()
{
	if (!_currentTrack) {
		return;
	}
	QModelIndex current = _proxy->mapFromSource(_model->index(_currentTrack->row(), 0));
	int row = current.row();
	while (row < _model->rowCount()) {
		QModelIndex next = current.sibling(row + 1, 0);
		if (this->playSingleTrack(next)) {
			break;
		} else {
			row++;
		}
	}
}

void UniqueLibrary::toggleShuffle()
{
	qDebug() << Q_FUNC_INFO << "not yet implemented";
}
