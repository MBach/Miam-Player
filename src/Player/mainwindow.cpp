#include "mainwindow.h"

#include <mediabuttons/mediabutton.h>
#include <abstractviewplaylists.h>
#include <musicsearchengine.h>
#include <quickstart.h>
#include <settings.h>
#include <settingsprivate.h>

#include "dialogs/customizethemedialog.h"
#include "dialogs/dragdropdialog.h"
#include "dialogs/equalizerdalog.h"
#include "views/viewloader.h"
#include "pluginmanager.h"

#include <QDesktopServices>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, _mediaPlayer(new MediaPlayer(this))
	, _pluginManager(new PluginManager(this))
	, _currentView(nullptr)
	, _tagEditor(nullptr)
{
	setupUi(this);
	actionPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	actionStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

	this->setAcceptDrops(true);
#ifndef Q_OS_MAC
	this->setWindowIcon(QIcon(":/icons/mp_win32"));
#else
	actionHideMenuBar->setVisible(false);
#endif

	// Fonts
	auto settings = SettingsPrivate::instance();
	this->updateFonts(settings->font(SettingsPrivate::FF_Menu));

	//menubar->installEventFilter(this);
	menubar->setHidden(settings->value("isMenuHidden", false).toBool());
}

void MainWindow::activateLastView()
{
	bool isEmpty = SettingsPrivate::instance()->musicLocations().isEmpty();
	actionScanLibrary->setDisabled(isEmpty);
	if (isEmpty) {
		initQuickStart();
	} else {
		// Find the last active view and connect database to it
		QString actionViewName = SettingsPrivate::instance()->value("lastActiveView", "actionViewPlaylists").toString();
		for (QAction *actionView : menuView->actions()) {
			if (actionView->objectName() == actionViewName) {
				actionView->trigger();
				break;
			}
		}
	}
}

void MainWindow::dispatchDrop(QDropEvent *event)
{
	/** Popup shown to one when tracks are dropped from another application to MiamPlayer. */
	DragDropDialog dragDropDialog;

	SettingsPrivate *settings = SettingsPrivate::instance();

	AbstractViewPlaylists *viewPlaylists = nullptr;

	// Drag & Drop actions
	connect(&dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToLibrary, settings, &SettingsPrivate::addMusicLocations);
	if (_currentView && _currentView->viewProperty(SettingsPrivate::VP_PlaylistFeature)) {
		viewPlaylists = static_cast<AbstractViewPlaylists*>(_currentView);
		connect(&dragDropDialog, &DragDropDialog::aboutToAddExtFoldersToPlaylist, viewPlaylists, &AbstractViewPlaylists::addExtFolders);
	}

	bool onlyFiles = dragDropDialog.setMimeData(event->mimeData());
	if (onlyFiles) {
		QStringList tracks;
		for (QString file : dragDropDialog.externalLocations) {
			tracks << "file://" + file;
		}
		tracks.sort(Qt::CaseInsensitive);
		QList<QUrl> urls;
		for (QString t : tracks) {
			urls << QUrl::fromLocalFile(t);
		}
		if (viewPlaylists) {
			viewPlaylists->addToPlaylist(urls);
		}
	} else {
		QList<QDir> dirs;
		for (QString location : dragDropDialog.externalLocations) {
			dirs << location;
		}
		switch (SettingsPrivate::instance()->dragDropAction()) {
		case SettingsPrivate::DD_OpenPopup:
			dragDropDialog.exec();
			break;
		case SettingsPrivate::DD_AddToLibrary:
			settings->addMusicLocations(dirs);
			break;
		case SettingsPrivate::DD_AddToPlaylist:
			if (viewPlaylists) {
				viewPlaylists->addExtFolders(dirs);
			}
			break;
		}
	}
}

void MainWindow::init()
{
	this->setupActions();

	// Init shortcuts
	Settings *settings = Settings::instance();
	QMapIterator<QString, QVariant> it(settings->shortcuts());
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().value<QKeySequence>());
	}
}

