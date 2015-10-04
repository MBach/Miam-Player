#include "mainwindow.h"

#include <musicsearchengine.h>
#include <quickstart.h>
#include <settings.h>
#include <settingsprivate.h>

#include "dialogs/customizethemedialog.h"
#include "dialogs/dragdropdialog.h"
#include "dialogs/equalizerdalog.h"
#include "playlists/playlist.h"
#include "pluginmanager.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtMultimedia/QAudioDeviceInfo>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, _uniqueLibrary(new UniqueLibrary(this))
	, _mediaPlayer(new MediaPlayer(this))
	, searchDialog(new SearchDialog(this))
	, _pluginManager(new PluginManager(this))
{
	setupUi(this);
	widgetSearchBar->setFrameBorder(false, false, true, false);
	seekSlider->setMediaPlayer(_mediaPlayer);

	this->setAcceptDrops(true);
#ifndef Q_OS_MAC
	this->setWindowIcon(QIcon(":/icons/mp_win32"));
#endif

	// Special behaviour for media buttons
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << stopButton
				 << seekForwardButton << skipForwardButton << playbackModeButton;
	for (MediaButton *button : mediaButtons) {
		button->setMediaPlayer(_mediaPlayer);
	}
	tabPlaylists->setMainWindow(this);

	/// XXX
	stackedWidget->addWidget(_uniqueLibrary);
	_uniqueLibrary->hide();

	this->loadThemeAndSettings();

	// Instantiate dialogs
	_playbackModeWidgetFactory = new PlaybackModeWidgetFactory(this, playbackModeButton, tabPlaylists);

	this->installEventFilter(this);
}

MediaPlayer *MainWindow::mediaPlayer() const
{
	return _mediaPlayer;
}

void MainWindow::activateLastView()
{
	// Find the last active view and connect database to it
	QString viewName = Settings::instance()->lastActiveView();
	for (QAction *actionView : menuView->actions()) {
		if (actionView->objectName() == viewName) {
			this->restoreGeometry(SettingsPrivate::instance()->value("mainWindowGeometry").toByteArray());
			actionView->trigger();
			break;
		}
	}
}

void MainWindow::dispatchDrop(QDropEvent *event)
{
	/** Popup shown to one when tracks are dropped from another application to MiamPlayer. */
	DragDropDialog *dragDropDialog = new DragDropDialog;

	SettingsPrivate *settings = SettingsPrivate::instance();

	// Drag & Drop actions
	connect(dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToLibrary, settings, &SettingsPrivate::addMusicLocations);
	connect(dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToPlaylist, tabPlaylists, &TabPlaylist::addExtFolders);

	bool onlyFiles = dragDropDialog->setMimeData(event->mimeData());
	if (onlyFiles) {
		QStringList tracks;
		for (QString file : dragDropDialog->externalLocations) {
			tracks << "file://" + file;
		}
		tracks.sort(Qt::CaseInsensitive);
		tabPlaylists->insertItemsToPlaylist(-1, tracks);
	} else {
		QList<QDir> dirs;
		for (QString location : dragDropDialog->externalLocations) {
			dirs << location;
		}
		switch (SettingsPrivate::instance()->dragDropAction()) {
		case SettingsPrivate::DD_OpenPopup:
			dragDropDialog->show();
			dragDropDialog->raise();
			dragDropDialog->activateWindow();
			break;
		case SettingsPrivate::DD_AddToLibrary:
			settings->addMusicLocations(dirs);
			break;
		case SettingsPrivate::DD_AddToPlaylist:
			tabPlaylists->addExtFolders(dirs);
			break;
		}
	}
}

