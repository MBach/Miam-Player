#include "viewplaylists.h"

#include <library/jumptowidget.h>
#include <styling/paintablewidget.h>
#include <libraryorderdialog.h>
#include <musicsearchengine.h>
#include <settingsprivate.h>
#include <settings.h>
#include "dialogs/playlistdialog.h"

#include <QFileDialog>
#include <QProgressBar>
#include <QStandardPaths>

ViewPlaylists::ViewPlaylists(MediaPlayer *mediaPlayer, QWidget *parent)
	: AbstractViewPlaylists(new ViewPlaylistsMediaPlayerControl(mediaPlayer, parent), parent)
	, _searchDialog(new SearchDialog(this))
	, _removeSelectedTracks(new QShortcut(this))
	, _showTabLibrary(new QShortcut(this))
	, _showTabFilesystem(new QShortcut(this))
	, _goToSearch(new QShortcut(this))
	, _sendToCurrentPlaylist(new QShortcut(this))
	, _sendToTagEditor(new QShortcut(this))
{
	this->setupUi(this);
	playButton->setMediaPlayer(mediaPlayer);
	stopButton->setMediaPlayer(mediaPlayer);
	playbackModeButton->setToggleShuffleOnly(false);

	paintableWidget->setFrameBorder(false, false, true, false);
	seekSlider->setMediaPlayer(mediaPlayer);

	// Init language before initiating tabPlaylists
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	translator.load(":/translations/tabPlaylists_" + settingsPrivate->language());
	QApplication::installTranslator(&translator);
	tabPlaylists->init(mediaPlayer);
	QMediaPlaylist::PlaybackMode mode = (QMediaPlaylist::PlaybackMode)settingsPrivate->value("lastActivePlaylistMode", 2).toInt();
	playbackModeButton->updateMode(mode);
	mediaPlayer->setPlaylist(tabPlaylists->currentPlayList()->mediaPlaylist());
	mediaPlayer->playlist()->setPlaybackMode(mode);

	widgetSearchBar->setFrameBorder(false, false, true, false);

	connect(mediaPlayer->playlist(), &QMediaPlaylist::playbackModeChanged, playbackModeButton, &PlaybackModeButton::updateMode);
	connect(tabPlaylists, &TabPlaylist::updatePlaybackModeButton, mediaPlayer->playlist(), &MediaPlaylist::setPlaybackMode);
	connect(tabPlaylists, &TabPlaylist::updatePlaybackModeButton, playbackModeButton, &PlaybackModeButton::updateMode);
	connect(playbackModeButton, &PlaybackModeButton::aboutToChangeCurrentPlaylistPlaybackMode, tabPlaylists, &TabPlaylist::changeCurrentPlaylistPlaybackMode);

	Settings *settings = Settings::instance();
	volumeSlider->setValue(settings->volume() * 100);

	// Buttons
	for (MediaButton *b : findChildren<MediaButton*>()) {
		if (!b) {
			continue;
		}
		b->setSize(settings->buttonsSize());
		b->setIconFromTheme(settings->theme());
		b->setVisible(settings->isMediaButtonVisible(b->objectName()));
	}

	searchBar->setFont(settingsPrivate->font(SettingsPrivate::FF_Library));

	/// XXX ?
	bool isEmpty = settingsPrivate->musicLocations().isEmpty();
	if (isEmpty) {
		widgetSearchBar->hide();
		changeHierarchyButton->hide();
		libraryHeader->hide();
		library->hide();
	}

	leftTabs->setCurrentIndex(settingsPrivate->value("leftTabsIndex").toInt());

	// Core
	connect(mediaPlayer, &MediaPlayer::stateChanged, this, &ViewPlaylists::mediaPlayerStateHasChanged);

	// Main Splitter
	connect(splitter, &QSplitter::splitterMoved, _searchDialog, &SearchDialog::moveSearchDialog);

	connect(searchBar, &LibraryFilterLineEdit::aboutToStartSearch, library->model()->proxy(), &LibraryFilterProxyModel::findMusic);
	connect(settingsPrivate, &SettingsPrivate::librarySearchModeHasChanged, this, [=]() {
		QString text;
		searchBar->setText(text);
		library->model()->proxy()->findMusic(text);
	});

	// Media buttons
	connect(skipBackwardButton, &QAbstractButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipBackward);
	connect(seekBackwardButton, &QAbstractButton::clicked, mediaPlayer, &MediaPlayer::seekBackward);
	connect(playButton, &QAbstractButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::togglePlayback);
	connect(stopButton, &QAbstractButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::stop);
	connect(seekForwardButton, &QAbstractButton::clicked, mediaPlayer, &MediaPlayer::seekForward);
	connect(skipForwardButton, &QAbstractButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipForward);

	connect(filesystem, &FileSystemTreeView::folderChanged, addressBar, &AddressBar::init);
	connect(addressBar, &AddressBar::aboutToChangePath, filesystem, &FileSystemTreeView::reloadWithNewPath);
	addressBar->init(settingsPrivate->defaultLocationFileExplorer());

	connect(libraryHeader, &LibraryHeader::aboutToChangeSortOrder, library, &LibraryTreeView::changeSortOrder);

	// Factorize code with lambda slot connected to replicated signal
	auto reloadLibrary = [this]() {
		searchBar->setText(QString());
		_searchDialog->clear();
		library->scrollToTop();
		qDebug() << "ViewPlaylists -> lambda reload";
		library->model()->load();
		this->update();
	};

	connect(libraryHeader, &LibraryHeader::aboutToChangeHierarchyOrder, reloadLibrary);
	connect(changeHierarchyButton, &QPushButton::toggled, this, [=]() {
		LibraryOrderDialog *libraryOrderDialog = new LibraryOrderDialog(this);
		libraryOrderDialog->move(libraryOrderDialog->mapFromGlobal(QCursor::pos()));
		libraryOrderDialog->show();
		connect(libraryOrderDialog, &LibraryOrderDialog::aboutToChangeHierarchyOrder, reloadLibrary);
	});

	for (TreeView *tab : this->findChildren<TreeView*>()) {
		connect(tab, &TreeView::aboutToInsertToPlaylist, tabPlaylists, &TabPlaylist::insertItemsToPlaylist);
		connect(tab, &TreeView::aboutToSendToTagEditor, this, &AbstractView::aboutToSendToTagEditor);
	}

	// Send one folder to the music locations
	connect(filesystem, &FileSystemTreeView::aboutToAddMusicLocations, settingsPrivate, &SettingsPrivate::addMusicLocations);

	// Send music to the tag editor
	connect(tabPlaylists, &TabPlaylist::aboutToSendToTagEditor, this, &AbstractView::aboutToSendToTagEditor);

	// Sliders
	connect(mediaPlayer, &MediaPlayer::positionChanged, timeLabel, &TimeLabel::setTime);

	// Volume bar
	connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
		mediaPlayer->setVolume((qreal)value / 100.0);
	});
	connect(mediaPlayer, &MediaPlayer::volumeChanged, this, [=](qreal v) {
		qDebug() << Q_FUNC_INFO;
		volumeSlider->blockSignals(true);
		volumeSlider->setValue(v * 100);
		volumeSlider->blockSignals(false);
	});

	connect(qApp, &QApplication::aboutToQuit, this, [=] {
		settingsPrivate->setValue("leftTabsIndex", leftTabs->currentIndex());
		settingsPrivate->setLastActivePlaylistGeometry(tabPlaylists->currentPlayList()->horizontalHeader()->saveState());
		settingsPrivate->sync();
	});

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &LibraryFilterLineEdit::aboutToStartSearch, this, [=](const QString &text) {
		if (settingsPrivate->isExtendedSearchVisible()) {
			if (text.isEmpty()) {
				_searchDialog->clear();
			} else {
				_searchDialog->setSearchExpression(text);
				_searchDialog->moveSearchDialog(0, 0);
				_searchDialog->show();
				_searchDialog->raise();
			}
		}
	});

	connect(this, &AbstractView::modelReloadRequested, this, [=]() {
		qDebug() << "AbstractView::modelReloadRequested";
		library->model()->load();
		for (Playlist *p : tabPlaylists->playlists()) {
			p->model()->reload();
		}
	});

	connect(settingsPrivate, &SettingsPrivate::languageAboutToChange, this, [=](const QString &newLanguage) {
		QApplication::removeTranslator(&translator);
		translator.load(":/translations/tabPlaylists_" + newLanguage);
		QApplication::installTranslator(&translator);
	});

	connect(settings, &Settings::viewPropertyChanged, this, &ViewPlaylists::setViewProperty);

	// Bind shortcuts
	_removeSelectedTracks->setContext(Qt::WidgetWithChildrenShortcut);
	_removeSelectedTracks->setObjectName("removeSelectedTracks");
	_showTabLibrary->setObjectName("showTabLibrary");
	_showTabFilesystem->setObjectName("showTabFilesystem");
	_goToSearch->setObjectName("search");
	_sendToCurrentPlaylist->setObjectName("sendToCurrentPlaylist");
	_sendToTagEditor->setObjectName("sendToTagEditor");

	QMapIterator<QString, QVariant> it(settings->shortcuts());
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().value<QKeySequence>());
	}

	connect(_removeSelectedTracks, &QShortcut::activated, tabPlaylists->currentPlayList(), &Playlist::removeSelectedTracks);
	connect(_showTabLibrary, &QShortcut::activated, this, [=]() { leftTabs->setCurrentIndex(0);	});
	connect(_showTabFilesystem, &QShortcut::activated, this, [=]() { leftTabs->setCurrentIndex(1); });
	connect(_goToSearch, &QShortcut::activated, searchBar, static_cast<void (QWidget::*)(void)>(&QWidget::setFocus));
	connect(_sendToCurrentPlaylist, &QShortcut::activated, this, [=]() {
		if (leftTabs->currentIndex() == 0) {
			library->appendToPlaylist();
		} else {
			filesystem->appendToPlaylist();
		}
	});
	connect(_sendToTagEditor, &QShortcut::activated, this, [=]() {
		if (leftTabs->currentIndex() == 0) {
			library->openTagEditor();
		} else {
			filesystem->openTagEditor();
		}
	});
}

