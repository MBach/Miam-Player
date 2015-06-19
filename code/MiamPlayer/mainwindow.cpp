#include "mainwindow.h"

#include <settings.h>
#include <settingsprivate.h>
#include <musicsearchengine.h>
#include "dialogs/customizethemedialog.h"
#include "dialogs/dragdropdialog.h"
#include "playlists/playlist.h"
#include "pluginmanager.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtMultimedia/QAudioDeviceInfo>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	setupUi(this);
	widgetSearchBar->setFrameBorder(false, false, true, false);

	this->setAcceptDrops(true);
#ifndef Q_OS_MAC
	this->setWindowIcon(QIcon(":/icons/mp_win32"));
#endif

	// Special behaviour for media buttons
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << stopButton
				 << seekForwardButton << skipForwardButton << playbackModeButton;
	tabPlaylists->setMainWindow(this);

	/// XXX
	_uniqueLibrary = new UniqueLibrary(this);
	stackedWidget->addWidget(_uniqueLibrary);
	_uniqueLibrary->hide();

	this->loadTheme();
	// Instantiate dialogs
	customizeOptionsDialog = new CustomizeOptionsDialog;

	playbackModeWidgetFactory = new PlaybackModeWidgetFactory(this, playbackModeButton, tabPlaylists);

	this->installEventFilter(this);
}

void MainWindow::activateLastView()
{
	// Find the last active view and connect database to it
	QString viewName = Settings::instance()->lastActiveView();
	QAction *lastActiveView = findChild<QAction*>(viewName);
	if (lastActiveView) {
		lastActiveView->trigger();
	}
	SqlDatabase::instance()->load();
}

void MainWindow::dispatchDrop(QDropEvent *event)
{
	/** Popup shown to one when tracks are dropped from another application to MiamPlayer. */
	DragDropDialog *dragDropDialog = new DragDropDialog;

	// Drag & Drop actions
	connect(dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToLibrary, customizeOptionsDialog, &CustomizeOptionsDialog::addMusicLocations);
	connect(dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToPlaylist, tabPlaylists, &TabPlaylist::addExtFolders);

	bool onlyFiles = dragDropDialog->setMimeData(event->mimeData());
	if (onlyFiles) {
		QStringList tracks;
		for (QString file : dragDropDialog->externalLocations()) {
			tracks << "file://" + file;
		}
		tracks.sort(Qt::CaseInsensitive);
		tabPlaylists->insertItemsToPlaylist(-1, tracks);
	} else {
		QList<QDir> dirs;
		for (QString location : dragDropDialog->externalLocations()) {
			dirs << location;
		}
		switch (SettingsPrivate::instance()->dragDropAction()) {
		case SettingsPrivate::DD_OpenPopup:
			dragDropDialog->show();
			break;
		case SettingsPrivate::DD_AddToLibrary:
			customizeOptionsDialog->addMusicLocations(dirs);
			break;
		case SettingsPrivate::DD_AddToPlaylist:
			tabPlaylists->addExtFolders(dirs);
			break;
		}
	}
}

void MainWindow::init()
{
	searchBar->init(this);
	library->init();
	_uniqueLibrary->init();
	tagEditor->init();

	// Load playlists at startup if any, otherwise just add an empty one
	this->setupActions();

	auto settingsPrivate = SettingsPrivate::instance();
	bool isEmpty = settingsPrivate->musicLocations().isEmpty();
	quickStart->setVisible(isEmpty);
	libraryHeader->setVisible(!isEmpty);
	changeHierarchyButton->setVisible(!isEmpty);
	/// XXX For each view
	library->setVisible(!isEmpty);
	/// XXX

	actionScanLibrary->setDisabled(isEmpty);
	widgetSearchBar->setVisible(!isEmpty);
	this->showTabPlaylists();
	if (isEmpty) {
		quickStart->searchMultimediaFiles();
	} else {
		// db->load();
	}

	Settings *settings = Settings::instance();
	this->restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
	leftTabs->setCurrentIndex(settings->value("leftTabsIndex").toInt());

	tabPlaylists->init();

	// Load shortcuts
	customizeOptionsDialog->initShortcuts();

	playbackModeWidgetFactory->update();
}

