#include <QtDebug>

#include <QDesktopServices>
#include <QDirIterator>
#include <QFileSystemModel>

#include "mainwindow.h"
#include "filesystemmodel.h"
#include "customizethemedialog.h"
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

	this->setWindowIcon(QIcon(":/icons/mmmmp.ico"));
	this->filesystem->header()->setResizeMode(QHeaderView::ResizeToContents);
	this->setStyleSheet(settings->styleSheet(this));
	leftTabs->setStyleSheet(settings->styleSheet(leftTabs));
	widgetSearchBar->setStyleSheet(settings->styleSheet(0));

	FileSystemModel *fileSystemModel = new FileSystemModel(this);
	fileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

	QStringList filters;
	filters << "*.mp3";
	fileSystemModel->setNameFilters(filters);

	filesystem->setModel(fileSystemModel);
	filesystem->setRootIndex(fileSystemModel->setRootPath(QDesktopServices::storageLocation(QDesktopServices::MusicLocation)));

	// Hide columns "size" and "date modified" columns, useless for almost everyone
	filesystem->setColumnHidden(1, true);
	filesystem->setColumnHidden(3, true);

	// Special behaviour for media buttons
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << pauseButton
				 << stopButton << seekForwardButton << skipForwardButton << repeatButton << shuffleButton;
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

	customizeThemeDialog = new CustomizeThemeDialog(this);
	customizeOptionsDialog = new CustomizeOptionsDialog(this);
	playlistManager = new PlaylistManager(tabPlaylists);

	this->setupActions();
	this->drawLibrary();
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHasChanged(bool)), this, SLOT(drawLibrary(bool)));

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

	// Send music to playlist
	connect(library, SIGNAL(sendToPlaylist(const QPersistentModelIndex &)), tabPlaylists, SLOT(addItemFromLibraryToPlaylist(const QPersistentModelIndex &)));
	connect(filesystem, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addSelectedItemToPlaylist(const QModelIndex &)));

	// Link buttons
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
	connect(actionRemoveSelectedTrack, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(removeSelectedTrack()));
	connect(actionMoveTrackUp, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(moveTrackUp()));
	connect(actionMoveTrackDown, SIGNAL(triggered()), tabPlaylists->currentPlayList(), SLOT(moveTrackDown()));
	connect(actionShowPlaylistManager, SIGNAL(triggered()), playlistManager, SLOT(open()));
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

		// (need to tested with Arabic language)
		if (tr("LTR") == "RTL") {
			QApplication::setLayoutDirection(Qt::RightToLeft);
		}
	} else {
		QMainWindow::changeEvent(event);
	}
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
	if (!isEmpty) {
		// Warning: This function violates the object-oriented principle of modularity.
		// However, getting access to the sender might be useful when many signals are connected to a single slot.
		if (sender() == actionScanLibrary) {
			b = true;
		}
		library->beginPopulateTree(b);
	}
}

/** Add a file from the filesystem to the current playlist. */
void MainWindow::addSelectedItemToPlaylist(const QModelIndex &item)
{
	FileSystemModel *fileSystemModel = qobject_cast<FileSystemModel *>(filesystem->model());
	// If item is a file
	if (!fileSystemModel->isDir(item)) {
		tabPlaylists->addItemToCurrentPlaylist(item);
	}
}

/** This buttons switch the play function with the pause function because they are mutually exclusive. */
void MainWindow::playAndPause()
{
	if (!tabPlaylists->currentPlayList()->tracks().isEmpty()) {
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
	QMessageBox::about(this, tr("About Mme MiamMiamMusicPlayer"), message);
}