ViewPlaylists::~ViewPlaylists()
{
	this->disconnect();
	if (_searchDialog) {
		delete _searchDialog;
		_searchDialog = nullptr;
	}
}

void ViewPlaylists::addToPlaylist(const QList<QUrl> &tracks)
{
	tabPlaylists->insertItemsToPlaylist(-1, tracks);
}

/** Bind a new shortcut to a specifc action in this view (like search for example). */
void ViewPlaylists::bindShortcut(const QString &objectName, const QKeySequence &keySequence)
{
	for (QShortcut *s : this->findChildren<QShortcut*>(objectName)) {
		if (s) {
			s->setKey(keySequence);
			break;
		}
	}
}

QPair<QString, QObjectList> ViewPlaylists::extensionPoints() const
{
	QObjectList libraryObjectList;
	libraryObjectList << library << library->properties << _mediaPlayerControl;
	return qMakePair(library->metaObject()->className(), libraryObjectList);
}

void ViewPlaylists::loadModel()
{
	emit modelReloadRequested();
}

int ViewPlaylists::selectedTracksInCurrentPlaylist() const
{
	return tabPlaylists->currentPlayList()->selectionModel()->selectedRows().count();
}

void ViewPlaylists::setMusicSearchEngine(MusicSearchEngine *musicSearchEngine)
{
	connect(musicSearchEngine, &MusicSearchEngine::aboutToSearch, this, [=]() {
		QVBoxLayout *vbox = new QVBoxLayout;
		vbox->setMargin(0);

		PaintableWidget *paintable = new PaintableWidget(this);
		paintable->setObjectName("paintable");
		paintable->setHalfTopBorder(false);
		paintable->setFrameBorder(false, true, true, false);
		QVBoxLayout *vbox2 = new QVBoxLayout;
		vbox2->addWidget(new QLabel(tr("Your library is updating..."), paintable));
		vbox2->addWidget(new QProgressBar(paintable));
		paintable->setLayout(vbox2);

		vbox->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
		vbox->addWidget(paintable);
		vbox->setObjectName("YO");
		library->setLayout(vbox);
	});

	connect(musicSearchEngine, &MusicSearchEngine::progressChanged, this, [=](int p) {
		if (QProgressBar *progress = this->findChild<QProgressBar*>()) {
			progress->setValue(p);
		}
	});

	connect(musicSearchEngine, &MusicSearchEngine::searchHasEnded, this, [=]() {
		if (library->layout()) {
			delete library->layout();
		}
		foreach (QWidget *w, library->findChildren<QWidget*>()) {
			if (w && w->objectName() == "paintable") {
				delete w;
			}
		}
	});
}