/** Plugins. */
void MainWindow::loadPlugins()
{
	/// FIXME
	QObjectList libraryObjectList;
	//libraryObjectList << library << library->properties;

	QObjectList tagEditorObjectList;
	//tagEditorObjectList << tagEditor->albumCover->contextMenu() << tagEditor->extensiblePushButtonArea << tagEditor->extensibleWidgetArea << tagEditor->tagEditorWidget << tagEditor;

	//_pluginManager->registerExtensionPoint(library->metaObject()->className(), libraryObjectList);
	//_pluginManager->registerExtensionPoint(tagEditor->metaObject()->className(), tagEditorObjectList);
	_pluginManager->init();
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Adds a group where view mode are mutually exclusive
	QActionGroup *viewModeGroup = new QActionGroup(this);
	connect(viewModeGroup, &QActionGroup::triggered, this, &MainWindow::activateView);
	actionViewPlaylists->setActionGroup(viewModeGroup);
	actionViewUniqueLibrary->setActionGroup(viewModeGroup);
	connect(actionViewTagEditor, &QAction::triggered, this, &MainWindow::showTagEditor);

	QActionGroup *actionPlaybackGroup = new QActionGroup(this);
	for (QAction *actionPlayBack : findChildren<QAction*>(QRegExp("actionPlayback*", Qt::CaseSensitive, QRegExp::Wildcard))) {
		actionPlaybackGroup->addAction(actionPlayBack);
		connect(actionPlayBack, &QAction::triggered, this, [=]() {
			const QMetaObject &mo = QMediaPlaylist::staticMetaObject;
			QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PlaybackMode"));
			QString enu = actionPlayBack->property("PlaybackMode").toString();
			/// FIXME
			//_playbackModeWidgetFactory->setPlaybackMode((QMediaPlaylist::PlaybackMode)metaEnum.keyToValue(enu.toStdString().data()));
		});
	}

	// Link user interface
	// Actions from the menu
	connect(actionExit, &QAction::triggered, this, [=]() {
		QCloseEvent event;
		this->closeEvent(&event);
		qApp->quit();
	});
	connect(actionShowCustomize, &QAction::triggered, this, [=]() {
		CustomizeThemeDialog customizeThemeDialog;
		customizeThemeDialog.exec();
	});

	connect(actionShowOptions, &QAction::triggered, this, &MainWindow::createCustomizeOptionsDialog);
	connect(actionAboutQt, &QAction::triggered, &QApplication::aboutQt);
	connect(actionHideMenuBar, &QAction::triggered, this, &MainWindow::toggleMenuBar);
	connect(actionScanLibrary, &QAction::triggered, this, [=]() {
		if (_currentView && _currentView->viewProperty(SettingsPrivate::VP_SearchArea)) {
			_currentView->setViewProperty(SettingsPrivate::VP_SearchArea, true);
		}

		/// FIXME
		//searchBar->clear();
		SqlDatabase::instance()->rebuild();
	});
	connect(actionShowHelp, &QAction::triggered, this, [=]() {
		QDesktopServices::openUrl(QUrl("http://miam-player.org/wiki/index.php"));
	});

	// Load music
	auto settingsPrivate = SettingsPrivate::instance();
	connect(settingsPrivate, &SettingsPrivate::musicLocationsHaveChanged, this, &MainWindow::musicLocationsHaveChanged);

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
	connect(actionSeekBackward, &QAction::triggered, _mediaPlayer, &MediaPlayer::seekBackward);
	connect(actionPlay, &QAction::triggered, _mediaPlayer, &MediaPlayer::togglePlayback);
	connect(actionStop, &QAction::triggered, _mediaPlayer, &MediaPlayer::stop);
	connect(actionStopAfterCurrent, &QAction::triggered, _mediaPlayer, &MediaPlayer::stopAfterCurrent);
	connect(actionSeekForward, &QAction::triggered, _mediaPlayer, &MediaPlayer::seekForward);
	connect(actionSkipForward, &QAction::triggered, _mediaPlayer, &MediaPlayer::skipForward);

	connect(actionShowEqualizer, &QAction::triggered, this, [=]() {
		EqualizerDialog *equalizerDialog = new EqualizerDialog(_mediaPlayer, this);
		equalizerDialog->show();
		equalizerDialog->activateWindow();
	});

	connect(actionMute, &QAction::triggered, _mediaPlayer, &MediaPlayer::toggleMute);

	connect(menuPlayback, &QMenu::aboutToShow, this, [=](){
		//QMediaPlaylist::PlaybackMode mode = tabPlaylists->currentPlayList()->mediaPlaylist()->playbackMode();
		const QMetaObject &mo = QMediaPlaylist::staticMetaObject;
		QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("PlaybackMode"));
		//QAction *action = findChild<QAction*>(QString("actionPlayback").append(metaEnum.valueToKey(mode)));
		//action->setChecked(true);
	});
}

