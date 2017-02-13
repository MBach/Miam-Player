#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include <styling/paintablewidget.h>
#include <filehelper.h>
#include <musicsearchengine.h>
#include <settingsprivate.h>
#include "uniquelibraryitemdelegate.h"

#include <QLabel>
#include <QProgressBar>
#include <QScrollBar>
#include <QStandardItemModel>

#include <ctime>
#include <random>

#include <QtDebug>

UniqueLibrary::UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent)
	: AbstractView(new UniqueLibraryMediaPlayerControl(mediaPlayer, parent), parent)
	, _currentTrack(nullptr)
	, _randomHistoryList(new QModelIndexList())
{
	setupUi(this);
	playButton->setMediaPlayer(mediaPlayer);
	stopButton->setMediaPlayer(mediaPlayer);
	seekSlider->setMediaPlayer(mediaPlayer);
	playbackModeButton->setToggleShuffleOnly(true);

	setAttribute(Qt::WA_StaticContents);

	mediaPlayer->setPlaylist(nullptr);

	connect(mediaPlayer, &MediaPlayer::positionChanged, this, [=](qint64 pos, qint64) {
		if (_currentTrack) {
			uint p = pos / 1000;
			_currentTrack->setData(p, Miam::DF_CurrentPosition);
		}
	});

	connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
		mediaPlayer->blockSignals(true);
		mediaPlayer->setVolume((qreal)value / 100.0);
		mediaPlayer->blockSignals(false);
	});
	connect(mediaPlayer, &MediaPlayer::volumeChanged, this, [=](qreal v) {
		volumeSlider->setValue(v * 100);
	});

	auto settings = Settings::instance();
	volumeSlider->setValue(settings->volume() * 100);

	uniqueTable->setItemDelegate(new UniqueLibraryItemDelegate(uniqueTable));
	connect(uniqueTable, &TableView::sendToTagEditor, this, &UniqueLibrary::aboutToSendToTagEditor);
	_proxy = uniqueTable->model()->proxy();

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &SearchBar::aboutToStartSearch, this, [=](const QString &text) {
		//uniqueTable->model()->proxy()->findMusic(text);
		if (_currentTrack) {
			delete _currentTrack;
			_currentTrack = nullptr;
		}
		uniqueTable->model()->load(text);
		uniqueTable->adjust();

		uniqueTable->scrollToTop();
		uniqueTable->verticalScrollBar()->setValue(0);
	});
	connect(uniqueTable, &TableView::doubleClicked, this, [=](const QModelIndex &index) {
		this->play(index, QAbstractItemView::EnsureVisible);
	});

	connect(skipBackwardButton, &MediaButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipBackward);
	connect(seekBackwardButton, &MediaButton::clicked, mediaPlayer, &MediaPlayer::seekBackward);
	connect(playButton, &MediaButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::togglePlayback);
	connect(stopButton, &MediaButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::stop);
	connect(seekForwardButton, &MediaButton::clicked, mediaPlayer, &MediaPlayer::seekForward);
	connect(skipForwardButton, &MediaButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipForward);
	connect(playbackModeButton, &MediaButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::toggleShuffle);

	connect(mediaPlayer, &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State) {
		seekSlider->update();
	});

	connect(mediaPlayer, &MediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
		if (mediaPlayer->state() != QMediaPlayer::StoppedState && status == QMediaPlayer::EndOfMedia) {
			if (mediaPlayer->isStopAfterCurrent()) {
				mediaPlayer->stop();
				mediaPlayer->setStopAfterCurrent(false);
			} else {
				if (playbackModeButton->isChecked()) {
					_randomHistoryList->append(_proxy->mapFromSource(_currentTrack->index()));
				}
				_mediaPlayerControl->skipForward();
			}
			seekSlider->setValue(0);
		}
	});

	auto settingsPrivate = SettingsPrivate::instance();
	connect(settingsPrivate, &SettingsPrivate::languageAboutToChange, this, [=](const QString &newLanguage) {
		QApplication::removeTranslator(&translator);
		translator.load(":/translations/uniqueLibrary_" + newLanguage);
		QApplication::installTranslator(&translator);
	});

	// Init button theme
	auto buttons = this->findChildren<MediaButton*>();
	for (MediaButton *b : buttons) {
		b->setIconFromTheme(settings->theme());
		b->setVisible(settings->isMediaButtonVisible(b->objectName()));
	}

	//playbackModeButton->setChecked(settingsPrivate->value("uniqueLibraryIsInShuffleState").toBool());
	playbackModeButton->setChecked(_mediaPlayerControl->isInShuffleState());

	// Init language
	translator.load(":/translations/uniqueLibrary_" + settingsPrivate->language());
	QApplication::installTranslator(&translator);

	std::srand(std::time(nullptr));

	uniqueTable->setFocus();

	connect(qApp, &QApplication::aboutToQuit, this, [=]() {
		if (_currentTrack) {
			settingsPrivate->setValue("uniqueLibraryLastPlayed", _currentTrack->row());
		}
	});
}

