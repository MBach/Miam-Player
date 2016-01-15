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
	seekSlider->setMediaPlayer(_mediaPlayer);
	connect(_mediaPlayer, &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			seekSlider->setValue(1000 * pos / duration);
		}
	});
	connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
		_mediaPlayer->setVolume((qreal)value / 100.0);
	});
	volumeSlider->setValue(Settings::instance()->volume() * 100);

	library->setItemDelegate(new UniqueLibraryItemDelegate(library->jumpToWidget(), library->model()->proxy()));
	_proxy = library->model()->proxy();

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &SearchBar::aboutToStartSearch, library->model()->proxy(), &UniqueLibraryFilterProxyModel::findMusic);
	connect(library, &TableView::doubleClicked, this, &UniqueLibrary::playSingleTrack);

	connect(skipBackwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipBackward);
	connect(seekBackwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(playButton, &MediaButton::clicked, this, [=]() {
		if (_currentTrack && _mediaPlayer->state() == QMediaPlayer::StoppedState) {
			this->playSingleTrack(_proxy->mapFromSource(_currentTrack->index()));
		} else {
			_mediaPlayer->togglePlayback();
		}
	});
	connect(stopButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::stop);
	connect(seekForwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekForward);
	connect(skipForwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipForward);
	connect(toggleShuffleButton, &MediaButton::clicked, this, &UniqueLibrary::toggleShuffle);

	auto settings = Settings::instance();
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		qDebug() << "ici" << state;
		switch (state) {
		case QMediaPlayer::StoppedState:
			if (_currentTrack) {
				_currentTrack->setData(false, Miam::DF_Highlighted);
			}
			playButton->setIcon(QIcon(":/player/" + settings->theme() + "/play"));
			seekSlider->setValue(0);
			break;
		case QMediaPlayer::PlayingState:
			if (_currentTrack) {
				_currentTrack->setData(true, Miam::DF_Highlighted);
			}
			playButton->setIcon(QIcon(":/player/" + settings->theme() + "/pause"));
			break;
		case QMediaPlayer::PausedState:
			playButton->setIcon(QIcon(":/player/" + settings->theme() + "/play"));
			break;
		}
		seekSlider->update();
	});

	connect(_mediaPlayer, &MediaPlayer::positionChanged, this, [=](qint64 pos, qint64) {
		if (_currentTrack) {
			uint p = pos / 1000;
			_currentTrack->setData(p, Miam::DF_CurrentPosition);
		}
	});

	auto settingsPrivate = SettingsPrivate::instance();
	connect(settingsPrivate, &SettingsPrivate::languageAboutToChange, this, [=](const QString &newLanguage) {
		QApplication::removeTranslator(&translator);
		translator.load(":/uniqueLibrary_" + newLanguage);
		QApplication::installTranslator(&translator);
	});

	// Init language
	translator.load(":/uniqueLibrary_" + settingsPrivate->language());
	QApplication::installTranslator(&translator);
}

void UniqueLibrary::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

bool UniqueLibrary::playSingleTrack(const QModelIndex &index)
{
	QStandardItem *item = library->model()->itemFromIndex(_proxy->mapToSource(index));
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
	if (_currentTrack) {
		_currentTrack->setData(false, Miam::DF_Highlighted);
	} else {
		return;
	}
	if (toggleShuffleButton->isChecked()) {
		this->playSingleTrack(_randomHistoryList.takeLast());
	} else {
		QModelIndex current = _proxy->mapFromSource(library->model()->index(_currentTrack->row(), 1));
		int row = current.row();
		while (row >= 0) {
			QModelIndex previous = current.sibling(row - 1, 1);
			if (this->playSingleTrack(previous)) {
				library->scrollTo(previous);
				break;
			} else {
				row--;
			}
		}
	}
}

void UniqueLibrary::skipForward()
{
	if (_currentTrack) {
		_currentTrack->setData(false, Miam::DF_Highlighted);
	} else {
		return;
	}

	if (toggleShuffleButton->isChecked()) {
		//
	} else {
		QModelIndex current = _proxy->mapFromSource(library->model()->index(_currentTrack->row(), 1));
		int row = current.row();
		while (row < library->model()->rowCount()) {
			QModelIndex next = current.sibling(row + 1, 1);
			if (this->playSingleTrack(next)) {
				library->scrollTo(next);
				break;
			} else {
				row++;
			}
		}
	}
}

void UniqueLibrary::toggleShuffle()
{
	toggleShuffleButton->setChecked(toggleShuffleButton->isChecked());
	qDebug() << Q_FUNC_INFO << "not yet implemented" << toggleShuffleButton->isChecked();
	if (toggleShuffleButton->isChecked()) {

	} else {
		_randomHistoryList.clear();
	}
}