/** Update fonts for menu and context menus. */
void MainWindow::updateFonts(const QFont &font)
{
#ifndef Q_OS_OSX
	menuBar()->setFont(font);
	for (QAction *action : findChildren<QAction*>()) {
		action->setFont(font);
	}
#else
	Q_UNUSED(font)
#endif
}

/** Redefined to be able to retransltate User Interface at runtime. */
void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);

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
	auto settingsPrivate = SettingsPrivate::instance();
	if (_currentView && _currentView->viewProperty(SettingsPrivate::VP_PlaylistFeature) && settingsPrivate->playbackKeepPlaylists()) {
		if (AbstractViewPlaylists *v = static_cast<AbstractViewPlaylists*>(_currentView)) {
			v->saveCurrentPlaylists();
		}
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
	if (e->type() == QEvent::KeyPress) {
		if (!this->menuBar()->isVisible()) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
			if (keyEvent->key() == Qt::Key_Alt) {
				qDebug() << Q_FUNC_INFO << "Alt was pressed";
				this->setProperty("altKey", true);
			} else {
				this->setProperty("altKey", false);
			}
		}
	} else if (e->type() == QEvent::KeyRelease) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
		if (this->property("altKey").toBool() && keyEvent->key() == Qt::Key_Alt) {
			qDebug() << Q_FUNC_INFO << "Alt was released";
			this->menuBar()->show();
			//this->menuBar()->setProperty("dirtyHackMnemonic", true);
			this->menuBar()->setFocus();
			this->setProperty("altKey", false);
			actionHideMenuBar->setChecked(false);
			SettingsPrivate::instance()->setValue("isMenuHidden", false);
		}
	}
	return b;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	/*if (watched == menubar) {
		//qDebug() << Q_FUNC_INFO << event->type();
	} else*/ if (watched == _tagEditor && QEvent::Close) {
		actionViewTagEditor->setChecked(false);
	}
	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::initQuickStart()
{
	QuickStart *quickStart = new QuickStart(this);
	quickStart->searchMultimediaFiles();
	connect(quickStart->commandLinkButtonLibrary, &QAbstractButton::clicked, this, &MainWindow::createCustomizeOptionsDialog);
	this->setCentralWidget(quickStart);
	actionOpenFiles->setDisabled(true);
	actionOpenFolder->setDisabled(true);
	menuView->setDisabled(true);
	menuPlayback->setDisabled(true);
	menuPlaylist->setDisabled(true);
}

