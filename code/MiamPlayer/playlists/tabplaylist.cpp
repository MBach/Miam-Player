#include "tabplaylist.h"

#include <QAudio>
#include <QApplication>
#include <QFileSystemModel>
#include <QHeaderView>

#include "tabbar.h"
#include "../treeview.h"
#include "settings.h"

#include <QEventLoop>

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent) :
	QTabWidget(parent), _tabIndex(-1), _closePlaylistPopup(new ClosePlaylistPopup(this))
{
	TabBar *tabBar = new TabBar(this);
	this->setTabBar(tabBar);
	///FIXME
	//this->setStyleSheet(Settings::getInstance()->styleSheet(this));
	this->setDocumentMode(true);
	//_watcher = new QFileSystemWatcher(this);
	messageBox = new TracksNotFoundMessageBox(this);

	Settings *settings = Settings::getInstance();

	// Add a new playlist
	connect(this, &QTabWidget::currentChanged, this, &TabPlaylist::checkAddPlaylistButton);

	connect(qApp, &QCoreApplication::aboutToQuit, [=]() {
		if (playlists().size() == 1 && playlist(0)->mediaPlaylist()->isEmpty()) {
			settings->remove("columnStateForPlaylist");
		} else {
			for (int i = 0; i < playlists().size(); i++) {
				settings->saveColumnStateForPlaylist(i, playlist(i)->horizontalHeader()->saveState());
			}
		}
	});

	// Removing a playlist
	connect(_closePlaylistPopup->buttonBox, &QDialogButtonBox::clicked, this, &TabPlaylist::execActionFromClosePopup);

	connect(this, &QTabWidget::tabCloseRequested, [=] (int index) {
		if (playlists().at(index)->mediaPlaylist()->isEmpty()) {
			this->removeTabFromCloseButton(index);
		} else {
			Settings::PlaylistDefaultAction action = settings->playbackDefaultActionForClose();
			switch (action) {
			case Settings::AskUserForAction:
				_closePlaylistPopup->setTabToClose(index);
				_closePlaylistPopup->show();
				break;
			case Settings::SaveOnClose:
				emit aboutToSavePlaylist(index);
				break;
			case Settings::DiscardOnClose:
				this->removeTabFromCloseButton(index);
				break;
			}
		}
	});

	/// TODO
	//connect(_watcher, &QFileSystemWatcher::fileChanged, [=](const QString &file) {
	//	qDebug() << "file has changed:" << file;
	//});
}

void TabPlaylist::setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
}

/** Retranslate tabs' name and all playlists in this widget. */
void TabPlaylist::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// No translation for the (+) tab button
		for (int i = 0; i < count() - 1; i++) {
			QString playlistName = tr("Playlist ");
			playlistName.append(QString::number(i + 1));
			if (tabText(i) == playlistName) {
				this->setTabText(i, playlistName);
			}
		}
		_closePlaylistPopup->retranslateUi(_closePlaylistPopup);
	}
}

/** Add a new playlist tab. */
Playlist* TabPlaylist::addPlaylist()
{
	QString newPlaylistName = tr("Playlist ").append(QString::number(count()));

	// Then append a new empty playlist to the others
	Playlist *p = new Playlist(this, _mediaPlayer);
	int i = insertTab(count(), p, newPlaylistName);

	// If there's a custom stylesheet on the playlist, copy it from the previous one
	if (i > 1) {
		Playlist *previous = this->playlist(i - 1);
		p->setStyleSheet(previous->styleSheet());
		/// FIXME: stylesheet should be for Class, not instances
		p->horizontalHeader()->setStyleSheet(previous->horizontalHeader()->styleSheet());
	}

	// Select the new empty playlist
	setCurrentIndex(i);
	emit created();
	return p;
}


/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	/*bool isEmpty = */ this->currentPlayList()->mediaPlaylist()->isEmpty();
	foreach (QDir folder, folders) {
		QDirIterator it(folder, QDirIterator::Subdirectories);
		QList<QMediaContent> medias;
		while (it.hasNext()) {
			medias.append(QMediaContent(QUrl::fromLocalFile(it.next())));
		}
		this->currentPlayList()->insertMedias(currentPlayList()->model()->rowCount(), medias);
	}
	// Automatically plays the first track
	/*if (isEmpty) {
		this->skip();
	}*/
}

/** Append a single track chosen by one from the library or the filesystem into the active playlist. */
void TabPlaylist::appendItemToPlaylist(const QString &track)
{
	QList<QString> tracks;
	tracks.append(track);
	this->insertItemsToPlaylist(-1, tracks);
}

/** Insert multiple tracks chosen by one from the library or the filesystem into a playlist. */
void TabPlaylist::insertItemsToPlaylist(int rowIndex, const QStringList &tracks)
{
	currentPlayList()->insertMedias(rowIndex, tracks);
}

void TabPlaylist::moveTracksDown()
{
	if (currentPlayList()) {
		currentPlayList()->moveTracksDown();
	}
}

void TabPlaylist::moveTracksUp()
{
	if (currentPlayList()) {
		currentPlayList()->moveTracksUp();
	}
}

void TabPlaylist::removeCurrentPlaylist()
{
	// Simulate a click on the close button
	emit tabCloseRequested(this->tabBar()->currentIndex());
}

void TabPlaylist::removeSelectedTracks()
{
	if (currentPlayList()) {
		currentPlayList()->removeSelectedTracks();
	}
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	qDebug() << Q_FUNC_INFO;
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 2)) {
		// If the playlist we want to delete has no more right tab, then pick the left tab
		if (index + 1 > count() - 2) {
			setCurrentIndex(index - 1);
		}
		Playlist *p = playlist(index);
		this->removeTab(index);
		delete p;
		emit destroyed(index);
	} else {
		// Clear the content of first tab
		currentPlayList()->mediaPlaylist()->clear();
		currentPlayList()->model()->removeRows(0, currentPlayList()->model()->rowCount());
		tabBar()->setTabText(0, tr("Playlist %1").arg(1));
	}
}

void TabPlaylist::updateRowHeight()
{
	Settings *settings = Settings::getInstance();
	for (int i = 0; i < count() - 1; i++) {
		Playlist *p = playlist(i);
		p->verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(Settings::PLAYLIST)).height());
		p->highlightCurrentTrack();
	}
}

/** When the user is clicking on the (+) button to add a new playlist. */
void TabPlaylist::checkAddPlaylistButton(int i)
{
	// The (+) button is the last tab
	if (i == count() - 1) {
		addPlaylist();
	} else {
		//currentPlayList()->countSelectedItems();
		emit updatePlaybackModeButton();
	}
}

void TabPlaylist::execActionFromClosePopup(QAbstractButton *action)
{
	switch(_closePlaylistPopup->buttonBox->standardButton(action)) {
	case QDialogButtonBox::Save:
		qDebug() << "save and delete";
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			Settings::getInstance()->setPlaybackDefaultActionForClose(Settings::SaveOnClose);
		}
		emit aboutToSavePlaylist(_closePlaylistPopup->index());
		break;
	case QDialogButtonBox::Discard:
		qDebug() << "discard and delete";
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			Settings::getInstance()->setPlaybackDefaultActionForClose(Settings::DiscardOnClose);
		}
		this->removeTabFromCloseButton(_closePlaylistPopup->index());
		_closePlaylistPopup->hide();
		break;
	default:
		qDebug() << "cancel and keep";
		break;
	}
}
