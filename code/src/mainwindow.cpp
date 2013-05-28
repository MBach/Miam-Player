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
	mediaButtons << skipBackwardButton << seekBackwardButton << playButton << pauseButton << stopButton;
	mediaButtons << seekForwardButton << skipForwardButton << playbackModeButton;
	/*foreach (MediaButton *b, mediaButtons) {
		b->setStyleSheet(settings->styleSheet(b));
	}*/

	pauseButton->hide();

	// Init the audio module
	audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice());
	tabPlaylists->mediaPlayer()->setVolume(settings->volume());
	volumeSlider->setValue(settings->volume());

	// Init shortcuts
	QMap<QString, QVariant> shortcutMap = settings->shortcuts();
	QMapIterator<QString, QVariant> it(shortcutMap);
	while (it.hasNext()) {
		it.next();
		this->bindShortcut(it.key(), it.value().toInt());
	}

	// Instantiate dialogs
	customizeThemeDialog = new CustomizeThemeDialog(this);
	customizeOptionsDialog = new CustomizeOptionsDialog(this);
	playlistManager = new PlaylistManager(tabPlaylists);
	dragDropDialog = new DragDropDialog(this);
	playbackModeWidgetFactory = new PlaybackModeWidgetFactory(this, playbackModeButton, tabPlaylists);

	// Tag Editor
	tagEditor->hide();
}