void MainWindow::createCustomizeOptionsDialog()
{
	CustomizeOptionsDialog *dialog = new CustomizeOptionsDialog(_pluginManager, this);
	connect(dialog, &CustomizeOptionsDialog::aboutToBindShortcut, this, &MainWindow::bindShortcut);
	if (_currentView && _currentView->viewProperty(SettingsPrivate::VP_FileExplorerFeature)) {
		connect(dialog, &CustomizeOptionsDialog::defaultLocationFileExplorerHasChanged, _currentView, &AbstractView::initFileExplorer);
	}
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
			this->showTagEditor();
			_tagEditor->addDirectory(fileInfo.absoluteDir());
		} else if (isAddToLibrary) {
			SettingsPrivate::instance()->addMusicLocations(QList<QDir>() << QDir(fileInfo.absoluteFilePath()));
		} else {
			if (_currentView) {
				if (AbstractViewPlaylists *v = static_cast<AbstractViewPlaylists*>(_currentView)) {
					if (isCreateNewPlaylist) {
						v->addPlaylist();
					}
					v->openFolder(fileInfo.absoluteFilePath());
				}
			}
		}
	} else if (!positionalArgs.isEmpty()) {
		if (isSendToTagEditor) {
			this->showTagEditor();
			QList<QUrl> tracks;
			for (QString p : positionalArgs) {
				tracks << QUrl::fromLocalFile(p);
			}
			_tagEditor->addItemsToEditor(tracks);
		} else {
			if (_currentView) {
				if (AbstractViewPlaylists *v = static_cast<AbstractViewPlaylists*>(_currentView)) {
					if (isCreateNewPlaylist) {
						v->addPlaylist();
					}
					QList<QUrl> tracks;
					for (QString p : positionalArgs) {
						tracks << QUrl::fromLocalFile(p);
					}
					v->addToPlaylist(tracks);
				}
			}
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
			_mediaPlayer->setVolume((qreal)vol / 100.0);
		}
	}
}

