#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include <library/jumptowidget.h>
#include <styling/paintablewidget.h>
#include <filehelper.h>
#include <musicsearchengine.h>
#include <settingsprivate.h>
#include "uniquelibraryitemdelegate.h"

#include <QLabel>
#include <QProgressBar>
#include <QStandardItemModel>

#include <ctime>
#include <random>

#include <QtDebug>

UniqueLibrary::UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent)
	: AbstractView(parent)
	, _mediaPlayer(mediaPlayer)
	, _currentTrack(nullptr)
{
	setupUi(this);
	playButton->setMediaPlayer(_mediaPlayer);
	stopButton->setMediaPlayer(_mediaPlayer);
	seekSlider->setMediaPlayer(_mediaPlayer);
	playbackModeButton->setToggleShuffleOnly(true);

	_mediaPlayer->setPlaylist(nullptr);

	connect(_mediaPlayer, &MediaPlayer::positionChanged, this, [=](qint64 pos, qint64) {
		if (_currentTrack) {
			uint p = pos / 1000;
			_currentTrack->setData(p, Miam::DF_CurrentPosition);
		}
	});

	connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
		_mediaPlayer->setVolume((qreal)value / 100.0);
	});
	auto settings = Settings::instance();
	volumeSlider->setValue(settings->volume() * 100);

	uniqueTable->setItemDelegate(new UniqueLibraryItemDelegate(uniqueTable));
	_proxy = uniqueTable->model()->proxy();

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &SearchBar::aboutToStartSearch, uniqueTable->model()->proxy(), &UniqueLibraryFilterProxyModel::findMusic);
	connect(uniqueTable, &TableView::doubleClicked, this, &UniqueLibrary::playSingleTrack);

	connect(skipBackwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipBackward);
	connect(seekBackwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(playButton, &MediaButton::clicked, this, [=]() {
		if (_currentTrack && _mediaPlayer->state() == QMediaPlayer::StoppedState) {
			this->playSingleTrack(_proxy->mapFromSource(_currentTrack->index()));
		} else {
			_mediaPlayer->togglePlayback();
		}
	});
	connect(stopButton, &MediaButton::clicked, this, [=]() {
		if (_currentTrack) {
			_currentTrack->setData(false, Miam::DF_Highlighted);
		}
		_mediaPlayer->stop();
	});
	connect(seekForwardButton, &MediaButton::clicked, _mediaPlayer, &MediaPlayer::seekForward);
	connect(skipForwardButton, &MediaButton::clicked, this, &UniqueLibrary::skipForward);
	connect(playbackModeButton, &MediaButton::clicked, this, &UniqueLibrary::toggleShuffle);

	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State) {
		seekSlider->update();
	});

	connect(_mediaPlayer, &MediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
		if (_mediaPlayer->state() != QMediaPlayer::StoppedState && status == QMediaPlayer::EndOfMedia) {
			if (_mediaPlayer->isStopAfterCurrent()) {
				_mediaPlayer->stop();
				_mediaPlayer->setStopAfterCurrent(false);
			} else {
				if (playbackModeButton->isChecked()) {
					_randomHistoryList.append(_proxy->mapFromSource(_currentTrack->index()));
				}
				skipForward();
			}
		}
	});

	auto settingsPrivate = SettingsPrivate::instance();
	connect(settingsPrivate, &SettingsPrivate::languageAboutToChange, this, [=](const QString &newLanguage) {
		QApplication::removeTranslator(&translator);
		translator.load(":/translations/uniqueLibrary_" + newLanguage);
		QApplication::installTranslator(&translator);
	});

	connect(playbackModeButton, &PlaybackModeButton::toggled, this, [=](bool checked) {
		settingsPrivate->setValue("uniqueLibraryIsInShuffleState", checked);
	});

	// Init button theme
	for (MediaButton *b : this->findChildren<MediaButton*>()) {
		b->setIconFromTheme(settings->theme());
	}

	playbackModeButton->setChecked(settingsPrivate->value("uniqueLibraryIsInShuffleState").toBool());

	// Init language
	translator.load(":/translations/uniqueLibrary_" + settingsPrivate->language());
	QApplication::installTranslator(&translator);

	std::srand(std::time(nullptr));

	uniqueTable->setFocus();
}

UniqueLibrary::~UniqueLibrary()
{
	disconnect(_mediaPlayer, &MediaPlayer::positionChanged, seekSlider, &SeekBar::setPosition);
	_mediaPlayer->stop();
}

bool UniqueLibrary::viewProperty(SettingsPrivate::ViewProperty vp) const
{
	switch (vp) {
	case SettingsPrivate::VP_MediaControls:
	case SettingsPrivate::VP_SearchArea:
	case SettingsPrivate::VP_VolumeIndicatorToggled:
	case SettingsPrivate::VP_HasAreaForRescan:
		return true;
	case SettingsPrivate::VP_HasTracksToDisplay:
		return uniqueTable->model()->rowCount() > 0;
	default:
		return false;
	}
}

void UniqueLibrary::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void UniqueLibrary::closeEvent(QCloseEvent *event)
{

	QWidget::closeEvent(event);
}