void MainWindow::init()
{
	// Load playlists at startup if any, otherwise just add an empty one
	this->setupActions();

	auto settingsPrivate = SettingsPrivate::instance();

	// Init shortcuts
	QMapIterator<QString, QVariant> it(settingsPrivate->shortcuts());
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().value<QKeySequence>());
	}

	bool isEmpty = settingsPrivate->musicLocations().isEmpty();
	library->setVisible(!isEmpty);
	libraryHeader->setVisible(!isEmpty);
	changeHierarchyButton->setVisible(!isEmpty);

	actionScanLibrary->setDisabled(isEmpty);
	widgetSearchBar->setVisible(!isEmpty);
	this->showTabPlaylists();
	if (isEmpty) {
		QuickStart *quickStart = new QuickStart(this);
		quickStart->searchMultimediaFiles();
	}

	Settings *settings = Settings::instance();
	leftTabs->setCurrentIndex(settings->value("leftTabsIndex").toInt());

	tabPlaylists->init(_mediaPlayer);
	_playbackModeWidgetFactory->update();
}

/** Plugins. */
void MainWindow::loadPlugins()
{
	QObjectList libraryObjectList;
	libraryObjectList << library << library->properties;

	QObjectList tagEditorObjectList;
	tagEditorObjectList << tagEditor->albumCover->contextMenu() << tagEditor->extensiblePushButtonArea << tagEditor->extensibleWidgetArea << tagEditor->tagEditorWidget << tagEditor;

	_pluginManager->registerExtensionPoint(library->metaObject()->className(), libraryObjectList);
	_pluginManager->registerExtensionPoint(tagEditor->metaObject()->className(), tagEditorObjectList);
	_pluginManager->init();
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	SqlDatabase *db = SqlDatabase::instance();
	connect(db, &SqlDatabase::aboutToLoad, libraryHeader, &LibraryHeader::resetSortOrder);

	// Adds a group where view mode are mutually exclusive
	QActionGroup *viewModeGroup = new QActionGroup(this);
	actionViewPlaylists->setActionGroup(viewModeGroup);
	actionViewUniqueLibrary->setActionGroup(viewModeGroup);
	actionViewTagEditor->setActionGroup(viewModeGroup);

	connect(actionViewPlaylists, &QAction::triggered, this, [=]() {
		stackedWidget->setCurrentIndex(0);
		stackedWidgetRight->setVisible(true);
		stackedWidgetRight->setCurrentIndex(0);
		Settings::instance()->setLastActiveView(actionViewPlaylists->objectName());
		library->createConnectionsToDB();
	});
	connect(actionViewUniqueLibrary, &QAction::triggered, this, [=]() {
		stackedWidgetRight->setVisible(false);
		stackedWidget->setCurrentIndex(1);
		Settings::instance()->setLastActiveView(actionViewUniqueLibrary->objectName());
		_uniqueLibrary->library->createConnectionsToDB();
	});
	connect(actionViewTagEditor, &QAction::triggered, this, [=]() {
		stackedWidget->setCurrentIndex(0);
		stackedWidgetRight->setVisible(true);
		stackedWidgetRight->setCurrentIndex(1);
		actionViewTagEditor->setChecked(true);
		Settings::instance()->setLastActiveView(actionViewTagEditor->objectName());
		library->createConnectionsToDB();
	});

	QActionGroup *actionPlaybackGroup = new QActionGroup(this);
	for (QAction *actionPlayBack : findChildren<QAction*>(QRegExp("actionPlayback*", Qt::CaseSensitive, QRegExp::Wildcard))) {
		actionPlaybackGroup->addAction(actionPlayBack);
		connect(actionPlayBack, &QAction::triggered, this, [=]() {
			const QMetaObject &mo = QMediaPlaylist::staticMetaObject;
			QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PlaybackMode"));
			QString enu = actionPlayBack->property("PlaybackMode").toString();
			_playbackModeWidgetFactory->setPlaybackMode((QMediaPlaylist::PlaybackMode)metaEnum.keyToValue(enu.toStdString().data()));
		});
	}

	// Link user interface
	// Actions from the menu
	connect(actionOpenFiles, &QAction::triggered, this, &MainWindow::openFiles);
	connect(actionOpenFolder, &QAction::triggered, this, &MainWindow::openFolderPopup);
	connect(actionExit, &QAction::triggered, this, [=]() {
		QCloseEvent event;
		this->closeEvent(&event);
	});
	connect(actionAddPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::addPlaylist);
	connect(actionDeleteCurrentPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::removeCurrentPlaylist);
	connect(actionShowCustomize, &QAction::triggered, this, [=]() {
		CustomizeThemeDialog *customizeThemeDialog = new CustomizeThemeDialog(this);
		customizeThemeDialog->loadTheme();
		customizeThemeDialog->exec();
	});

	connect(actionShowOptions, &QAction::triggered, this, &MainWindow::createCustomizeOptionsDialog);
	connect(actionAboutQt, &QAction::triggered, &QApplication::aboutQt);
	connect(actionHideMenuBar, &QAction::triggered, this, &MainWindow::toggleMenuBar);
	connect(actionScanLibrary, &QAction::triggered, this, [=]() {
		searchBar->clear();
		db->rebuild();
	});
	connect(actionShowHelp, &QAction::triggered, this, [=]() {
		QDesktopServices::openUrl(QUrl("http://miam-player.org/wiki/index.php"));
	});

	// Load music
	auto settings = SettingsPrivate::instance();
	connect(settings, &SettingsPrivate::musicLocationsHaveChanged, [=](const QStringList &oldLocations, const QStringList &newLocations) {
		qDebug() << Q_FUNC_INFO << oldLocations << newLocations;
		bool libraryIsEmpty = newLocations.isEmpty();
		library->setVisible(!libraryIsEmpty);
		libraryHeader->setVisible(!libraryIsEmpty);
		changeHierarchyButton->setVisible(!libraryIsEmpty);
		actionScanLibrary->setDisabled(libraryIsEmpty);
		widgetSearchBar->setVisible(!libraryIsEmpty);

		auto db = SqlDatabase::instance();
		if (libraryIsEmpty) {
			leftTabs->setCurrentIndex(0);
			db->rebuild(oldLocations, QStringList());
			QuickStart *quickStart = new QuickStart(this);
			quickStart->searchMultimediaFiles();
		} else {
			db->rebuild(oldLocations, newLocations);
		}
	});

	for (TreeView *tab : this->findChildren<TreeView*>()) {
		connect(tab, &TreeView::aboutToInsertToPlaylist, tabPlaylists, &TabPlaylist::insertItemsToPlaylist);
		connect(tab, &TreeView::sendToTagEditor, this, [=](const QModelIndexList , const QStringList &tracks) {
			this->showTagEditor();
			tagEditor->addItemsToEditor(tracks);
		});
	}

	// Send one folder to the music locations
	connect(filesystem, &FileSystemTreeView::aboutToAddMusicLocations, settings, &SettingsPrivate::addMusicLocations);

	// Send music to the tag editor
	connect(tagEditor, &TagEditor::aboutToCloseTagEditor, this, &MainWindow::showTabPlaylists);
	connect(tabPlaylists, &TabPlaylist::aboutToSendToTagEditor, [=](const QList<QUrl> &tracks) {
		this->showTagEditor();
		tagEditor->addItemsToEditor(tracks);
	});

	// Media buttons and their shortcuts
	connect(menuPlayback, &QMenu::aboutToShow, this, [=]() {
		bool isPlaying = (_mediaPlayer->state() == QMediaPlayer::PlayingState || _mediaPlayer->state() == QMediaPlayer::PausedState);
		actionSeekBackward->setEnabled(isPlaying);
		actionStop->setEnabled(isPlaying);
		actionStopAfterCurrent->setEnabled(isPlaying);
		actionStopAfterCurrent->setChecked(_mediaPlayer->isStopAfterCurrent());
		actionSeekForward->setEnabled(isPlaying);

		bool notEmpty = _mediaPlayer->playlist() && !_mediaPlayer->playlist()->isEmpty();
		actionSkipBackward->setEnabled(notEmpty);
		actionPlay->setEnabled(notEmpty);
		actionSkipForward->setEnabled(notEmpty);
	});
	connect(actionSkipBackward, &QAction::triggered, _mediaPlayer, &MediaPlayer::skipBackward);
	connect(skipBackwardButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::skipBackward);
	connect(actionSeekBackward, &QAction::triggered, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(seekBackwardButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(actionPlay, &QAction::triggered, _mediaPlayer, &MediaPlayer::play);

	connect(playButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::play);
	connect(actionStop, &QAction::triggered, _mediaPlayer, &MediaPlayer::stop);
	connect(actionStopAfterCurrent, &QAction::triggered, _mediaPlayer, &MediaPlayer::stopAfterCurrent);
	connect(stopButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::stop);
	connect(actionSeekForward, &QAction::triggered, _mediaPlayer, &MediaPlayer::seekForward);
	connect(seekForwardButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::seekForward);
	connect(actionSkipForward, &QAction::triggered, _mediaPlayer, &MediaPlayer::skipForward);
	connect(skipForwardButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::skipForward);
	connect(playbackModeButton, &MediaButton::mediaButtonChanged, _playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);

	connect(actionShowEqualizer, &QAction::triggered, this, [=]() {
		EqualizerDialog *equalizerDialog = new EqualizerDialog(_mediaPlayer, this);
		equalizerDialog->show();
		equalizerDialog->activateWindow();
	});

	// Sliders
	connect(_mediaPlayer, &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			seekSlider->setValue(1000 * pos / duration);
			timeLabel->setTime(pos, duration);
		}
	});

	// Volume bar
	connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
		_mediaPlayer->setVolume((qreal)value / 100.0);
	});
	volumeSlider->setValue(Settings::instance()->volume() * 100);

	connect(library, &QTreeView::doubleClicked, [=] (const QModelIndex &) {
		library->appendToPlaylist();
	});

	// Main Splitter
	connect(splitter, &QSplitter::splitterMoved, searchDialog, &SearchDialog::moveSearchDialog);

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, &LibraryFilterLineEdit::aboutToStartSearch, this, [=](const QString &text) {
		if (settings->isExtendedSearchVisible()) {
			if (text.isEmpty()) {
				searchDialog->clear();
			} else {
				searchDialog->setSearchExpression(text);
				searchDialog->moveSearchDialog(0, 0);
				searchDialog->show();
				searchDialog->raise();
			}
		}
	});

	connect(searchBar, &LibraryFilterLineEdit::aboutToStartSearch, library, &LibraryTreeView::findMusic);
	connect(settings, &SettingsPrivate::librarySearchModeHasChanged, this, [=]() {
		QString text;
		searchBar->setText(text);
		library->findMusic(text);
	});

	// Core
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &MainWindow::mediaPlayerStateHasChanged);

	// Playback
	connect(tabPlaylists, &TabPlaylist::updatePlaybackModeButton, _playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);
	connect(actionRemoveSelectedTracks, &QAction::triggered, this, [=]() {
		if (tabPlaylists->currentPlayList()) {
			tabPlaylists->currentPlayList()->removeSelectedTracks();
		}
	});
	connect(actionMoveTracksUp, &QAction::triggered, tabPlaylists, &TabPlaylist::moveTracksUp);
	connect(actionMoveTracksDown, &QAction::triggered, tabPlaylists, &TabPlaylist::moveTracksDown);
	connect(actionOpenPlaylistManager, &QAction::triggered, this, &MainWindow::openPlaylistManager);
	connect(actionMute, &QAction::triggered, _mediaPlayer, &MediaPlayer::toggleMute);
	connect(actionIncreaseVolume, &QAction::triggered, this, [=]() {
		volumeSlider->setValue(volumeSlider->value() + 5);
	});
	connect(actionDecreaseVolume, &QAction::triggered, this, [=]() {
		volumeSlider->setValue(volumeSlider->value() - 5);
	});

	connect(filesystem, &FileSystemTreeView::folderChanged, addressBar, &AddressBar::init);
	connect(addressBar, &AddressBar::aboutToChangePath, filesystem, &FileSystemTreeView::reloadWithNewPath);

	// Playback modes
	connect(playbackModeButton, &QPushButton::clicked, _playbackModeWidgetFactory, &PlaybackModeWidgetFactory::togglePlaybackModes);

	connect(menuPlayback, &QMenu::aboutToShow, this, [=](){
		QMediaPlaylist::PlaybackMode mode = tabPlaylists->currentPlayList()->mediaPlaylist()->playbackMode();
		const QMetaObject &mo = QMediaPlaylist::staticMetaObject;
		QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PlaybackMode"));
		QAction *action = findChild<QAction*>(QString("actionPlayback").append(metaEnum.valueToKey(mode)));
		action->setChecked(true);
	});

	// Lambda function to reduce duplicate code
	auto updateActions = [this] (bool b) {
		actionRemoveSelectedTracks->setEnabled(b);
		actionMoveTracksUp->setEnabled(b);
		actionMoveTracksDown->setEnabled(b);
	};

	connect(menuPlaylist, &QMenu::aboutToShow, this, [=]() {
		bool b = tabPlaylists->currentPlayList()->selectionModel()->hasSelection();
		updateActions(b);
		if (b) {
			int selectedRows = tabPlaylists->currentPlayList()->selectionModel()->selectedRows().count();
			actionRemoveSelectedTracks->setText(tr("&Remove selected tracks", "Number of tracks to remove", selectedRows));
			actionMoveTracksUp->setText(tr("Move selected tracks &up", "Move upward", selectedRows));
			actionMoveTracksDown->setText(tr("Move selected tracks &down", "Move downward", selectedRows));
		}
	});
	connect(tabPlaylists, &TabPlaylist::selectionChanged, this, [=](bool isEmpty) {
		updateActions(!isEmpty);
	});

	connect(libraryHeader, &LibraryHeader::aboutToChangeSortOrder, library, &LibraryTreeView::changeSortOrder);
	connect(libraryHeader, &LibraryHeader::aboutToChangeHierarchyOrder, library, &LibraryTreeView::changeHierarchyOrder);
	connect(libraryHeader, &LibraryHeader::aboutToChangeHierarchyOrder, this, static_cast<void (MainWindow::*)(void)>(&MainWindow::update));
	connect(changeHierarchyButton, &QPushButton::toggled, libraryHeader, &LibraryHeader::showDialog);

	connect(qApp, &QApplication::aboutToQuit, this, [=] {
		settings->setValue("mainWindowGeometry", saveGeometry());
		settings->setValue("leftTabsIndex", leftTabs->currentIndex());
		settings->setLastActivePlaylistGeometry(tabPlaylists->currentPlayList()->horizontalHeader()->saveState());
		Settings::instance()->setVolume((qreal)volumeSlider->value() / 100.0);
		settings->sync();
	});
}