bool ViewPlaylists::viewProperty(Settings::ViewProperty vp) const
{
	switch (vp) {
	case Settings::VP_MediaControls:
	case Settings::VP_SearchArea:
	case Settings::VP_PlaylistFeature:
	case Settings::VP_HasAreaForRescan:
	case Settings::VP_FileExplorerFeature:
	case Settings::VP_VolumeIndicatorToggled:
	case Settings::VP_CanSendTracksToEditor:
		return true;
	case Settings::VP_HasTracksToDisplay:
		return library->model()->rowCount() > 0;
	default:
		return AbstractView::viewProperty(vp);
	}
}

void ViewPlaylists::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
	} else {
		AbstractViewPlaylists::changeEvent(event);
	}
}

/** Open a new Dialog where one can add a folder to current playlist. */
void ViewPlaylists::openFolder(const QString &dir) const
{
	Settings::instance()->setValue("lastOpenedLocation", dir);
	QDirIterator it(dir, QDirIterator::Subdirectories);
	QStringList suffixes = FileHelper::suffixes(FileHelper::ET_All, false);
	QList<QUrl> localTracks;
	while (it.hasNext()) {
		it.next();
		if (suffixes.contains(it.fileInfo().suffix())) {
			localTracks << QUrl::fromLocalFile(it.filePath());
		}
	}
	if (Miam::showWarning(tr("playlist"), localTracks.count()) == QMessageBox::Ok) {
		tabPlaylists->insertItemsToPlaylist(-1, localTracks);
	}
}