/** Plugins. */
void MainWindow::loadPlugins()
{
	PluginManager *pm = PluginManager::instance();
	pm->setMainWindow(this);
	int row = Settings::instance()->value("customizeOptionsDialogCurrentTab", 0).toInt();
	if (customizeOptionsDialog->listWidget->isRowHidden(5) && row == 5) {
		customizeOptionsDialog->listWidget->setCurrentRow(0);
	} else {
		customizeOptionsDialog->listWidget->setCurrentRow(row);
	}
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	SqlDatabase *db = SqlDatabase::instance();

	// Load music
	connect(customizeOptionsDialog, &CustomizeOptionsDialog::musicLocationsHaveChanged, [=](const QStringList &oldLocations, const QStringList &newLocations) {
		bool libraryIsEmpty = newLocations.isEmpty();
		quickStart->setVisible(libraryIsEmpty);
		library->setVisible(!libraryIsEmpty);
		libraryHeader->setVisible(!libraryIsEmpty);
		changeHierarchyButton->setVisible(!libraryIsEmpty);
		actionScanLibrary->setDisabled(libraryIsEmpty);
		widgetSearchBar->setVisible(!libraryIsEmpty);

		if (libraryIsEmpty) {
			db->rebuild(oldLocations, QStringList());
			quickStart->searchMultimediaFiles();
		} else {
			db->rebuild(oldLocations, newLocations);
		}
	});
	connect(customizeOptionsDialog, &CustomizeOptionsDialog::defaultLocationFileExplorerHasChanged, addressBar, &AddressBar::init);

	connect(db, &SqlDatabase::aboutToLoad, libraryHeader, &LibraryHeader::resetSortOrder);

	// Adds a group where view mode are mutually exclusive
	QActionGroup *viewModeGroup = new QActionGroup(this);
	actionViewPlaylists->setActionGroup(viewModeGroup);
	actionViewUniqueLibrary->setActionGroup(viewModeGroup);
	actionViewTagEditor->setActionGroup(viewModeGroup);

	connect(actionViewPlaylists, &QAction::triggered, this, [=]() {
		stackedWidget->setCurrentIndex(0);
		stackedWidgetRight->setCurrentIndex(0);
		Settings::instance()->setLastActiveView(actionViewPlaylists->objectName());
	});
	connect(actionViewUniqueLibrary, &QAction::triggered, this, [=]() {
		stackedWidget->setCurrentIndex(1);
		Settings::instance()->setLastActiveView(actionViewUniqueLibrary->objectName());
	});
	connect(actionViewTagEditor, &QAction::triggered, this, [=]() {
		stackedWidget->setCurrentIndex(0);
		stackedWidgetRight->setCurrentIndex(1);
		actionViewTagEditor->setChecked(true);
		Settings::instance()->setLastActiveView(actionViewTagEditor->objectName());
	});

	QActionGroup *actionPlaybackGroup = new QActionGroup(this);
	for (QAction *actionPlayBack : findChildren<QAction*>(QRegExp("actionPlayback*", Qt::CaseSensitive, QRegExp::Wildcard))) {
		actionPlaybackGroup->addAction(actionPlayBack);
		connect(actionPlayBack, &QAction::triggered, this, [=]() {
			const QMetaObject &mo = QMediaPlaylist::staticMetaObject;
			QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PlaybackMode"));
			QString enu = actionPlayBack->property("PlaybackMode").toString();
			playbackModeWidgetFactory->setPlaybackMode((QMediaPlaylist::PlaybackMode)metaEnum.keyToValue(enu.toStdString().data()));
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
	connect(actionShowOptions, &QAction::triggered, customizeOptionsDialog, &CustomizeOptionsDialog::open);
	connect(actionAboutQt, &QAction::triggered, &QApplication::aboutQt);
	connect(actionScanLibrary, &QAction::triggered, this, [=]() {
		searchBar->clear();
		db->rebuild();
	});
	connect(actionShowHelp, &QAction::triggered, this, [=]() {
		QDesktopServices::openUrl(QUrl("http://miam-player.org/wiki/index.php"));
	});

	// Quick Start
	connect(quickStart->commandLinkButtonLibrary, &QAbstractButton::clicked, customizeOptionsDialog, &CustomizeOptionsDialog::open);

	// Lambda function to reduce duplicate code
	SettingsPrivate *settings = SettingsPrivate::instance();
	auto applyButtonClicked = [this, db, settings] (const QStringList &newLocations) {
		settings->setMusicLocations(newLocations);
		quickStart->hide();
		library->show();
		libraryHeader->show();
		changeHierarchyButton->show();
		widgetSearchBar->show();
		actionScanLibrary->setEnabled(true);
		db->rebuild();
	};

	// Set only one location in the Library: the default music folder
	connect(quickStart->defaultFolderApplyButton, &QDialogButtonBox::clicked, [=] (QAbstractButton *) {
		QString musicLocation = quickStart->defaultFolderTableWidget->item(0, 1)->data(Qt::DisplayRole).toString();
		customizeOptionsDialog->addMusicLocation(QDir(musicLocation));
		musicLocation = QDir::toNativeSeparators(musicLocation);
		QStringList newLocations;
		newLocations.append(musicLocation);
		applyButtonClicked(newLocations);
	});

	// Select only folders that are checked by one
	connect(quickStart->quickStartApplyButton, &QDialogButtonBox::clicked, [=] (QAbstractButton *) {
		QStringList newLocations;
		for (int i = 1; i < quickStart->quickStartTableWidget->rowCount(); i++) {
			if (quickStart->quickStartTableWidget->item(i, 0)->checkState() == Qt::Checked) {
				QString musicLocation = quickStart->quickStartTableWidget->item(i, 1)->data(Qt::UserRole).toString();
				customizeOptionsDialog->addMusicLocation(QDir(musicLocation));
				musicLocation = QDir::toNativeSeparators(musicLocation);
				newLocations.append(musicLocation);
			}
		}
		applyButtonClicked(newLocations);
	});

	for (TreeView *tab : this->findChildren<TreeView*>()) {
		connect(tab, &TreeView::aboutToInsertToPlaylist, tabPlaylists, &TabPlaylist::insertItemsToPlaylist);
		connect(tab, &TreeView::sendToTagEditor, this, [=](const QModelIndexList , const QStringList &tracks) {
			this->showTagEditor();
			tagEditor->addItemsToEditor(tracks);
		});
	}

	// Send one folder to the music locations
	connect(filesystem, &FileSystemTreeView::aboutToAddMusicLocations, customizeOptionsDialog, &CustomizeOptionsDialog::addMusicLocations);

	// Send music to the tag editor
	connect(tagEditor, &TagEditor::aboutToCloseTagEditor, this, &MainWindow::showTabPlaylists);
	connect(tabPlaylists, &TabPlaylist::aboutToSendToTagEditor, [=](const QList<QUrl> &tracks) {
		this->showTagEditor();
		tagEditor->addItemsToEditor(tracks);
	});

	// Media buttons and their shortcuts
	auto mp = MediaPlayer::instance();
	mp->setParent(this);
	connect(menuPlayback, &QMenu::aboutToShow, this, [=]() {
		bool isPlaying = (mp->state() == QMediaPlayer::PlayingState || mp->state() == QMediaPlayer::PausedState);
		actionSeekBackward->setEnabled(isPlaying);
		actionStop->setEnabled(isPlaying);
		actionStopAfterCurrent->setEnabled(isPlaying);
		actionStopAfterCurrent->setChecked(mp->isStopAfterCurrent());
		actionSeekForward->setEnabled(isPlaying);

		bool notEmpty = mp->playlist() && !mp->playlist()->isEmpty();
		actionSkipBackward->setEnabled(notEmpty);
		actionPlay->setEnabled(notEmpty);
		actionSkipForward->setEnabled(notEmpty);
	});
	connect(actionSkipBackward, &QAction::triggered, mp, &MediaPlayer::skipBackward);
	connect(skipBackwardButton, &QAbstractButton::clicked, mp, &MediaPlayer::skipBackward);
	connect(actionSeekBackward, &QAction::triggered, mp, &MediaPlayer::seekBackward);
	connect(seekBackwardButton, &QAbstractButton::clicked, mp, &MediaPlayer::seekBackward);
	connect(actionPlay, &QAction::triggered, mp, &MediaPlayer::play);

	connect(playButton, &QAbstractButton::clicked, mp, &MediaPlayer::play);
	connect(actionStop, &QAction::triggered, mp, &MediaPlayer::stop);
	connect(actionStopAfterCurrent, &QAction::triggered, mp, &MediaPlayer::stopAfterCurrent);
	connect(stopButton, &QAbstractButton::clicked, mp, &MediaPlayer::stop);
	connect(actionSeekForward, &QAction::triggered, mp, &MediaPlayer::seekForward);
	connect(seekForwardButton, &QAbstractButton::clicked, mp, &MediaPlayer::seekForward);
	connect(actionSkipForward, &QAction::triggered, mp, &MediaPlayer::skipForward);
	connect(skipForwardButton, &QAbstractButton::clicked, mp, &MediaPlayer::skipForward);
	connect(playbackModeButton, &MediaButton::mediaButtonChanged, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);

	// Sliders
	connect(mp, &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			seekSlider->setValue(1000 * pos / duration);
			timeLabel->setTime(pos, duration);
		}
	});

	// Volume bar
	connect(volumeSlider, &QSlider::valueChanged, mp, &MediaPlayer::setVolume);
	volumeSlider->setValue(Settings::instance()->volume());

	// Filter the library when user is typing some text to find artist, album or tracks
	library->setSearchBar(searchBar);

	// Core
	connect(mp, &MediaPlayer::stateChanged, this, &MainWindow::mediaPlayerStateHasChanged);

	// Playback
	connect(tabPlaylists, &TabPlaylist::updatePlaybackModeButton, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);
	connect(actionRemoveSelectedTracks, &QAction::triggered, this, [=]() {
		if (tabPlaylists->currentPlayList()) {
			tabPlaylists->currentPlayList()->removeSelectedTracks();
		}
	});
	connect(actionMoveTracksUp, &QAction::triggered, tabPlaylists, &TabPlaylist::moveTracksUp);
	connect(actionMoveTracksDown, &QAction::triggered, tabPlaylists, &TabPlaylist::moveTracksDown);
	connect(actionOpenPlaylistManager, &QAction::triggered, this, &MainWindow::openPlaylistManager);
	connect(actionMute, &QAction::triggered, mp, &MediaPlayer::toggleMute);
	connect(actionIncreaseVolume, &QAction::triggered, this, [=]() {
		volumeSlider->setValue(volumeSlider->value() + 5);
	});
	connect(actionDecreaseVolume, &QAction::triggered, this, [=]() {
		volumeSlider->setValue(volumeSlider->value() - 5);
	});

	connect(filesystem, &FileSystemTreeView::folderChanged, addressBar, &AddressBar::init);
	connect(addressBar, &AddressBar::aboutToChangePath, filesystem, &FileSystemTreeView::reloadWithNewPath);

	// Playback modes
	connect(playbackModeButton, &QPushButton::clicked, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::togglePlaybackModes);

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
		delete PluginManager::instance();
		settings->setValue("mainWindowGeometry", saveGeometry());
		settings->setValue("leftTabsIndex", leftTabs->currentIndex());
		settings->setLastActivePlaylistGeometry(tabPlaylists->currentPlayList()->horizontalHeader()->saveState());
		Settings::instance()->setVolume(volumeSlider->value());
		settings->sync();
	});

	// Shortcuts
	connect(customizeOptionsDialog, &CustomizeOptionsDialog::aboutToBindShortcut, this, &MainWindow::bindShortcut);
}

