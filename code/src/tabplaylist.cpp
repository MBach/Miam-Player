#include "tabplaylist.h"
#include "library/libraryitem.h"
#include "settings.h"

#include <QAudio>
#include <QApplication>
#include <QFileSystemModel>
#include <QHeaderView>

#include "tabbar.h"
#include "treeview.h"

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent) :
	QTabWidget(parent)
{
	TabBar *tabBar = new TabBar(this);
	this->setTabBar(tabBar);
	this->setStyleSheet(Settings::getInstance()->styleSheet(this));
	this->setDocumentMode(true);
	messageBox = new TracksNotFoundMessageBox(this);
	_mediaPlayer = new QMediaPlayer(this);

	// Link core mp3 actions
	/// FIXME Qt5
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &TabPlaylist::stateChanged);
	//connect(metaInformationResolver, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(metaStateChanged(QMediaPlayer::State)));
	//connect(mediaObject, SIGNAL(finished()), this, SLOT(skipForward()));

	// Keep playlists in memory before exit
	connect(qApp, &QCoreApplication::aboutToQuit, this, &TabPlaylist::savePlaylists);

	// Add a new playlist
	connect(this, &QTabWidget::currentChanged, this, &TabPlaylist::checkAddPlaylistButton);

	// Other actions
	connect(this, &QTabWidget::tabCloseRequested, this, &TabPlaylist::removeTabFromCloseButton);
}

/** Retranslate tabs' name and all playlists in this widget. */
void TabPlaylist::retranslateUi()
{
	// No translation for the (+) tab button
	for (int i=0; i < count()-1; i++) {
		QString playlistName = tr("Playlist ");
		playlistName.append(QString::number(i+1));
		if (tabText(i) == playlistName) {
			setTabText(i, playlistName);
		}
		playlist(i)->retranslateUi();
	}
}

/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	/// FIXME Qt5
	//bool isEmpty = this->currentPlayList()->playlistModel()->tracks().isEmpty();
	/*bool isEmpty = (this->currentPlayList()->playlistModel()->rowCount() == 0);
	foreach (QDir folder, folders) {
		QDirIterator it(folder, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			//this->currentPlayList()->playlistModel()->append(MediaSource(it.next()));
		}
	}
	// Automatically plays the first track
	if (isEmpty) {
		this->skipForward();
	}*/
}

/** Add multiple tracks chosen by one from the library or the filesystem into a playlist. */
void TabPlaylist::addItemsToPlaylist(const QList<QPersistentModelIndex> &indexes, Playlist *playlist, int row)
{
	// Select the playlist
	if (playlist == NULL) {
		playlist = this->currentPlayList();
	} else {
		this->setCurrentWidget(playlist);
	}
	/// FIXME Qt5
	//bool isEmpty = (this->currentPlayList()->playlistModel()->rowCount() == 0);
	bool isEmpty = true;
	// Append tracks
	foreach (QPersistentModelIndex index, indexes) {
		if (index.isValid()) {
			/// FIXME Qt5
			QMediaContent source(QUrl::fromLocalFile(TreeView::absFilePath(index)));
			if (row == -1) {
				row++;
			}
			bool b = playlist->mediaPlaylist()->insertMedia(row, source);
			qDebug() << "add?" << b << row;
		}
	}

	// Automatically plays the first track
	if (isEmpty) {
		this->skipForward();
	}
}

/** Add a single track chosen by one from the library or the filesystem into the active playlist. */
void TabPlaylist::addItemToPlaylist(const QModelIndex &index)
{
	QList<QPersistentModelIndex> indexes;
	indexes.append(index);
	this->addItemsToPlaylist(indexes, currentPlayList());
}

/** Add a new playlist tab. */
Playlist* TabPlaylist::addPlaylist(const QString &playlistName)
{
	// Get the next label for the playlist
	QString newPlaylistName;
	if (playlistName.isEmpty()) {
		newPlaylistName = tr("Playlist ").append(QString::number(count()));
	} else {
		newPlaylistName = playlistName;
	}

	// Then append a new empty playlist to the others
	Playlist *p = new Playlist(this, _mediaPlayer);
	p->init();
	int i = insertTab(count(), p, newPlaylistName);

	// If there's a custom stylesheet on the playlist, copy it from the previous one
	if (i > 1) {
		Playlist *previous = this->playlist(i - 1);
		p->setStyleSheet(previous->styleSheet());
		p->horizontalHeader()->setStyleSheet(previous->horizontalHeader()->styleSheet());
	}
	// Forward this signal to the MainWindow instance
	connect(p, SIGNAL(selectedTracks(int)), this, SIGNAL(aboutToChangeMenuLabels(int)));

	// Select the new empty playlist
	setCurrentIndex(i);
	emit created();
	return p;
}