/** Update fonts for menu and context menus. */
void MainWindow::updateFonts(const QFont &font)
{
	menuBar()->setFont(font);
	for (QAction *action : findChildren<QAction*>()) {
		action->setFont(font);
	}
}

/** Open a new Dialog where one can add a folder to current playlist. */
void MainWindow::openFolder(const QString &dir)
{
	Settings::instance()->setValue("lastOpenedLocation", dir);
	QDirIterator it(dir, QDirIterator::Subdirectories);
	QStringList suffixes = FileHelper::suffixes(FileHelper::All, false);
	QStringList tracks;
	while (it.hasNext()) {
		it.next();
		if (suffixes.contains(it.fileInfo().suffix())) {
			tracks << "file://" + it.filePath();
		}
	}
	if (Miam::showWarning(tr("playlist"), tracks.count()) == QMessageBox::Ok) {
		tabPlaylists->insertItemsToPlaylist(-1, tracks);
	}
}

/** Redefined to be able to retransltate User Interface at runtime. */
void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
		//quickStart->retranslateUi(quickStart);
		tagEditor->retranslateUi(tagEditor);
		tagEditor->tagConverter->retranslateUi(tagEditor->tagConverter);
		libraryHeader->libraryOrderDialog->retranslateUi(libraryHeader->libraryOrderDialog);

		// (need to be tested with Arabic language)
		if (tr("LTR") == "RTL") {
			QApplication::setLayoutDirection(Qt::RightToLeft);
		}
	} else {
		QMainWindow::changeEvent(event);
	}
}

