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
	this->setWindowIcon(QIcon(":/icons/mmmmp.ico"));
	this->filesystem->header()->setResizeMode(QHeaderView::ResizeToContents);

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
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << stopButton << seekForwardButton << skipForwardButton;

	// Order is important?
	customizeThemeDialog = new CustomizeThemeDialog(this);
	customizeOptionsDialog = new CustomizeOptionsDialog(this);

	// Init the audio module
	audioOutput = new AudioOutput(MusicCategory, this);
	createPath(tabPlaylists->media(), audioOutput);
	seekSlider->setMediaObject(tabPlaylists->media());
	volumeSlider->setAudioOutput(audioOutput);

	// Init the theme
	customizeThemeDialog->loadTheme();

	addPlaylist();
	setupActions();
	//showFirstRun();
	drawLibrary();
	//library->beginPopulateTree();
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeThemeDialog, SIGNAL(libraryNeedToBeRepaint()), this, SLOT(drawLibrary()));
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHasChanged(bool)), this, SLOT(drawLibrary(bool)));

	// Link user interface
	// Actions from the menu
	connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(actionAddPlaylist, SIGNAL(triggered()), this, SLOT(addPlaylist()));
	connect(actionDeleteCurrentPlaylist, SIGNAL(triggered()), tabPlaylists, SLOT(removeCurrentPlaylist()));
	connect(actionShowCustomize, SIGNAL(triggered()), customizeThemeDialog, SLOT(open()));
	connect(actionShowOptions, SIGNAL(triggered()), customizeOptionsDialog, SLOT(open()));	
	connect(actionAboutM4P, SIGNAL(triggered()), this, SLOT(aboutM4P()));
	connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(actionScanLibrary, SIGNAL(triggered()), this, SLOT(drawLibrary()));

	// When no library is set
	connect(commandLinkButtonLibrary, SIGNAL(clicked()), customizeOptionsDialog, SLOT(open()));

	// Send music to playlist
	connect(library, SIGNAL(sendToPlaylist(const QPersistentModelIndex &)), this, SLOT(addItemFromLibraryToPlaylist(const QPersistentModelIndex &)));
	connect(filesystem, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addSelectedItemToPlaylist(const QModelIndex &)));

	// Add a new playlist
	connect(tabPlaylists, SIGNAL(currentChanged(int)), this, SLOT(checkAddPlaylistButton(int)));

	// Change track
	connect(tabPlaylists->currentPlayList()->table(), SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(changeTrack(QTableWidgetItem*)));

	// Link buttons
	connect(playButton, SIGNAL(clicked()), this, SLOT(playAndPause()));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(skipBackwardButton, SIGNAL(clicked()), this, SLOT(skipBackward()));
	connect(skipForwardButton, SIGNAL(clicked()), this, SLOT(skipForward()));

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, SIGNAL(textEdited(QString)), library, SLOT(filterLibrary(QString)));

	// TEST
	connect(this, SIGNAL(delegateStateChanged()), customizeThemeDialog, SIGNAL(themeChanged()));
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

/** Add a new playlist tab. */
void MainWindow::addPlaylist()
{
	// Get the next label for the playlist
	QString newPlaylistName = QString(tr("Playlist ")).append(QString::number(tabPlaylists->count()));
	Playlist *playlist = new Playlist(tabPlaylists);

	// Then append a new empty playlist to the others
	int i = tabPlaylists->insertTab(tabPlaylists->count(), playlist, newPlaylistName);

	// Select the new empty playlist
	tabPlaylists->setCurrentIndex(i);
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

void MainWindow::addItemFromLibraryToPlaylist(const QPersistentModelIndex &item)
{
	bool isEmpty = tabPlaylists->currentPlayList()->tracks()->isEmpty();
	QTableWidgetItem *indexInPlaylist = tabPlaylists->addItemToCurrentPlaylist(item);
	qDebug() << "row: " << indexInPlaylist->row();
	if (isEmpty) {
		//tabPlaylists->media()->setCurrentSource();
		changeTrack(indexInPlaylist);
	}
}


/** When the user is clicking on the (+) button to add a new playlist. */
void MainWindow::checkAddPlaylistButton(int i)
{
	// The (+) button is the last tab
	if (i == tabPlaylists->count()-1) {
		addPlaylist();
	}
}

/** When the user is double clicking on a track in a playlist. */
void MainWindow::changeTrack(QTableWidgetItem *item)
{
	/*tabPlaylists->currentPlayList()->setActiveTrack(track);
	MediaSource media = tabPlaylists->currentPlayList()->currentTrack();
	tabPlaylists->sourceChanged(media);*/
	//tabPlaylists->currentPlayList()->tracks()->at(item->row());
	MediaSource media = tabPlaylists->currentPlayList()->tracks()->at(item->row());
	qDebug() << "media.fileName():" << media.fileName();
	tabPlaylists->sourceChanged(media);
	playAndPause();
}

/** This buttons switch the play function with the pause function because they are mutually exclusive. */
void MainWindow::playAndPause()
{
	if (!tabPlaylists->currentPlayList()->tracks()->isEmpty()) {
		QIcon icon;
		Settings *settings = Settings::getInstance();
		if (tabPlaylists->media()->state() == Phonon::PlayingState) {
			icon.addFile(":/player/" + settings->theme() + "/play");
			tabPlaylists->media()->pause();
		} else {
			icon.addFile(":/player/" + settings->theme() + "/pause");
			tabPlaylists->media()->play();
		}
		playButton->setIcon(icon);
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

/** Change the current track to the previous one. */
void MainWindow::skipBackward()
{
	QModelIndex activeTrack = tabPlaylists->currentPlayList()->activeTrack();
	if (activeTrack.row() > 0) {
		//changeTrack(activeTrack.sibling(activeTrack.row()-1, 1));
	}
}

/** Change the current track to the next one. */
void MainWindow::skipForward()
{
	QModelIndex activeTrack = tabPlaylists->currentPlayList()->activeTrack();
	int track = -1;
	if (activeTrack.isValid()) {
		track = activeTrack.row();
	} else {
		qDebug() << "no valid track !";
		tabPlaylists->currentPlayList()->tracks();
	}
	if (track+1 < tabPlaylists->currentPlayList()->tracks()->size()) {
		qDebug() << "ici";
		//changeTrack(activeTrack.sibling(track+1, 1));
	}
}

/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
void MainWindow::aboutM4P()
{	
	QString message = tr("This software is a MP3 player very simple to use.<br><br>It does not include extended functionalities like lyrics, or to be connected to the Web. It offers a highly customizable user interface and enables favorite tracks.");
	QMessageBox::about(this, tr("About Mme MiamMiamMusicPlayer"), message);
}

/** Load the user defined language at startup. Called once. */
void MainWindow::loadLanguage()
{
	customizeOptionsDialog->loadLanguage();
}
