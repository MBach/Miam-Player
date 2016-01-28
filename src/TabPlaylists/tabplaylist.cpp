#include "tabplaylist.h"

#include <QDirIterator>

#include <settings.h>
#include <settingsprivate.h>

#include "dialogs/closeplaylistpopup.h"
#include "tabbar.h"
#include "cornerwidget.h"
#include <model/sqldatabase.h>

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent)
	: QTabWidget(parent)
	, _mediaPlayer(nullptr)
	, _playlistManager(new PlaylistManager(this))
	, _contextMenu(new QMenu(this))
{
	TabBar *tabBar = new TabBar(this);
	this->setTabBar(tabBar);
	this->setMovable(true);

	// Add a new playlist
	connect(this, &QTabWidget::currentChanged, this, [=]() {
		qDebug() << this->currentPlayList()->mediaPlaylist()->mediaCount();
		QMediaPlaylist::PlaybackMode m = this->currentPlayList()->mediaPlaylist()->playbackMode();
		emit updatePlaybackModeButton(m);
	});

	connect(this, &TabPlaylist::aboutToSavePlaylist, _playlistManager, &PlaylistManager::saveAndRemovePlaylist);
	connect(_playlistManager, &PlaylistManager::aboutToRemovePlaylist, this, &TabPlaylist::removeTabFromCloseButton);

	// Removing a playlist
	connect(this, &QTabWidget::tabCloseRequested, this, &TabPlaylist::closePlaylist);

	connect(tabBar, &TabBar::tabRenamed, this, [=](int index, const QString &text) {
		Playlist *p = playlist(index);
		p->setTitle(text);
		this->setTabIcon(index, this->defaultIcon(QIcon::Normal));
	});

	/// FIXME: when changing font for saved and untouched playlists, overwritting to normal instead of disabled
	/// Reducing size is ok, inreasing size is ko
	/*connect(settings, &SettingsPrivate::fontHasChanged, this, [=](const SettingsPrivate::FontFamily ff, const QFont &) {
		if (ff == SettingsPrivate::FF_Playlist) {
			for (int i = 0; i < count() - 1; i++) {
				if (playlist(i)->mediaPlaylist()->isEmpty()) {
					this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));
				} else {
					this->setTabIcon(i, this->defaultIcon(QIcon::Normal));
				}
			}
		}
	});*/

	auto settings = SettingsPrivate::instance();
	connect(settings, &SettingsPrivate::fontHasChanged, [=](const SettingsPrivate::FontFamily ff, const QFont &font) {
		if (ff == SettingsPrivate::FF_Playlist) {
			int s = QFontMetrics(settings->font(SettingsPrivate::FF_Playlist)).height();
			for (Playlist *playlist : playlists()) {
				for (int i = 0; i < playlist->model()->columnCount(); i++) {
					playlist->model()->setHeaderData(i, Qt::Horizontal, font, Qt::FontRole);
				}
				playlist->verticalHeader()->setDefaultSectionSize(s);
			}
		}
	});

	// Context menu to add few actions for each playlist
	QAction *renamePlaylist = new QAction(tr("Rename playlist"), _contextMenu);
	_deletePlaylist = new QAction(tr("Delete playlist..."), _contextMenu);
	QAction *loadBackground = new QAction(tr("Load background..."), _contextMenu);
	QAction *clearBackground = new QAction(tr("Clear background"), _contextMenu);

	_deletePlaylist->setEnabled(false);
	loadBackground->setEnabled(false);
	clearBackground->setEnabled(false);

	_contextMenu->addAction(renamePlaylist);
	_contextMenu->addAction(_deletePlaylist);
	_contextMenu->addSeparator();
	_contextMenu->addAction(loadBackground);
	_contextMenu->addAction(clearBackground);

	// Rename a playlist
	connect(renamePlaylist, &QAction::triggered, this, [=]() {
		QPoint mrcp = _contextMenu->property("mouseRightClickPos").toPoint();
		int index = tabBar->tabAt(mrcp);
		this->setCurrentIndex(index);
		tabBar->editTab(index);
	});

	// Ask one if he wants to delete a playlist
	connect(_deletePlaylist, &QAction::triggered, this, [=]() {
		QPoint mrcp = _contextMenu->property("mouseRightClickPos").toPoint();
		int index = tabBar->tabAt(mrcp);
		Playlist *p = this->playlist(index);
		QString deleteMessage = tr("You're about to delete '%1'. Are you sure you want to continue?").arg(p->title());
		if (QMessageBox::Ok == QMessageBox::warning(this, tr("Warning"), deleteMessage, QMessageBox::Ok, QMessageBox::Cancel)) {
			this->deletePlaylist(p->id());
		}
	});

	// Add the possibility to draw a custom background for every playlist
	connect(loadBackground, &QAction::triggered, this, [=]() {
		qDebug() << Q_FUNC_INFO << "Load background not implemented yet";
	});
	this->setAcceptDrops(true);

	CornerWidget *corner = new CornerWidget(this);
	this->setCornerWidget(corner, Qt::TopRightCorner);
	connect(corner, &CornerWidget::innerButtonClicked, this, &TabPlaylist::addPlaylist);
	corner->installEventFilter(this);
}