void MainWindow::closeEvent(QCloseEvent *)
{
	auto settings = SettingsPrivate::instance();
	if (settings->playbackKeepPlaylists()) {
		QList<uint> list = settings->lastPlaylistSession();
		list.clear();
		for (int i = 0; i < tabPlaylists->count(); i++) {
			Playlist *p = tabPlaylists->playlist(i);
			bool isOverwritting = p->id() != 0;
			uint id = tabPlaylists->playlistManager()->savePlaylist(p, isOverwritting, true);
			if (id != 0) {
				list.append(id);
			}
		}
		settings->setLastPlaylistSession(list);
		int idx = tabPlaylists->currentIndex();
		Playlist *p = tabPlaylists->playlist(idx);
		settings->setValue("lastActiveTab", idx);
		qDebug() << p->mediaPlaylist()->playbackMode();
		int m = p->mediaPlaylist()->playbackMode();
		settings->setValue("lastActivePlaylistMode", m);
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("playlist/x-tableview-item") || event->mimeData()->hasFormat("treeview/x-treeview-item")) {
		// Display a forbid cursor when one has started a drag from a playlist
		// Accepted drops are other playlists or the tabbar
		event->ignore();
	} else {
		event->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *event)
{
	// Ignore Drag & Drop if the source is a part of this player
	if (event->source() != nullptr) {
		return;
	}
	this->dispatchDrop(event);
}

bool MainWindow::event(QEvent *e)
{
	bool b = QMainWindow::event(e);
	// Init the address bar. It's really important to have the exact width on screen
	if (e->type() == QEvent::Show) {
		if (!filesystem->isVisible()) {
			addressBar->setMinimumWidth(leftTabs->width());
		}
		addressBar->init(QDir(SettingsPrivate::instance()->defaultLocationFileExplorer()));
	}
	return b;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
	_playbackModeWidgetFactory->move();
	QMainWindow::moveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	//qDebug() << Q_FUNC_INFO << e->oldSize() << e->size();
	QMainWindow::resizeEvent(e);
}

void MainWindow::loadThemeAndSettings()
{
	auto settings = Settings::instance();
	auto settingsPrivate = SettingsPrivate::instance();

	// Buttons
	for (MediaButton *b : mediaButtons) {
		b->setMediaPlayer(_mediaPlayer);
		if (settingsPrivate->isThemeCustomized()) {
			b->setIcon(QIcon(settingsPrivate->customIcon(b->objectName())));
		} else {
			b->setIconFromTheme(settings->theme());
		}
		b->setVisible(settingsPrivate->isMediaButtonVisible(b->objectName()));
	}

	// Fonts
	this->updateFonts(settingsPrivate->font(SettingsPrivate::FF_Menu));
	searchBar->setFont(settingsPrivate->font(SettingsPrivate::FF_Library));
}

void MainWindow::createCustomizeOptionsDialog()
{
	CustomizeOptionsDialog *dialog = new CustomizeOptionsDialog(_pluginManager, this);
	connect(dialog, &CustomizeOptionsDialog::aboutToBindShortcut, this, &MainWindow::bindShortcut);
	connect(dialog, &CustomizeOptionsDialog::defaultLocationFileExplorerHasChanged, addressBar, &AddressBar::init);
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

void MainWindow::processArgs(const QStringList &args)
{
	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::tr("Command line helper for Miam-Player"));
	parser.addHelpOption();

	QCommandLineOption directoryOption(QStringList() << "d" << "directory", tr("Directory to open."), tr("dir"));
	QCommandLineOption createNewPlaylist(QStringList() << "n" << "new-playlist", tr("Medias are added into a new playlist."));
	QCommandLineOption sendToTagEditor(QStringList() << "t" << "tag-editor", tr("Medias are sent to tag editor."));
	QCommandLineOption addToLibrary(QStringList() << "l" << "library", tr("Directory is sent to library."));
	QCommandLineOption playPause(QStringList() << "p" << "play", tr("Play or pause track in active playlist."));
	QCommandLineOption stop(QStringList() << "s" << "stop", tr("Stop playback."));
	QCommandLineOption skipForward(QStringList() << "f" << "forward", tr("Play next track."));
	QCommandLineOption skipBackward(QStringList() << "b" << "backward", tr("Play previous track."));
	QCommandLineOption volume(QStringList() << "v" << "volume", tr("Set volume of the player."), tr("volume"));

	parser.addOption(directoryOption);
	parser.addPositionalArgument("files", "Files to open", "[files]");
	parser.addOption(createNewPlaylist);
	parser.addOption(sendToTagEditor);
	parser.addOption(addToLibrary);
	parser.addOption(playPause);
	parser.addOption(stop);
	parser.addOption(skipForward);
	parser.addOption(skipBackward);
	parser.addOption(volume);
	parser.process(args);

	QStringList positionalArgs = parser.positionalArguments();
	bool isDirectoryOption = parser.isSet(directoryOption);
	bool isCreateNewPlaylist = parser.isSet(createNewPlaylist);
	bool isSendToTagEditor = parser.isSet(sendToTagEditor);
	bool isAddToLibrary = parser.isSet(addToLibrary);

	// -d <dir> and -f <files...> options are exclusive
	// It could be possible to use them at the same time but it can be confusing. Directory takes precedence
	if (isDirectoryOption) {
		QFileInfo fileInfo(parser.value(directoryOption));
		if (!fileInfo.isDir()) {
			parser.showHelp();
		}
		if (isSendToTagEditor) {
			tagEditor->addDirectory(fileInfo.absoluteDir());
			actionViewTagEditor->trigger();
		} else if (isAddToLibrary) {
			SettingsPrivate::instance()->addMusicLocations(QList<QDir>() << QDir(fileInfo.absoluteFilePath()));
		} else {
			if (isCreateNewPlaylist) {
				tabPlaylists->addPlaylist();
			}
			this->openFolder(fileInfo.absoluteFilePath());
		}
	} else if (!positionalArgs.isEmpty()) {
		if (isSendToTagEditor) {
			tagEditor->addItemsToEditor(positionalArgs);
			actionViewTagEditor->trigger();
		} else {
			if (isCreateNewPlaylist) {
				tabPlaylists->addPlaylist();
			}
			QStringList tracks;
			for (QString p : positionalArgs) {
				tracks << "file://" + p;
			}
			tabPlaylists->insertItemsToPlaylist(-1, tracks);
		}
	} else if (parser.isSet(playPause)) {
		if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
			_mediaPlayer->pause();
		} else {
			_mediaPlayer->play();
		}
	} else if (parser.isSet(skipForward)) {
		_mediaPlayer->skipForward();
	} else if (parser.isSet(skipBackward)) {
		_mediaPlayer->skipBackward();
	} else if (parser.isSet(stop)) {
		_mediaPlayer->stop();
	} else if (parser.isSet(volume)) {
		bool ok = false;
		int vol = parser.value(volume).toInt(&ok);
		if (ok) {
			volumeSlider->setValue(vol);
		}
	}
}

