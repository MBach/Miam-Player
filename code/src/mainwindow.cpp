#include <QtDebug>

#include <QDirIterator>
#include <QFileSystemModel>
#include <QStandardPaths>

#include "mainwindow.h"
#include "dialogs/customizethemedialog.h"
#include "playlist.h"

#include <QtMultimedia/QAudioDeviceInfo>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	setupUi(this);
	Settings *settings = Settings::getInstance();

	this->setAcceptDrops(true);

	this->setWindowIcon(QIcon(":/icons/mmmmp.ico"));
	this->setStyleSheet(settings->styleSheet(this));
	leftTabs->setStyleSheet(settings->styleSheet(leftTabs));
	widgetSearchBar->setStyleSheet(settings->styleSheet(0));
	splitter->setStyleSheet(settings->styleSheet(splitter));
	volumeSlider->setStyleSheet(settings->styleSheet(volumeSlider));
	seekSlider->setStyleSheet(settings->styleSheet(seekSlider));

	// Special behaviour for media buttons
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << pauseButton
				 << stopButton << seekForwardButton << skipForwardButton << repeatButton << shuffleButton;
	foreach (MediaButton *b, mediaButtons) {
		b->setStyleSheet(settings->styleSheet(b));
	}

	pauseButton->hide();

	/// FIXME Qt5
	// Init the audio module
	audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice());
	audioOutput->setVolume(Settings::getInstance()->volume());
	//createPath(tabPlaylists->media(), audioOutput);
	//seekSlider->setMediaObject(tabPlaylists->media());
	//volumeSlider->setAudioOutput(audioOutput);

	// Init shortcuts
	QMap<QString, QVariant> shortcutMap = settings->shortcuts();
	QMapIterator<QString, QVariant> it(shortcutMap);
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().toInt());
	}

	// Init checkable buttons
	actionRepeat->setChecked(settings->repeatPlayBack());
	actionShuffle->setChecked(settings->shufflePlayBack());

	// Load playlists at startup if any, otherwise just add an empty one
	tabPlaylists->restorePlaylists();

	// Instantiate dialogs
	customizeThemeDialog = new CustomizeThemeDialog(this);
	customizeOptionsDialog = new CustomizeOptionsDialog(this);
	playlistManager = new PlaylistManager(tabPlaylists);
	dragDropDialog = new DragDropDialog(this);

	// Tag Editor
	tagEditor->hide();

	this->setupActions();
	this->drawLibrary();
	this->restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
	splitter->restoreState(settings->value("splitterState").toByteArray());
	leftTabs->setCurrentIndex(settings->value("leftTabsIndex").toInt());

	// Init the address bar
	addressBar->init(QStandardPaths::displayName(QStandardPaths::MusicLocation));
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHaveChanged(bool)), this, SLOT(drawLibrary(bool)));

	// Link user interface
	// Actions from the menu
	connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
	/// FIXME Qt5
	//connect(actionAddPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::addPlaylist);
	connect(actionDeleteCurrentPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::removeCurrentPlaylist);
	connect(actionShowCustomize, &QAction::triggered, customizeThemeDialog, &QDialog::open);
	connect(actionShowOptions, &QAction::triggered, customizeOptionsDialog, &QDialog::open);
	connect(actionAboutM4P, &QAction::triggered, this, &MainWindow::aboutM4P);
	connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(actionScanLibrary, &QAction::triggered, this, &MainWindow::drawLibrary);

	// When no library is set
	connect(commandLinkButtonLibrary, &QAbstractButton::clicked, customizeOptionsDialog, &QDialog::open);

	foreach (TreeView *tab, this->findChildren<TreeView*>()) {
		connect(tab, &TreeView::setTagEditorVisible, this, &MainWindow::toggleTagEditor);
		connect(tab, &TreeView::aboutToSendToPlaylist, tabPlaylists, &TabPlaylist::addItemsToPlaylist);
		connect(tab, &TreeView::sendToTagEditor, tagEditor, &TagEditor::addItemsToEditor);
	}

	// Send one folder to the music locations
	connect(filesystem, &FileSystemTreeView::aboutToAddMusicLocation, customizeOptionsDialog, &CustomizeOptionsDialog::addMusicLocation);
	connect(filesystem, &QAbstractItemView::doubleClicked, tabPlaylists, &TabPlaylist::addItemToPlaylist);

	// Send music to the tag editor
	connect(tagEditor, &TagEditor::closeTagEditor, this, &MainWindow::toggleTagEditor);

	// Rebuild the treeview when tracks have changed using the tag editor
	connect(tagEditor, &TagEditor::rebuildTreeView, library, &LibraryTreeView::rebuild);

	// Link media buttons
	Settings *settings = Settings::getInstance();
	connect(skipBackwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::skipBackward);
	connect(seekBackwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::seekBackward);
	connect(playButton, &QAbstractButton::clicked, this, &MainWindow::playAndPause);
	connect(stopButton, &QAbstractButton::clicked, this, &MainWindow::stop);
	connect(seekForwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::seekForward);
	connect(skipForwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::skipForward);
	connect(repeatButton, &QAbstractButton::clicked, settings, &Settings::setRepeatPlayBack);
	connect(shuffleButton, &QAbstractButton::clicked, settings, &Settings::setShufflePlayBack);
	/// FIXME Qt5
	//connect(volumeSlider->audioOutput(), SIGNAL(volumeChanged(qreal)), settings, SLOT(setVolume(qreal)));

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, SIGNAL(textEdited(QString)), library, SLOT(filterLibrary(QString)));

	// Playback
	connect(actionRemoveSelectedTracks, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::removeSelectedTracks);
	connect(actionMoveTrackUp, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::moveTracksUp);
	connect(actionMoveTrackDown, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::moveTracksDown);
	connect(actionShowPlaylistManager, &QAction::triggered, playlistManager, &QDialog::open);

	connect(tabPlaylists, &TabPlaylist::aboutToChangeMenuLabels, this, &MainWindow::changeMenuLabels);

	connect(filesystem, &FileSystemTreeView::folderChanged, addressBar, &AddressBar::init);
	connect(addressBar, &AddressBar::pathChanged, filesystem, &FileSystemTreeView::reloadWithNewPath);

	// Drag & Drop actions
	connect(dragDropDialog, SIGNAL(rememberDragDrop(QToolButton*)), customizeOptionsDialog, SLOT(setExternalDragDropPreference(QToolButton*)));
	/// FIXME
	//connect(dragDropDialog, SIGNAL(aboutToAddExtFoldersToLibrary(QList<QDir>)), library->searchEngine(), SLOT(setLocations(QList<QDir>)));
	//connect(dragDropDialog, SIGNAL(reDrawLibrary()), this, SLOT(drawLibrary()));
	connect(dragDropDialog, SIGNAL(aboutToAddExtFoldersToPlaylist(QList<QDir>)), tabPlaylists, SLOT(addExtFolders(QList<QDir>)));
}