UniqueLibrary::~UniqueLibrary()
{
	disconnect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::positionChanged, seekSlider, &SeekBar::setPosition);
	_mediaPlayerControl->mediaPlayer()->stop();
	if (_currentTrack) {
		qDebug() << _currentTrack << _currentTrack->row();
		//SettingsPrivate::instance()->setValue("uniqueLibraryLastPlayed", _currentTrack->row());
	}
	this->disconnect();
}

void UniqueLibrary::loadModel()
{
	uniqueTable->model()->load();
	uniqueTable->adjust();

	auto settingsPrivate = SettingsPrivate::instance();
	if (!settingsPrivate->value("uniqueLibraryLastPlayed").isNull()) {
		int track = settingsPrivate->value("uniqueLibraryLastPlayed").toInt();
		QModelIndex lastPlayed = uniqueTable->model()->index(track, 1);
		if (lastPlayed.isValid()) {
			QModelIndex p = uniqueTable->model()->proxy()->mapFromSource(lastPlayed);
			QStandardItem *trackItem = uniqueTable->model()->itemFromIndex(lastPlayed);
			if (p.isValid() && trackItem != nullptr) {
				_currentTrack = trackItem;
				uniqueTable->setCurrentIndex(p);
				uniqueTable->scrollTo(p, QAbstractItemView::PositionAtCenter);
			}
		}
	}
}

bool UniqueLibrary::viewProperty(Settings::ViewProperty vp) const
{
	switch (vp) {
	case Settings::VP_MediaControls:
	case Settings::VP_SearchArea:
	case Settings::VP_VolumeIndicatorToggled:
	case Settings::VP_HasAreaForRescan:
	case Settings::VP_CanSendTracksToEditor:
		return true;
	case Settings::VP_HasTracksToDisplay:
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

bool UniqueLibrary::play(const QModelIndex &index, QAbstractItemView::ScrollHint sh)
{
	QStandardItem *item = uniqueTable->model()->itemFromIndex(_proxy->mapToSource(index));
	if (item && item->type() == Miam::IT_Track) {
		seekSlider->setValue(0);
		_mediaPlayerControl->mediaPlayer()->playMediaContent(QUrl::fromLocalFile(index.data(Miam::DF_URI).toString()));
		if (playbackModeButton->isChecked()) {
			uniqueTable->scrollTo(index, sh);
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

bool UniqueLibrary::playSingleTrack(const QModelIndex &index)
{
	return this->play(index);
}

void UniqueLibrary::setMusicSearchEngine(MusicSearchEngine *musicSearchEngine)
{
	if (_currentTrack) {
		delete _currentTrack;
		_currentTrack = nullptr;
	}
	connect(musicSearchEngine, &MusicSearchEngine::aboutToSearch, this, [=]() {

		QVBoxLayout *vbox = new QVBoxLayout;
		vbox->setMargin(0);
		vbox->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

		PaintableWidget *paintable = new PaintableWidget(this);
		paintable->setObjectName("paintable");
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
		if (uniqueTable->layout()) {
			delete uniqueTable->layout();
		}
		if (PaintableWidget *w = findChild<PaintableWidget*>("paintable")) {
			w->deleteLater();
		}
	});
}

void UniqueLibrary::setViewProperty(Settings::ViewProperty vp, QVariant value)
{
	Q_UNUSED(value)
	switch (vp) {
	case Settings::VP_SearchArea:
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