QList<MediaPlaylist *> ViewPlaylists::playlists() const
{
	QList<MediaPlaylist*> result;
	for (Playlist *p : tabPlaylists->playlists()) {
		result.append(p->mediaPlaylist());
	}
	return result;
}

void ViewPlaylists::saveCurrentPlaylists()
{
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	QList<uint> list = settingsPrivate->lastPlaylistSession();
	list.clear();
	for (int i = 0; i < tabPlaylists->count(); i++) {
		Playlist *p = tabPlaylists->playlist(i);
		bool isOverwritting = p->id() != 0;
		uint id = tabPlaylists->playlistManager()->savePlaylist(p, isOverwritting, true);
		if (id != 0) {
			list.append(id);
		}
	}

	int idx = tabPlaylists->currentIndex();
	Playlist *p = tabPlaylists->playlist(idx);
	int m = p->mediaPlaylist()->playbackMode();

	settingsPrivate->setLastPlaylistSession(list);
	settingsPrivate->setValue("lastActiveTab", idx);
	settingsPrivate->setValue("lastActivePlaylistMode", m);
}

void ViewPlaylists::addExtFolders(const QList<QDir> &folders)
{
	bool isEmpty = tabPlaylists->currentPlayList()->mediaPlaylist()->isEmpty();

	QStringList tracks;
	for (QDir folder : folders) {
		QDirIterator it(folder.absolutePath(), FileHelper::suffixes(FileHelper::ET_Standard, true), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isFile()) {
				tracks << it.fileInfo().absoluteFilePath();
			}
		}
	}
	tracks.sort(Qt::CaseInsensitive);
	QList<QUrl> urls;
	for (QString t : tracks) {
		urls << QUrl::fromLocalFile(t);
	}
	tabPlaylists->insertItemsToPlaylist(-1, urls);

	// Automatically plays the first track
	if (isEmpty) {
		_mediaPlayerControl->mediaPlayer()->setPlaylist(tabPlaylists->currentPlayList()->mediaPlaylist());
		_mediaPlayerControl->mediaPlayer()->play();
	}
}

void ViewPlaylists::addPlaylist()
{
	tabPlaylists->addPlaylist();
}

void ViewPlaylists::initFileExplorer(const QDir &dir)
{
	addressBar->init(dir);
}

void ViewPlaylists::moveTracksDown()
{
	if (tabPlaylists->currentPlayList()) {
		tabPlaylists->currentPlayList()->moveTracksDown();
	}
}

void ViewPlaylists::moveTracksUp()
{
	if (tabPlaylists->currentPlayList()) {
		tabPlaylists->currentPlayList()->moveTracksUp();
	}
}