/** Update fonts for menu and context menus. */
void MainWindow::updateFonts(const QFont &font)
{
	menuBar()->setFont(font);
	for (QAction *action : findChildren<QAction*>()) {
		action->setFont(font);
	}
}

QMessageBox::StandardButton MainWindow::showWarning(const QString &target, int count)
{
	QMessageBox::StandardButton ret = QMessageBox::Ok;
	/// XXX: extract magic number (to where?)
	if (count > 300) {
		QMessageBox msgBox;
		QString totalFiles = tr("There are more than 300 files to add to the %1 (%2 to add).");
		msgBox.setText(totalFiles.arg(target).arg(count));
		msgBox.setInformativeText(tr("Are you sure you want to continue? This might take some time."));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Ok);
		ret = (QMessageBox::StandardButton) msgBox.exec();
	}
	return ret;
}

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
	if (showWarning(tr("playlist"), tracks.count()) == QMessageBox::Ok) {
		tabPlaylists->insertItemsToPlaylist(-1, tracks);
	}
}

/** Redefined to be able to retransltate User Interface at runtime. */
void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
		customizeOptionsDialog->retranslateUi(customizeOptionsDialog);
		quickStart->retranslateUi(quickStart);
		//playlistDialog->retranslateUi(playlistDialog);
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
	}
	QCoreApplication::quit();
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
	if (event->source() != NULL) {
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
	playbackModeWidgetFactory->move();
	QMainWindow::moveEvent(event);
}

