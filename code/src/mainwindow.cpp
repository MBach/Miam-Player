#include <QtDebug>

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileSystemModel>

#include "mainwindow.h"
#include "dialogs/customizethemedialog.h"
#include "playlist.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QMessageBox>

using namespace Phonon;

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

	// Init the audio module
	audioOutput = new AudioOutput(MusicCategory, this);
	audioOutput->setVolume(Settings::getInstance()->volume());
	createPath(tabPlaylists->media(), audioOutput);
	seekSlider->setMediaObject(tabPlaylists->media());
	volumeSlider->setAudioOutput(audioOutput);

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
	addressBar->init(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHaveChanged(bool)), this, SLOT(drawLibrary(bool)));

	// Link user interface
	// Actions from the menu
	connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(actionAddPlaylist, SIGNAL(triggered()), tabPlaylists, SLOT(addPlaylist()));
	connect(actionDeleteCurrentPlaylist, SIGNAL(triggered()), tabPlaylists, SLOT(removeCurrentPlaylist()));
	connect(actionShowCustomize, SIGNAL(triggered()), customizeThemeDialog, SLOT(open()));
	connect(actionShowOptions, SIGNAL(triggered()), customizeOptionsDialog, SLOT(open()));
	connect(actionAboutM4P, SIGNAL(triggered()), this, SLOT(aboutM4P()));
	connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(actionScanLibrary, SIGNAL(triggered()), this, SLOT(drawLibrary()));

	// When no library is set
	connect(commandLinkButtonLibrary, SIGNAL(clicked()), customizeOptionsDialog, SLOT(open()));

	foreach (TreeView *tab, this->findChildren<TreeView*>()) {
		connect(tab, SIGNAL(setTagEditorVisible(bool)), this, SLOT(toggleTagEditor(bool)));
		connect(tab, SIGNAL(aboutToSendToPlaylist(QList<QPersistentModelIndex>, Playlist*, int)), tabPlaylists, SLOT(addItemsToPlaylist(QList<QPersistentModelIndex>, Playlist*, int)));
		connect(tab, SIGNAL(sendToTagEditor(QList<QPersistentModelIndex>)), tagEditor, SLOT(addItemsToEditor(QList<QPersistentModelIndex>)));
	}

	// Send one folder to the music locations
	connect(filesystem, SIGNAL(aboutToAddMusicLocation(QString)), customizeOptionsDialog, SLOT(addMusicLocation(QString)));
	connect(filesystem, SIGNAL(doubleClicked(QModelIndex)), tabPlaylists, SLOT(addItemToPlaylist(QModelIndex)));

	// Send music to the tag editor
	connect(tagEditor, SIGNAL(closeTagEditor(bool)), this, SLOT(toggleTagEditor(bool)));

	// Rebuild the treeview when tracks have changed using the tag editor
	connect(tagEditor, SIGNAL(rebuildTreeView(QList<QPersistentModelIndex>)), library, SLOT(rebuild(QList<QPersistentModelIndex>)));

	// Link media buttons
	Settings *settings = Settings::getInstance();
	connect(skipBackwardButton, SIGNAL(clicked()), tabPlaylists, SLOT(skipBackward()));
	connect(seekBackwardButton, SIGNAL(clicked()), tabPlaylists, SLOT(seekBackward()));
	connect(playButton, SIGNAL(clicked()), this, SLOT(playAndPause()));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(seekForwardButton, SIGNAL(clicked()), tabPlaylists, SLOT(seekForward()));
	connect(skipForwardButton, SIGNAL(clicked()), tabPlaylists, SLOT(skipForward()));
	connect(repeatButton, SIGNAL(toggled(bool)), settings, SLOT(setRepeatPlayBack(bool)));
	connect(shuffleButton, SIGNAL(toggled(bool)), settings, SLOT(setShufflePlayBack(bool)));
	connect(volumeSlider->audioOutput(), SIGNAL(volumeChanged(qreal)), settings, SLOT(setVolume(qreal)));

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, SIGNAL(textEdited(QString)), library, SLOT(filterLibrary(QString)));

	// Playback
	connect(actionRemoveSelectedTracks, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(removeSelectedTracks()));
	connect(actionMoveTrackUp, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(moveTracksUp()));
	connect(actionMoveTrackDown, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(moveTracksDown()));
	connect(actionShowPlaylistManager, SIGNAL(triggered()), playlistManager, SLOT(open()));

	connect(tabPlaylists, SIGNAL(aboutToChangeMenuLabels(int)), this, SLOT(changeMenuLabels(int)));

	connect(filesystem, SIGNAL(folderChanged(QString)), addressBar, SLOT(init(QString)));
	connect(addressBar, SIGNAL(pathChanged(QString)), filesystem, SLOT(reloadWithNewPath(QString)));

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

void MainWindow::closeEvent(QCloseEvent *event)
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
	if (!tabPlaylists->currentPlayList()->playlistModel()->rowCount() == 0) {
		if (tabPlaylists->media()->state() == PlayingState) {
			tabPlaylists->media()->pause();
			playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/pause"), true);
		} else {
			tabPlaylists->media()->play();
			playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/play"), true);
		}
	}
}

void MainWindow::stop()
{
	tabPlaylists->media()->stop();
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
		tagEditor->clear();
	}
	tagEditor->setVisible(b);
	seekSlider->setVisible(!b);
	tabPlaylists->setVisible(!b);
}