void MainWindow::activateView(QAction *menuAction)
{
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();

	// User a Helper to load views depending on which classes are attached to the QAction
	ViewLoader v(_mediaPlayer);
	_currentView = v.load(menuAction->objectName());

	//if (_currentView) {
	//	auto db = SqlDatabase::instance();
	//	connect(db, &SqlDatabase::aboutToUpdateView, _currentView, &AbstractView::updateModel);
	//}

	if (_currentView && _currentView->viewProperty(SettingsPrivate::VP_OwnWindow)) {

	} else {
		if (!_currentView) {
			return;
		}
		// First, clean the view (can be a QuickStart instance)
		if (this->centralWidget()) {
			QWidget *w = this->takeCentralWidget();
			w->deleteLater();
		}

		// Replace the main widget
		this->restoreGeometry(settingsPrivate->lastActiveViewGeometry(menuAction->objectName()));
		this->setCentralWidget(_currentView);
	}

	// Basically, a music player provides a playlist feature or it does not.
	// It implies a clean and separate way to display things, I suppose.
	bool b = _currentView->viewProperty(SettingsPrivate::VP_PlaylistFeature);
	menuView->setEnabled(true);
	menuPlayback->setEnabled(true);
	menuPlaylist->setEnabled(true);
	menuPlaylist->menuAction()->setVisible(b);
	actionOpenFiles->setEnabled(b);
	actionOpenFolder->setEnabled(b);

	if (b) {
		AbstractViewPlaylists *viewPlaylists = static_cast<AbstractViewPlaylists*>(_currentView);
		connect(viewPlaylists, &AbstractViewPlaylists::aboutToSendToTagEditor, this, [=](const QModelIndexList &, const QList<QUrl> &tracks) {
			actionViewTagEditor->trigger();
			/// TODO, refresh indexes when tags have changed
			_tagEditor->addItemsToEditor(tracks);
		});

		connect(actionOpenFiles, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::openFiles);
		connect(actionOpenFolder, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::openFolderPopup);
		connect(actionAddPlaylist, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::addPlaylist);
		connect(actionDeleteCurrentPlaylist, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::removeCurrentPlaylist);
		connect(menuPlaylist, &QMenu::aboutToShow, this, [=]() {
			int selectedTracks = viewPlaylists->selectedTracksInCurrentPlaylist();
			bool b = selectedTracks > 0;
			actionRemoveSelectedTracks->setEnabled(b);
			actionMoveTracksUp->setEnabled(b);
			actionMoveTracksDown->setEnabled(b);
			if (selectedTracks > 1) {
				actionRemoveSelectedTracks->setText(tr("&Remove selected tracks", "Number of tracks to remove", selectedTracks));
				actionMoveTracksUp->setText(tr("Move selected tracks &up", "Move upward", selectedTracks));
				actionMoveTracksDown->setText(tr("Move selected tracks &down", "Move downward", selectedTracks));
			}
		});

		// Playback
		connect(actionRemoveSelectedTracks, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::removeSelectedTracks);
		connect(actionMoveTracksUp, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::moveTracksUp);
		connect(actionMoveTracksDown, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::moveTracksDown);
		connect(actionOpenPlaylistManager, &QAction::triggered, viewPlaylists, &AbstractViewPlaylists::openPlaylistManager);
	} else {
		disconnect(actionOpenFiles);
		disconnect(actionOpenFolder);
		disconnect(actionAddPlaylist);
		disconnect(actionDeleteCurrentPlaylist);
		disconnect(menuPlaylist);
		disconnect(actionRemoveSelectedTracks);
		disconnect(actionMoveTracksUp);
		disconnect(actionMoveTracksDown);
		disconnect(actionOpenPlaylistManager);
	}

	connect(actionIncreaseVolume, &QAction::triggered, _currentView, &AbstractView::volumeSliderIncrease);
	connect(actionIncreaseVolume, &QAction::triggered, _currentView, &AbstractView::volumeSliderDecrease);

	connect(qApp, &QApplication::aboutToQuit, this, [=] {
		if (_currentView) {
			QActionGroup *actionGroup = this->findChild<QActionGroup*>();
			if (!_currentView->viewProperty(SettingsPrivate::VP_OwnWindow)) {
				settingsPrivate->setLastActiveViewGeometry(actionGroup->checkedAction()->objectName(), this->saveGeometry());
			}
			settingsPrivate->sync();
		}
	});
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
	} /// FIXME
	/*else if (objectName == "showTabLibrary" || objectName == "showTabFilesystem") {
		leftTabs->setShortcut(objectName, keySequence);
	} else if (objectName == "sendToCurrentPlaylist") {
		library->sendToCurrentPlaylist->setKey(keySequence);
	} else if (objectName == "sendToTagEditor") {
		library->openTagEditor->setKey(keySequence);
	} else if (objectName == "search") {
		searchBar->shortcut->setKey(keySequence);
	}*/
}

void MainWindow::musicLocationsHaveChanged(const QStringList &oldLocations, const QStringList &newLocations)
{
	qDebug() << Q_FUNC_INFO << oldLocations << newLocations;
	bool libraryIsEmpty = newLocations.isEmpty();
	actionScanLibrary->setDisabled(libraryIsEmpty);

	auto db = SqlDatabase::instance();
	if (libraryIsEmpty) {
		db->rebuild(oldLocations, QStringList());
		initQuickStart();

	} else {
		db->rebuild(oldLocations, newLocations);
		this->activateLastView();
	}
}

void MainWindow::showTagEditor()
{
	if (actionViewTagEditor->isChecked()) {
		/// XXX
		// Tag editor is opened, closing it
		if (_tagEditor) {
			_tagEditor->deleteLater();
			_tagEditor = nullptr;
		}
		_tagEditor = new TagEditor;
		_tagEditor->installEventFilter(this);
		_tagEditor->show();
		_tagEditor->activateWindow();
	} else {
		if (_tagEditor) {
			_tagEditor->deleteLater();
			_tagEditor = nullptr;
		}
	}
}

void MainWindow::toggleMenuBar(bool checked)
{
	menuBar()->setVisible(!checked);
	SettingsPrivate::instance()->setValue("isMenuHidden", checked);
}