/** When the user is double clicking on a track in a playlist. */
void TabPlaylist::changeTrack(const QModelIndex &item)
{
	/// FIXME Qt5
	//MediaSource media = currentPlayList()->track(item.row());
	//currentPlayList()->playlistModel()->setActiveTrack(item.row());
	//mediaObject->setCurrentSource(media);
	qDebug() << "ici";
	currentPlayList()->highlightCurrentTrack();
	/// FIXME
	// Autoscrolling is enabled only when skiping a track (or when current track is finished)
	//if (autoscroll) {
		currentPlayList()->scrollTo(item, QAbstractItemView::PositionAtCenter);
	//}
	_mediaPlayer->play();
}

/** When the user is clicking on the (+) button to add a new playlist. */
void TabPlaylist::checkAddPlaylistButton(int i)
{
	// The (+) button is the last tab
	if (i == count()-1) {
		addPlaylist();
	} else {
		currentPlayList()->countSelectedItems();
	}
}

/** Action sent from the menu. */
void TabPlaylist::removeCurrentPlaylist()
{
	removeTabFromCloseButton(this->tabBar()->currentIndex());
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 2)) {
		// If the playlist we want to delete has no more right tab, then pick the left tab
		if (index+1 > count()-2) {
			setCurrentIndex(index-1);
		}
		/// FIXME: when closing a tab which is playing
		/// call mediaObject->clear() [asynchronous ; see how stateChanged() can delete it safely ]
		Playlist *p = playlist(index);
		this->removeTab(index);
		delete p;
		emit destroyed(index);
	} else {
		// Clear the content of last tab
		//currentPlayList()->playlistModel()->clear();
	}
}

/** Restore playlists at startup. */
void TabPlaylist::restorePlaylists()
{
	Settings *settings = Settings::getInstance();
	if (settings->playbackKeepPlaylists()) {
		QList<QVariant> playlists = settings->value("playlists").toList();
		if (!playlists.isEmpty()) {
			QStringList tracksNotFound;

			// For all playlists (stored as a pair of { QList<QVariant[Str=track]> ; QVariant[Str=playlist name] }
			for (int i = 0; i < playlists.size(); i++) {
				QList<QVariant> vTracks = playlists.at(i++).toList();
				//Playlist *p = this->addPlaylist(playlists.at(i).toString());

				// For all tracks in one playlist
				foreach(QVariant vTrack, vTracks) {

					// Check if the file is still on the disk before appending
					QString track = vTrack.toString();
					if (QFile::exists(track)) {
						/// FIXME Qt5
						//p->playlistModel()->append(MediaSource(track));
					} else {
						tracksNotFound << track;
					}
				}
			}
			// Error handling
			if (!tracksNotFound.isEmpty()) {
				messageBox->displayError(TracksNotFoundMessageBox::RESTORE_AT_STARTUP, tracksNotFound);
			}
		} else {
			this->addPlaylist();
		}
	} else {
		this->addPlaylist();
	}
}

/** Seek backward in the current playing track for a small amount of time. */
void TabPlaylist::seekBackward()
{
	/*if (mediaObject->state() == PlayingState || mediaObject->state() == PausedState) {
		qint64 time = mediaObject->currentTime() - Settings::getInstance()->playbackSeekTime();
		if (time < 0) {
			mediaObject->seek(0);
		} else {
			mediaObject->seek(time);
		}
	}*/
}

/** Seek forward in the current playing track for a small amount of time. */
void TabPlaylist::seekForward()
{
	/*if (mediaObject->state() == PlayingState || mediaObject->state() == PausedState) {
		qint64 time = mediaObject->currentTime() + Settings::getInstance()->playbackSeekTime();
		if (time > mediaObject->totalTime()) {
			skipForward();
		} else {
			mediaObject->seek(time);
		}
	}*/
}

