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
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << pauseButton
				 << stopButton << seekForwardButton << skipForwardButton << repeatButton << shuffleButton;
	pauseButton->hide();

	customizeThemeDialog = new CustomizeThemeDialog(this);
	customizeOptionsDialog = new CustomizeOptionsDialog(this);

	// Init the audio module
	audioOutput = new AudioOutput(MusicCategory, this);
	audioOutput->setVolume(Settings::getInstance()->volume());
	createPath(tabPlaylists->media(), audioOutput);
	seekSlider->setMediaObject(tabPlaylists->media());
	volumeSlider->setAudioOutput(audioOutput);

	// Init shortcuts
	Settings *settings = Settings::getInstance();
	QMap<QString, QVariant> shortcutMap = settings->shortcuts();
	QMapIterator<QString, QVariant> it(shortcutMap);
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().toInt());
	}

	// Init checkable buttons
	actionRepeat->setChecked(settings->repeatPlayBack());
	actionShuffle->setChecked(settings->shufflePlayBack());

	addPlaylist();
	setupActions();
	drawLibrary();
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHasChanged(bool)), this, SLOT(drawLibrary(bool)));

	// Link user interface
	// Actions from the menu
	connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
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
	connect(library, SIGNAL(sendToPlaylist(const QPersistentModelIndex &)), tabPlaylists, SLOT(addItemFromLibraryToPlaylist(const QPersistentModelIndex &)));
	connect(filesystem, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addSelectedItemToPlaylist(const QModelIndex &)));

	// Add a new playlist
	connect(tabPlaylists, SIGNAL(currentChanged(int)), this, SLOT(checkAddPlaylistButton(int)));

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

/** When the user is clicking on the (+) button to add a new playlist. */
void MainWindow::checkAddPlaylistButton(int i)
{
	// The (+) button is the last tab
	if (i == tabPlaylists->count()-1) {
		addPlaylist();
	}
}

/** This buttons switch the play function with the pause function because they are mutually exclusive. */
void MainWindow::playAndPause()
{
	if (!tabPlaylists->currentPlayList()->tracks()->isEmpty()) {
		MediaButton *b;
		if (tabPlaylists->media()->state() == Phonon::PlayingState) {
			tabPlaylists->media()->pause();
			b = playButton;
		} else {
			tabPlaylists->media()->play();
			b = pauseButton;
		}
		qDebug() << b->objectName();
		playButton->setIcon(b->icon(), true);
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