/** Get the current playlist. */
Playlist* TabPlaylist::currentPlayList() const
{
	return qobject_cast<Playlist*>(this->currentWidget());
}

QIcon TabPlaylist::defaultIcon(QIcon::Mode mode)
{
	QIcon icon(":/icons/playlistIcon");
	QIcon displayedIcon(icon.pixmap(QSize(tabBar()->fontMetrics().ascent(), tabBar()->fontMetrics().ascent()), mode));
	return displayedIcon;
}

/** Redefined to forward events to children. */
bool TabPlaylist::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::DragEnter) {
		event->accept();
		return true;
	} else if (event->type() == QEvent::Drop) {
		QDropEvent *de = static_cast<QDropEvent*>(event);
		if (de->source() == nullptr) {
			// Drag & Drop comes from another application but has landed in the playlist area
			de->ignore();
			QDropEvent *d = new QDropEvent(de->pos(), de->possibleActions(), de->mimeData(), de->mouseButtons(), de->keyboardModifiers());
			/// FIXME
			//_mainWindow->dispatchDrop(d);
			return true;
		} else {
			if (obj == cornerWidget()) {
				auto p = this->addPlaylist();
				p->forceDrop(de);
			} else {
				currentPlayList()->forceDrop(de);
			}
			return true;
		}
	}
	return QTabWidget::eventFilter(obj, event);
}

void TabPlaylist::init(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	blockSignals(true);
	auto settings = SettingsPrivate::instance();
	if (settings->playbackRestorePlaylistsAtStartup()) {
		QList<uint> list = settings->lastPlaylistSession();
		if (!list.isEmpty()) {
			for (int i = 0; i < list.count(); i++) {
				this->loadPlaylist(list.at(i));
			}
			int lastActiveTab = settings->value("lastActiveTab").toInt();
			setCurrentIndex(lastActiveTab);
			if (playlist(lastActiveTab)) {
				_mediaPlayer->setPlaylist(playlist(lastActiveTab)->mediaPlaylist());
			}
		}
	}
	if (playlists().isEmpty()) {
		addPlaylist();
	}
	blockSignals(false);
	if (settings->contains("lastActivePlaylistMode")) {
		QMediaPlaylist::PlaybackMode mode = (QMediaPlaylist::PlaybackMode)settings->value("lastActivePlaylistMode").toInt();
		currentPlayList()->mediaPlaylist()->setPlaybackMode(mode);
		emit updatePlaybackModeButton(mode);
	}
}

/** Load a playlist saved in database. */
void TabPlaylist::loadPlaylist(uint playlistId)
{
	Playlist *playlist = nullptr;
	auto _db = SqlDatabase::instance();
	PlaylistDAO playlistDao = _db->selectPlaylist(playlistId);

	/// TODO: Do not load the playlist if it's already displayed

	int index = currentIndex();
	if (index >= 0) {
		playlist = this->playlist(index);
		if (!playlist->mediaPlaylist()->isEmpty()) {
			playlist = addPlaylist();
			this->tabBar()->setTabText(count() - 1, playlistDao.title());
		} else {
			this->tabBar()->setTabText(index, playlistDao.title());
		}
	} else {
		playlist = addPlaylist();
		this->tabBar()->setTabText(count() - 1, playlistDao.title());
	}
	playlist->setHash(playlistDao.checksum().toUInt());

	/// Reload tracks from filesystem of remote location, do not use outdated or incomplete data from cache!
	/// Use (host, id) or (uri)
	QList<TrackDAO> tracks = _db->selectPlaylistTracks(playlistId);
	playlist->insertMedias(-1, tracks);
	playlist->setId(playlistId);
	playlist->setTitle(playlistDao.title());

	this->setTabIcon(index, defaultIcon(QIcon::Disabled));
}

/** Get the playlist at index. */
Playlist* TabPlaylist::playlist(int index)
{
	return qobject_cast<Playlist*>(this->widget(index));
}