/** Change the current track to the previous one. */
void TabPlaylist::skipBackward()
{
	/*int activeTrack = currentPlayList()->playlistModel()->activeTrack();
	if (activeTrack-- > 0) {
		QStandardItem *item = currentPlayList()->playlistModel()->item(activeTrack, 1);
		this->changeTrack(item->index(), true);
	}*/
}

/** Change the current track to the next one. */
void TabPlaylist::skipForward()
{
	/*int next;
	if (Settings::getInstance()->repeatPlayBack() &&
			currentPlayList()->playlistModel()->activeTrack() == this->currentPlayList()->playlistModel()->rowCount() - 1) {
		next = -1;
	} else {
		next = currentPlayList()->playlistModel()->activeTrack();
	}
	if (++next < currentPlayList()->playlistModel()->rowCount()) {
		QStandardItem *item = currentPlayList()->playlistModel()->item(next, 1);
		this->changeTrack(item->index(), true);
	}*/
}

/** Save playlists before exit. */
void TabPlaylist::savePlaylists()
{
	Settings *settings = Settings::getInstance();
	if (settings->playbackKeepPlaylists()) {
		QList<QVariant> vPlaylists;
		// Iterate on all playlists, except empty ones and the last one (the (+) button)
		for (int i = 0; i < count() - 1; i++) {
			Playlist *p = playlist(i);
			/// FIXME Qt5
			/*QList<MediaSource> t;
			for (int j = 0; j < p->playlistModel()->rowCount(); j++) {
				t.append(p->track(j));
			}
			//QListIterator<MediaSource> tracks(p->playlistModel()->tracks());
			QListIterator<MediaSource> tracks(t);
			QList<QVariant> vTracks;
			while (tracks.hasNext()) {
				vTracks.append(tracks.next().fileName());
			}
			if (!vTracks.isEmpty()) {
				vPlaylists.append(QVariant(vTracks));
				vPlaylists.append(QVariant(tabBar()->tabText(i)));
			}*/
		}
		// Tracks are stored in QList< QList<QVariant> >
		settings->setValue("playlists", vPlaylists);
		settings->sync();
	}
}

void TabPlaylist::tick(qint64 time)
{
	//QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);
	// TODO
	//timeLcd->display(displayTime.toString("mm:ss"));
}

void TabPlaylist::stateChanged(QMediaPlayer::State newState)
{
	/*switch (newState) {
	case ErrorState:
		if (mediaObject->errorType() == FatalError) {
			QMessageBox::warning(this, tr("Fatal Error"),
			mediaObject->errorString());
		} else {
			QMessageBox::warning(this, tr("Error"),
			mediaObject->errorString());
		}
		break;

	case BufferingState:
		break;

	case PlayingState:
	case PausedState:
		emit iconStatusChanged(newState);
		break;

	case StoppedState:
		if (oldState == LoadingState) {
			// Play media only if one has not removed the playlist meanwhile, otherwise,  playlist
			if (currentPlayList() && currentPlayList()->playlistModel()->rowCount() == 0) {
				/// todo clear mediaObject!
			} else {
				mediaObject->play();
			}
		}
		break;
	default:
		;
	}*/
}

void TabPlaylist::metaStateChanged(QMediaPlayer::State newState)
{
	/*if (newState == ErrorState) {
		QMessageBox::warning(this, tr("Error opening files"), metaInformationResolver->errorString());
		//while (!sources.isEmpty() && !(sources.takeLast() == metaInformationResolver->currentSource())) {
		// TODO
		//};
		return;
	}

	if (newState != StoppedState && newState != PausedState) {
		return;
	}

	if (metaInformationResolver->currentSource().type() == MediaSource::Invalid) {
		return;
	}

	if (!currentPlayList()->selectionModel()->hasSelection()) {
		currentPlayList()->selectRow(0);
		mediaObject->setCurrentSource(metaInformationResolver->currentSource());
	}

	/// TODO: code review
	int index = currentPlayList()->playlistModel()->activeTrack();
	if (currentPlayList()->playlistModel()->rowCount() > index) {
		//metaInformationResolver->setCurrentSource(currentPlayList()->tracks().at(index));
	} else {
		currentPlayList()->resizeColumnsToContents();
		if (currentPlayList()->columnWidth(0) > 300) {
			currentPlayList()->setColumnWidth(0, 300);
		}
	}*/
}