/** Redefined to be able to retransltate User Interface at runtime. */
void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
		customizeThemeDialog->retranslateUi(customizeThemeDialog);
		customizeOptionsDialog->retranslateUi(customizeOptionsDialog);
		// Also retranslate each playlist which includes columns like "album", "length", ...
		tabPlaylists->retranslateUi();
		library->retranslateUi();
		tagEditor->retranslateUi(tagEditor);

		// (need to tested with Arabic language)
		if (tr("LTR") == "RTL") {
			QApplication::setLayoutDirection(Qt::RightToLeft);
		}
	} else {
		QMainWindow::changeEvent(event);
	}
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
	Settings *settings = Settings::getInstance();
	settings->setValue("mainWindowGeometry", saveGeometry());
	settings->setValue("splitterState", splitter->saveState());
	settings->setValue("leftTabsIndex", leftTabs->currentIndex());
	Settings::getInstance()->sync();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	// Ignore Drag & Drop if the source is a part of this player
	if (event->source() != NULL) {
		return;
	}
	dragDropDialog->setMimeData(event->mimeData());

	QRadioButton *radioButtonDD = customizeOptionsDialog->findChild<QRadioButton*>(Settings::getInstance()->dragAndDropBehaviour());
	if (radioButtonDD == customizeOptionsDialog->radioButtonDDAddToLibrary) {
		// TODO
	} else if (radioButtonDD == customizeOptionsDialog->radioButtonDDAddToPlaylist) {
		tabPlaylists->addExtFolders(dragDropDialog->externalLocations());
	} else if (radioButtonDD == customizeOptionsDialog->radioButtonDDOpenPopup) {
		dragDropDialog->show();
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::bindShortcut(const QString &objectName, int keySequence)
{
	Settings::getInstance()->setShortcut(objectName, keySequence);
	QAction *action = findChild<QAction*>("action" + objectName.left(1).toUpper() + objectName.mid(1));
	// Connect actions first
	if (action) {
		action->setShortcut(QKeySequence(keySequence));
	} else {
		// Is this really necessary? Everything should be in the menu
		MediaButton *button = findChild<MediaButton*>(objectName + "Button");
		if (button) {
			button->setShortcut(QKeySequence(keySequence));
		}
	}
}

/** Draw the library widget by calling subcomponents. */
void MainWindow::drawLibrary(bool b)
{
	bool isEmpty = Settings::getInstance()->musicLocations().isEmpty();
	widgetFirstRun->setVisible(isEmpty);
	library->setVisible(!isEmpty);
	actionScanLibrary->setEnabled(!isEmpty);
	widgetSearchBar->setVisible(!isEmpty);
	this->toggleTagEditor(false);
	if (!isEmpty) {
		// Warning: This function violates the object-oriented principle of modularity.
		// However, getting access to the sender might be useful when many signals are connected to a single slot.
		if (sender() == actionScanLibrary) {
			b = true;
		}
		library->beginPopulateTree(b);
	}
}

/** Change the labels like "Remove selected track(s)" depending of the number of selected elements in the current playlist. */
void MainWindow::changeMenuLabels(int itemCount)
{
	bool b = (itemCount > 0);
	actionRemoveSelectedTracks->setEnabled(b);
	actionMoveTrackUp->setEnabled(b);
	actionMoveTrackDown->setEnabled(b);

	if (itemCount <= 1) {
		actionRemoveSelectedTracks->setText(tr("&Remove selected track"));
		actionMoveTrackUp->setText(tr("Move selected track &up"));
		actionMoveTrackDown->setText(tr("Move selected track &down"));
	} else {
		actionRemoveSelectedTracks->setText(tr("&Remove selected tracks"));
		actionMoveTrackUp->setText(tr("Move selected tracks &up"));
		actionMoveTrackDown->setText(tr("Move selected tracks &down"));
	}
}

/** These buttons switch the play function with the pause function because they are mutually exclusive. */
void MainWindow::playAndPause()
{
	/// FIXME Qt5
	/*if (!tabPlaylists->currentPlayList()->playlistModel()->rowCount() == 0) {
		if (tabPlaylists->media()->state() == PlayingState) {
			tabPlaylists->media()->pause();
			playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/pause"), true);
		} else {
			tabPlaylists->media()->play();
			playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/play"), true);
		}
	}*/
}

void MainWindow::stop()
{
	//tabPlaylists->media()->stop();
	Settings *settings = Settings::getInstance();
	QString play(":/player/" + settings->theme() + "/play");
	if (playButton->icon().name() != play) {
		playButton->setIcon(QIcon(play));
	}
}

/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
void MainWindow::aboutM4P()
{
	QString message = tr("This software is a MP3 player very simple to use.<br><br>It does not include extended functionalities like lyrics, or to be connected to the Web. It offers a highly customizable user interface and enables favorite tracks.");
	QMessageBox::about(this, QString("Madame MiamMiam's Music Player v").append(qApp->applicationVersion()), message);
}

void MainWindow::toggleTagEditor(bool b)
{
	if (b) {
		//qDebug() << "MainWindow::toggleTagEditor";
		//tagEditor->clear();
	}
	tagEditor->setVisible(b);
	seekSlider->setVisible(!b);
	tabPlaylists->setVisible(!b);
}