void ViewPlaylists::openFiles()
{
	QString audioFiles = tr("Audio files");
	Settings *settings = Settings::instance();
	QString lastOpenedLocation;
	QString defaultMusicLocation = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
	if (settings->value("lastOpenedLocation").toString().isEmpty()) {
		lastOpenedLocation = defaultMusicLocation;
	} else {
		lastOpenedLocation = settings->value("lastOpenedLocation").toString();
	}

	audioFiles.append(" (" + FileHelper::suffixes(FileHelper::ET_Standard, true).join(" ") + ");;");
	audioFiles.append("Game Music Emu (" + FileHelper::suffixes(FileHelper::ET_GameMusicEmu, true).join(" ") + ");;");
	audioFiles.append("Playlists (" + FileHelper::suffixes(FileHelper::ET_Playlist, true).join(" ") + ");;");
	audioFiles.append(tr("Every file type (*)"));

	QStringList files = QFileDialog::getOpenFileNames(this, tr("Choose some files to open"), lastOpenedLocation,
													  audioFiles);
	if (files.isEmpty()) {
		settings->setValue("lastOpenedLocation", defaultMusicLocation);
	} else {
		QFileInfo fileInfo(files.first());
		settings->setValue("lastOpenedLocation", fileInfo.absolutePath());
		QList<QUrl> tracks;
		QStringList playlists;
		for (QString file : files) {

			QFileInfo fi(file);
			if (FileHelper::suffixes(FileHelper::ET_Playlist).contains(fi.suffix())) {
				playlists << file;
			} else {
				tracks << QUrl::fromLocalFile(file);
			}
		}
		if (!tracks.isEmpty()) {
			tabPlaylists->insertItemsToPlaylist(-1, tracks);
		}
		// Open every playlist file in new tab
		if (!playlists.isEmpty()) {
			this->openPlaylists(playlists);

		}
	}
}

void ViewPlaylists::openFolderPopup()
{
	Settings *settings = Settings::instance();
	QString lastOpenedLocation;
	QString defaultMusicLocation = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
	if (settings->value("lastOpenedLocation").toString().isEmpty()) {
		lastOpenedLocation = defaultMusicLocation;
	} else {
		lastOpenedLocation = settings->value("lastOpenedLocation").toString();
	}
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a folder to open"), lastOpenedLocation);
	if (dir.isEmpty()) {
		settings->setValue("lastOpenedLocation", defaultMusicLocation);
	} else {
		this->openFolder(dir);
	}
}

void ViewPlaylists::openPlaylistManager()
{
	PlaylistDialog *playlistDialog = new PlaylistDialog(this);
	playlistDialog->setPlaylists(tabPlaylists->playlists());
	connect(playlistDialog, &PlaylistDialog::aboutToLoadPlaylist, tabPlaylists, &TabPlaylist::loadPlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToDeletePlaylist, tabPlaylists, &TabPlaylist::deletePlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToRenamePlaylist, tabPlaylists, &TabPlaylist::renamePlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToRenameTab, tabPlaylists, &TabPlaylist::renameTab);
	connect(playlistDialog, &PlaylistDialog::aboutToSavePlaylist, tabPlaylists, &TabPlaylist::savePlaylist);
	playlistDialog->exec();
}

void ViewPlaylists::openPlaylists(const QStringList &playlists)
{
	for (QString playlistFile : playlists) {
		QFileInfo fi(playlistFile);
		Playlist *p = nullptr;
		if (tabPlaylists->currentPlayList()->mediaPlaylist()->isEmpty()) {
			p = tabPlaylists->currentPlayList();
		} else {
			p = tabPlaylists->addPlaylist();
		}
		if (tabPlaylists->playlistManager()->loadPlaylist(p, fi)) {
			tabPlaylists->renamePlaylist(p);
		}
	}
}

void ViewPlaylists::removeCurrentPlaylist()
{
	tabPlaylists->removeCurrentPlaylist();
}

void ViewPlaylists::removeSelectedTracks()
{
	if (tabPlaylists->currentPlayList()) {
		tabPlaylists->currentPlayList()->removeSelectedTracks();
	}
}

void ViewPlaylists::setViewProperty(Settings::ViewProperty vp, QVariant value)
{
	switch (vp) {
	case Settings::VP_MediaControls:
		for (MediaButton *b : findChildren<MediaButton*>()) {
			b->setSize(value.toInt());
		}
		break;
	case Settings::VP_SearchArea:
		if (_searchDialog) {
			_searchDialog->clear();
		}
		searchBar->clear();
		break;
	default:
		break;
	}
}

void ViewPlaylists::volumeSliderDecrease()
{
	volumeSlider->setValue(volumeSlider->value() - 5);
}

void ViewPlaylists::volumeSliderIncrease()
{
	volumeSlider->setValue(volumeSlider->value() + 5);
}

void ViewPlaylists::mediaPlayerStateHasChanged(QMediaPlayer::State state)
{
	if (state == QMediaPlayer::PlayingState) {
		seekSlider->setEnabled(true);
	} else {
		seekSlider->setDisabled(state == QMediaPlayer::StoppedState);
		if (state == QMediaPlayer::StoppedState) {
			seekSlider->setValue(0);
			timeLabel->setTime(0, 0);
		}
	}
	// Remove bold font when player has stopped
	tabPlaylists->currentPlayList()->viewport()->update();
	seekSlider->update();
}