void MainWindow::init()
{
	// Load playlists at startup if any, otherwise just add an empty one
	this->setupActions();
	this->drawLibrary();

	Settings *settings = Settings::getInstance();
	this->restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
	splitter->restoreState(settings->value("splitterState").toByteArray());
	leftTabs->setCurrentIndex(settings->value("leftTabsIndex").toInt());

	// Init the address bar
	addressBar->init(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
}

/** Set up all actions and behaviour. */
void MainWindow::setupActions()
{
	// Load music
	connect(customizeOptionsDialog, SIGNAL(musicLocationsHaveChanged(bool)), this, SLOT(drawLibrary(bool)));

	// Link user interface
	// Actions from the menu
    connect(actionExit, &QAction::triggered, &QApplication::quit);
	connect(actionAddPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::addPlaylist);
	connect(actionDeleteCurrentPlaylist, &QAction::triggered, tabPlaylists, &TabPlaylist::removeCurrentPlaylist);
	connect(actionShowCustomize, &QAction::triggered, customizeThemeDialog, &QDialog::open);
	connect(actionShowOptions, &QAction::triggered, customizeOptionsDialog, &QDialog::open);
	connect(actionAboutM4P, &QAction::triggered, this, &MainWindow::aboutM4P);
    connect(actionAboutQt, &QAction::triggered, &QApplication::aboutQt);
	connect(actionScanLibrary, &QAction::triggered, this, &MainWindow::drawLibrary);

	// Quick Start
	connect(quickStart->commandLinkButtonLibrary, &QAbstractButton::clicked, [=] () {
		customizeOptionsDialog->listWidget->setCurrentRow(0);
		customizeOptionsDialog->open();
	});

	connect(quickStart->quickStartApplyButton, &QDialogButtonBox::clicked, [=] (QAbstractButton *) {
		for (int i = 1; i < quickStart->quickStartTableWidget->rowCount(); i++) {
			if (quickStart->quickStartTableWidget->item(i, 0)->checkState() == Qt::Checked) {
				QString musicLocation = quickStart->quickStartTableWidget->item(i, 1)->data(Qt::UserRole).toString();
				customizeOptionsDialog->addMusicLocation(musicLocation);
			}
		}
		this->drawLibrary(true);
		quickStart->hide();
	});

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

	// Media buttons
	connect(tabPlaylists->mediaPlayer(), &QMediaPlayer::stateChanged, this, &MainWindow::stateChanged);
	connect(skipBackwardButton, &QAbstractButton::clicked, [=] () {
		tabPlaylists->skip(false);
	});
	connect(seekBackwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::seekBackward);
	connect(playButton, &QAbstractButton::clicked, tabPlaylists->mediaPlayer(), &QMediaPlayer::play);
	connect(stopButton, &QAbstractButton::clicked, tabPlaylists->mediaPlayer(), &QMediaPlayer::stop);
	connect(seekForwardButton, &QAbstractButton::clicked, tabPlaylists, &TabPlaylist::seekForward);
	connect(skipForwardButton, &QAbstractButton::clicked, [=] () { tabPlaylists->skip(); });
	connect(playbackModeButton, &MediaButton::mediaButtonChanged, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);

	// Sliders
	connect(tabPlaylists->mediaPlayer(), &QMediaPlayer::positionChanged, [=] (qint64 pos) {
		if (tabPlaylists->mediaPlayer()->duration() > 0) {
			seekSlider->setValue(1000 * pos / tabPlaylists->mediaPlayer()->duration());
		}
	});
	connect(seekSlider, &QSlider::sliderMoved, [=] (int pos) {
		tabPlaylists->mediaPlayer()->blockSignals(true);
		tabPlaylists->mediaPlayer()->setPosition(pos * tabPlaylists->mediaPlayer()->duration() / 1000);
		tabPlaylists->mediaPlayer()->blockSignals(false);
	});

	// Volume
	connect(volumeSlider, &QSlider::valueChanged, tabPlaylists->mediaPlayer(), &QMediaPlayer::setVolume);

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(searchBar, SIGNAL(textEdited(QString)), library, SLOT(filterLibrary(QString)));

	// Playback
	// Warning: tabPlaylists->restorePlaylists() needs to be exactly at this position!
	connect(tabPlaylists, &TabPlaylist::updatePlaybackModeButton, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::update);
	tabPlaylists->restorePlaylists();
	connect(actionRemoveSelectedTracks, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::removeSelectedTracks);
	connect(actionMoveTrackUp, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::moveTracksUp);
	connect(actionMoveTrackDown, &QAction::triggered, tabPlaylists->currentPlayList(), &Playlist::moveTracksDown);
	connect(actionShowPlaylistManager, &QAction::triggered, playlistManager, &QDialog::open);

	connect(tabPlaylists, &TabPlaylist::aboutToChangeMenuLabels, this, &MainWindow::changeMenuLabels);

	connect(filesystem, &FileSystemTreeView::folderChanged, addressBar, &AddressBar::init);
	connect(addressBar, &AddressBar::pathChanged, filesystem, &FileSystemTreeView::reloadWithNewPath);

	// Drag & Drop actions
	connect(dragDropDialog, SIGNAL(rememberDragDrop(QToolButton*)), customizeOptionsDialog, SLOT(setExternalDragDropPreference(QToolButton*)));
    /// FIXME Qt5
	//connect(dragDropDialog, SIGNAL(aboutToAddExtFoldersToLibrary(QList<QDir>)), library->searchEngine(), SLOT(setLocations(QList<QDir>)));
	//connect(dragDropDialog, SIGNAL(reDrawLibrary()), this, SLOT(drawLibrary()));
	connect(dragDropDialog, SIGNAL(aboutToAddExtFoldersToPlaylist(QList<QDir>)), tabPlaylists, SLOT(addExtFolders(QList<QDir>)));

	connect(playbackModeButton, &QPushButton::clicked, playbackModeWidgetFactory, &PlaybackModeWidgetFactory::togglePlaybackModes);
}

/** Redefined to be able to retransltate User Interface at runtime. */
void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
		quickStart->retranslateUi(quickStart);
		customizeThemeDialog->retranslateUi(customizeThemeDialog);
		customizeOptionsDialog->retranslateUi(customizeOptionsDialog);
		// Also retranslate each playlist which includes columns like "album", "length", ...
		tabPlaylists->retranslateUi();
		library->retranslateUi();
		tagEditor->retranslateUi(tagEditor);

		// (need to be tested with Arabic language)
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
	settings->setVolume(volumeSlider->value());
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

void MainWindow::moveEvent(QMoveEvent *event)
{
	playbackModeWidgetFactory->move();
	QMainWindow::moveEvent(event);
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
	quickStart->setVisible(isEmpty);
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

void MainWindow::stateChanged(QMediaPlayer::State newState)
{
	playButton->disconnect();
	if (newState == QMediaPlayer::PlayingState) {
		playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/pause"));
		connect(playButton, &QAbstractButton::clicked, tabPlaylists->mediaPlayer(), &QMediaPlayer::pause);
		seekSlider->setEnabled(true);
	} else {
		playButton->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/play"));
		connect(playButton, &QAbstractButton::clicked, tabPlaylists->mediaPlayer(), &QMediaPlayer::play);
		seekSlider->setDisabled(newState == QMediaPlayer::StoppedState);
	}
}