void MainWindow::bindShortcut(const QString &objectName, const QKeySequence &keySequence)
{
	QAction *action = findChild<QAction*>("action" + objectName.left(1).toUpper() + objectName.mid(1));
	// Connect actions first
	if (action) {
		action->setShortcut(keySequence);
		// Some default shortcuts might interfer with other widgets, so we need to restrict where it applies
		if (action == actionIncreaseVolume || action == actionDecreaseVolume) {
			action->setShortcutContext(Qt::WidgetShortcut);
		} else if (action == actionRemoveSelectedTracks) {
			action->setShortcutContext(Qt::ApplicationShortcut);
		}
	// Specific actions not defined in main menu
	} else if (objectName == "showTabLibrary" || objectName == "showTabFilesystem") {
		leftTabs->setShortcut(objectName, keySequence);
	} else if (objectName == "sendToCurrentPlaylist") {
		library->sendToCurrentPlaylist->setKey(keySequence);
	} else if (objectName == "sendToTagEditor") {
		library->openTagEditor->setKey(keySequence);
	} else if (objectName == "search") {
		searchBar->shortcut->setKey(keySequence);
	}
}

void MainWindow::mediaPlayerStateHasChanged(QMediaPlayer::State state)
{
	playButton->disconnect();
	actionPlay->disconnect();
	if (state == QMediaPlayer::PlayingState) {
		QString iconPath;
		if (SettingsPrivate::instance()->hasCustomIcon("pauseButton")) {
			iconPath = SettingsPrivate::instance()->customIcon("pauseButton");
		} else {
			iconPath = ":/player/" + Settings::instance()->theme() + "/pause";
		}
		playButton->setIcon(QIcon(iconPath));
		connect(playButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::pause);
		connect(actionPlay, &QAction::triggered, _mediaPlayer, &MediaPlayer::pause);
		seekSlider->setEnabled(true);
	} else {
		playButton->setIcon(QIcon(":/player/" + Settings::instance()->theme() + "/play"));
		connect(playButton, &QAbstractButton::clicked, _mediaPlayer, &MediaPlayer::play);
		connect(actionPlay, &QAction::triggered, _mediaPlayer, &MediaPlayer::play);
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

void MainWindow::openFiles()
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

	audioFiles.append(" (" + FileHelper::suffixes(FileHelper::Standard, true).join(" ") + ")");
	audioFiles.append(";;Game Music Emu (" + FileHelper::suffixes(FileHelper::GameMusicEmu, true).join(" ") + ");;");
	audioFiles.append(tr("Every file type (*)"));

	QStringList files = QFileDialog::getOpenFileNames(this, tr("Choose some files to open"), lastOpenedLocation,
													  audioFiles);
	if (files.isEmpty()) {
		settings->setValue("lastOpenedLocation", defaultMusicLocation);
	} else {
		QFileInfo fileInfo(files.first());
		settings->setValue("lastOpenedLocation", fileInfo.absolutePath());
		QStringList tracks;
		for (QString file : files) {
			tracks << "file://" + file;
		}
		tabPlaylists->insertItemsToPlaylist(-1, tracks);
	}
}

void MainWindow::openFolderPopup()
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

void MainWindow::openPlaylistManager()
{
	PlaylistDialog *playlistDialog = new PlaylistDialog(this);
	playlistDialog->setPlaylists(tabPlaylists->playlists());
	connect(playlistDialog, &PlaylistDialog::aboutToLoadPlaylist, tabPlaylists, &TabPlaylist::loadPlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToDeletePlaylist, tabPlaylists, &TabPlaylist::deletePlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToRenamePlaylist, tabPlaylists, &TabPlaylist::renamePlaylist);
	connect(playlistDialog, &PlaylistDialog::aboutToRenameTab, tabPlaylists, &TabPlaylist::renameTab);
	connect(playlistDialog, &PlaylistDialog::aboutToSavePlaylist, tabPlaylists, &TabPlaylist::savePlaylist);
	playlistDialog->open();
}

void MainWindow::showTabPlaylists()
{
	if (!actionViewPlaylists->isChecked()) {
		actionViewPlaylists->setChecked(true);
		Settings::instance()->setLastActiveView(actionViewPlaylists->objectName());
	}
	stackedWidgetRight->setCurrentIndex(0);
}

void MainWindow::showTagEditor()
{
	if (!actionViewTagEditor->isChecked()) {
		actionViewTagEditor->setChecked(true);
		Settings::instance()->setLastActiveView(actionViewTagEditor->objectName());
	}
	stackedWidgetRight->setCurrentIndex(1);
}

void MainWindow::toggleMenuBar(bool checked)
{
	qDebug() << Q_FUNC_INFO << "not yet implemented" << checked;
}