void UniqueLibrary::setMusicSearchEngine(MusicSearchEngine *musicSearchEngine)
{
	connect(musicSearchEngine, &MusicSearchEngine::aboutToSearch, this, [=]() {

		QVBoxLayout *vbox = new QVBoxLayout;
		vbox->setMargin(0);
		vbox->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

		PaintableWidget *paintable = new PaintableWidget(uniqueTable);
		paintable->setHalfTopBorder(false);
		paintable->setFrameBorder(false, true, false, false);
		vbox->addWidget(paintable);
		QVBoxLayout *vbox2 = new QVBoxLayout;
		vbox2->addWidget(new QLabel(tr("Your library is updating..."), paintable));
		vbox2->addWidget(new QProgressBar(paintable));
		paintable->setLayout(vbox2);
		uniqueTable->setLayout(vbox);
	});

	connect(musicSearchEngine, &MusicSearchEngine::progressChanged, this, [=](int p) {
		if (QProgressBar *progress = this->findChild<QProgressBar*>()) {
			progress->setValue(p);
		}
	});

	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, [=]() {
		auto l = uniqueTable->layout();
		while (!l->isEmpty()) {
			if (QLayoutItem *i = l->takeAt(0)) {
				if (QWidget *w = i->widget()) {
					delete w;
				}
				delete i;
			}
		}
		delete uniqueTable->layout();
		uniqueTable->model()->load();
	});
}

void UniqueLibrary::setViewProperty(SettingsPrivate::ViewProperty vp, QVariant value)
{
	Q_UNUSED(value)
	switch (vp) {
	case SettingsPrivate::VP_SearchArea:
		searchBar->clear();
		break;
	default:
		break;
	}
}

void UniqueLibrary::volumeSliderDecrease()
{
	volumeSlider->setValue(volumeSlider->value() - 5);
}

void UniqueLibrary::volumeSliderIncrease()
{
	volumeSlider->setValue(volumeSlider->value() + 5);
}

bool UniqueLibrary::playSingleTrack(const QModelIndex &index)
{
	QStandardItem *item = uniqueTable->model()->itemFromIndex(_proxy->mapToSource(index));
	if (item && item->type() == Miam::IT_Track) {
		_mediaPlayer->playMediaContent(QUrl::fromLocalFile(index.data(Miam::DF_URI).toString()));
		if (playbackModeButton->isChecked()) {
			uniqueTable->scrollTo(index, QAbstractItemView::PositionAtCenter);
		} else {
			uniqueTable->scrollTo(index, QAbstractItemView::EnsureVisible);
		}
		// Clear highlight first
		if (_currentTrack) {
			_currentTrack->setData(false, Miam::DF_Highlighted);
		}
		_currentTrack = item;
		_currentTrack->setData(true, Miam::DF_Highlighted);
		return true;
	} else {
		return false;
	}
}

void UniqueLibrary::skipBackward()
{
	if (!_currentTrack) {
		return;
	}
	_mediaPlayer->blockSignals(true);

	if (playbackModeButton->isChecked()) {
		if (_randomHistoryList.isEmpty()) {
			return;
		} else {
			this->playSingleTrack(_randomHistoryList.takeLast());
		}
	} else {
		QModelIndex current = _proxy->mapFromSource(uniqueTable->model()->index(_currentTrack->row(), 1));
		int row = current.row();
		while (row >= 0) {
			QModelIndex previous = current.sibling(row - 1, 1);
			if (this->playSingleTrack(previous)) {
				uniqueTable->scrollTo(previous);
				break;
			} else {
				row--;
			}
		}
	}
	_mediaPlayer->blockSignals(false);
}

void UniqueLibrary::skipForward()
{
	_mediaPlayer->blockSignals(true);

	if (_currentTrack) {
		_currentTrack->setData(false, Miam::DF_Highlighted);

		// Append to random history the track the player is playing
		if (playbackModeButton->isChecked()) {
			_randomHistoryList.append(_proxy->mapFromSource(_currentTrack->index()));
		}
	}

	if (playbackModeButton->isChecked()) {
		int rows = uniqueTable->model()->rowCount();
		if (rows > 0) {
			int r = rand() % rows;
			QModelIndex idx = uniqueTable->model()->index(r, 1);
			while (uniqueTable->model()->itemFromIndex(idx)->type() != Miam::IT_Track) {
				idx = uniqueTable->model()->index(rand() % rows, 1);
			}
			QModelIndex next = _proxy->mapFromSource(idx);
			this->playSingleTrack(next);
		}
	} else {
		QModelIndex current;
		if (_currentTrack) {
			current = _proxy->mapFromSource(uniqueTable->model()->index(_currentTrack->row(), 1));
		} else {
			current = _proxy->index(0, 1);
		}
		int row = current.row();
		while (row < uniqueTable->model()->rowCount()) {
			QModelIndex next = current.sibling(row + 1, 1);
			if (this->playSingleTrack(next)) {
				break;
			} else {
				row++;
			}
		}
	}
	_mediaPlayer->blockSignals(false);
}

void UniqueLibrary::toggleShuffle()
{
	playbackModeButton->setChecked(playbackModeButton->isChecked());
	if (!playbackModeButton->isChecked()) {
		_randomHistoryList.clear();
	}
}