void MainWindow::loadTheme()
{
	// Buttons
	auto settings = Settings::instance();
	auto settingsPrivate = SettingsPrivate::instance();
	for (MediaButton *b : mediaButtons) {
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
	auto mp = MediaPlayer::instance();

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
			customizeOptionsDialog->addMusicLocations(QList<QDir>() << QDir(fileInfo.absoluteFilePath()));
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
		if (mp->state() == QMediaPlayer::PlayingState) {
			mp->pause();
		} else {
			mp->play();
		}
	} else if (parser.isSet(skipForward)) {
		mp->skipForward();
	} else if (parser.isSet(skipBackward)) {
		mp->skipBackward();
	} else if (parser.isSet(stop)) {
		mp->stop();
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
	auto mp = MediaPlayer::instance();
	if (state == QMediaPlayer::PlayingState) {
		QString iconPath;
		if (SettingsPrivate::instance()->hasCustomIcon("pauseButton")) {
			iconPath = SettingsPrivate::instance()->customIcon("pauseButton");
		} else {
			iconPath = ":/player/" + Settings::instance()->theme() + "/pause";
		}
		playButton->setIcon(QIcon(iconPath));
		connect(playButton, &QAbstractButton::clicked, mp, &MediaPlayer::pause);
		connect(actionPlay, &QAction::triggered, mp, &MediaPlayer::pause);
		seekSlider->setEnabled(true);
	} else {
		playButton->setIcon(QIcon(":/player/" + Settings::instance()->theme() + "/play"));
		connect(playButton, &QAbstractButton::clicked, mp, &MediaPlayer::play);
		connect(actionPlay, &QAction::triggered, mp, &MediaPlayer::play);
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