/** Retranslate context menu. */
void TabPlaylist::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		for (QAction *action : _contextMenu->actions()) {
			action->setText(QApplication::translate("TabPlaylist", action->text().toStdString().data()));
		}
	}
}

void TabPlaylist::contextMenuEvent(QContextMenuEvent *event)
{
	int tab = tabBar()->tabAt(event->pos());
	if (tab >= 0 && tab < count()) {
		_contextMenu->move(mapToGlobal(event->pos()));
		_contextMenu->setProperty("mouseRightClickPos", event->pos());
		Playlist *p = playlist(tab);
		_deletePlaylist->setEnabled(p && p->id() != 0);
		_contextMenu->show();
	}
}

/** Add a new playlist tab. */
Playlist* TabPlaylist::addPlaylist()
{
	QString newPlaylistName = tr("Playlist %1").arg(count() + 1);
	QByteArray ba;
	if (playlists().isEmpty()) {
		ba = SettingsPrivate::instance()->lastActivePlaylistGeometry();
	} else {
		ba = currentPlayList()->horizontalHeader()->saveState();
	}

	// Then append a new empty playlist to the others
	Playlist *p = new Playlist(_mediaPlayer, this);
	p->setTitle(newPlaylistName);
	p->installEventFilter(this);
	if (!ba.isEmpty()) {
		p->horizontalHeader()->restoreState(ba);
	}

	// Always create an icon in Disabled mode. It will be enabled when one will provide some tracks
	int i = addTab(p, newPlaylistName);
	this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));

	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaRemoved, this, [=](int start, int) {
		if (_mediaPlayer->playlist() == p->mediaPlaylist() && p->mediaPlaylist()->currentIndex() == start) {
			_mediaPlayer->stop();
		}
	});

	// Forward from inner class to MainWindow the signals
	connect(p, &Playlist::aboutToSendToTagEditor, this, &TabPlaylist::aboutToSendToTagEditor);
	connect(p, &Playlist::selectionHasChanged, this, &TabPlaylist::selectionChanged);

	// Check if tab icon should indicate that playlist has changed or not
	connect(p, &Playlist::contentHasChanged, this, [=]() {
		int playlistTabIndex = -1;
		for (int i = 0; i < playlists().count(); i++) {
			if (p == playlist(i)) {
				playlistTabIndex = i;
				break;
			}
		}
		if (playlistTabIndex != -1) {
			if (p->isModified()) {
				this->setTabIcon(playlistTabIndex, this->defaultIcon(QIcon::Normal));
			}
		}
	});

	// Select the new empty playlist
	setCurrentIndex(i);
	return p;
}

/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	bool isEmpty = this->currentPlayList()->mediaPlaylist()->isEmpty();

	QStringList tracks;
	for (QDir folder : folders) {
		QDirIterator it(folder.absolutePath(), FileHelper::suffixes(FileHelper::Standard, true), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isFile()) {
				tracks << it.fileInfo().absoluteFilePath();
			}
		}
	}
	tracks.sort(Qt::CaseInsensitive);
	QList<QUrl> urls;
	for (QString t : tracks) {
		urls << QUrl::fromLocalFile(t);
	}
	this->insertItemsToPlaylist(-1, urls);

	// Automatically plays the first track
	if (isEmpty) {
		_mediaPlayer->setPlaylist(this->currentPlayList()->mediaPlaylist());
		_mediaPlayer->play();
	}
}

/** Insert multiple tracks chosen by one from the library or the filesystem into a playlist. */
void TabPlaylist::insertItemsToPlaylist(int rowIndex, const QList<QUrl> &tracks)
{
	QList<QMediaContent> t;
	for (QUrl u : tracks) {
		t << QMediaContent(u);

	}
	currentPlayList()->insertMedias(rowIndex, t);
	if (currentPlayList()->isModified()) {
		this->setTabIcon(currentIndex(), this->defaultIcon(QIcon::Normal));
	}
	if (_mediaPlayer->playlist() == nullptr) {
		_mediaPlayer->setPlaylist(currentPlayList()->mediaPlaylist());
	}
	if (currentPlayList()->mediaPlaylist()->currentIndex() == -1) {
		currentPlayList()->mediaPlaylist()->setCurrentIndex(0);
	}
	if (currentPlayList()->mediaPlaylist()->playbackMode() == QMediaPlaylist::Random) {
		currentPlayList()->mediaPlaylist()->shuffle(-1);
	}
}

void TabPlaylist::savePlaylist(Playlist *p, bool overwrite)
{
	uint playlistId = _playlistManager->savePlaylist(p, overwrite, false);
	for (int i = 0; i < this->count(); i++) {
		Playlist *p2 = this->playlist(i);
		if (p2->id() == playlistId) {
			this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));
			break;
		}
	}
}

void TabPlaylist::renamePlaylist(Playlist *p)
{
	for (int i = 0; i < playlists().count(); i++) {
		Playlist *tmp = playlist(i);
		if (tmp == p) {
			this->setTabText(i, p->title());
			this->setTabIcon(i, this->defaultIcon(QIcon::Normal));
			break;
		}
	}
}

void TabPlaylist::renameTab(const PlaylistDAO &dao)
{
	for (int i = 0; i < playlists().count(); i++) {
		Playlist *tmp = playlist(i);
		if (tmp->id() == dao.id().toUInt()) {
			tmp->setTitle(dao.title());
			this->setTabText(i, dao.title());
			break;
		}
	}
}

void TabPlaylist::removeCurrentPlaylist()
{
	// Simulate a click on the close button
	emit tabCloseRequested(currentIndex());
}

void TabPlaylist::deletePlaylist(uint playlistId)
{
	int index = -1;
	for (int i = 0; i < playlists().count(); i++) {
		Playlist *tmp = playlist(i);
		if (tmp->id() == playlistId) {
			index = i;
			break;
		}
	}
	if (_playlistManager->deletePlaylist(playlistId)) {
		if (index != -1) {
			this->removeTabFromCloseButton(index);
		}
	}
}

void TabPlaylist::changeCurrentPlaylistPlaybackMode(QMediaPlaylist::PlaybackMode mode)
{
	qDebug() << Q_FUNC_INFO << mode;
	this->currentPlayList()->mediaPlaylist()->setPlaybackMode(mode);
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 1)) {
		Playlist *p = playlist(index);
		if (_mediaPlayer->playlist() == p->mediaPlaylist()) {
			_mediaPlayer->stop();
		}
		if (!p->mediaPlaylist()->isEmpty()) {
			p->mediaPlaylist()->removeMedia(0, p->mediaPlaylist()->mediaCount() - 1);
		}
		this->removeTab(index);
		delete p;
	} else {
		// Clear the content of first tab
		Playlist *p = playlist(index);
		if (_mediaPlayer->playlist() == p->mediaPlaylist()) {
			_mediaPlayer->stop();
		}
		p->mediaPlaylist()->clear();
		p->model()->removeRows(0, p->model()->rowCount());
		p->setHash(0);
		p->setId(0);
		tabBar()->setTabText(0, tr("Playlist %1").arg(1));
		p->setTitle(tabBar()->tabText(0));
		this->setTabIcon(index, this->defaultIcon(QIcon::Disabled));
	}
}

int TabPlaylist::closePlaylist(int index)
{
	Playlist *p = playlists().at(index);
	if (!(p && p->mediaPlaylist())) {
		return 0;
	}

	// If playlist is a loaded one, and hasn't changed then just close it. As well if empty too
	if (!p->isModified()) {
		this->removeTabFromCloseButton(index);
		this->setTabIcon(index, this->defaultIcon(QIcon::Disabled));
	} else {
		SettingsPrivate::PlaylistDefaultAction action = SettingsPrivate::instance()->playbackDefaultActionForClose();
		// Override default action and ask once again to user because it's not allowed to save empty playlist automatically
		if (p->mediaPlaylist()->isEmpty() && action == SettingsPrivate::PL_SaveOnClose) {
			action = SettingsPrivate::PL_AskUserForAction;
		}
		switch (action) {
		case SettingsPrivate::PL_AskUserForAction: {
			int returnCode = 0;
			ClosePlaylistPopup closePopup(p, index);
			connect(&closePopup, &ClosePlaylistPopup::aboutToSavePlaylist, [=](bool overwrite) {
				emit aboutToSavePlaylist(p, index, overwrite);
			});
			connect(&closePopup, &ClosePlaylistPopup::aboutToDeletePlaylist, this, &TabPlaylist::deletePlaylist);
			connect(&closePopup, &ClosePlaylistPopup::aboutToRemoveTab, this, &TabPlaylist::removeTabFromCloseButton);
			connect(&closePopup, &ClosePlaylistPopup::aboutToCancel, this, [&returnCode]() {
				// Interrupt exit!
				returnCode = 1;
			});
			closePopup.exec();
			return returnCode;
		}
		case SettingsPrivate::PL_SaveOnClose:
			emit aboutToSavePlaylist(p, false);
			break;
		case SettingsPrivate::PL_DiscardOnClose:
			this->removeTabFromCloseButton(index);
			break;
		}
	}
	return 0;
}
