#include "tabplaylist.h"

#include <QDirIterator>

#include "mainwindow.h"
#include "settings.h"
#include "tabbar.h"
#include "cornerwidget.h"

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent) :
	QTabWidget(parent), _closePlaylistPopup(new ClosePlaylistPopup(this)), _mainWindow(NULL)
{
	TabBar *tabBar = new TabBar(this);
	this->setTabBar(tabBar);
	this->setMovable(true);
	messageBox = new TracksNotFoundMessageBox(this);

	//SettingsPrivate *settings = SettingsPrivate::instance();

	// Add a new playlist
	connect(this, &QTabWidget::currentChanged, this, [=]() {
		emit updatePlaybackModeButton();
	});

	// Removing a playlist
	connect(_closePlaylistPopup->buttonBox, &QDialogButtonBox::clicked, this, &TabPlaylist::execActionFromClosePopup);
	connect(this, &QTabWidget::tabCloseRequested, this, &TabPlaylist::closePlaylist);

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

	// Context menu to add few actions for each playlist
	_contextMenu = new QMenu(this);
	QAction *renamePlaylist = new QAction(tr("Rename playlist"), _contextMenu);
	QAction *loadBackground = new QAction(tr("Load background..."), _contextMenu);
	QAction *clearBackground = new QAction(tr("Clear background"), _contextMenu);
	loadBackground->setEnabled(false);
	clearBackground->setEnabled(false);

	_contextMenu->addAction(renamePlaylist);
	_contextMenu->addSeparator();
	_contextMenu->addAction(loadBackground);
	_contextMenu->addAction(clearBackground);

	connect(renamePlaylist, &QAction::triggered, this, [=]() {
		QPoint p = _contextMenu->property("mouseRightClickPos").toPoint();
		int index = tabBar->tabAt(p);
		this->setCurrentIndex(index);
		tabBar->editTab(index);
	});
	connect(loadBackground, &QAction::triggered, this, [=]() {
		qDebug() << Q_FUNC_INFO << "Load background not implemented yet";
	});
	this->setAcceptDrops(true);

	CornerWidget *corner = new CornerWidget(this);
	this->setCornerWidget(corner, Qt::TopRightCorner);
	connect(corner, &CornerWidget::innerButtonClicked, this, &TabPlaylist::addPlaylist);
	corner->installEventFilter(this);
}

TabPlaylist::~TabPlaylist()
{}

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
		if (de->source() == NULL) {
			// Drag & Drop comes from another application but has landed in the playlist area
			de->ignore();
			QDropEvent *d = new QDropEvent(de->pos(), de->possibleActions(), de->mimeData(), de->mouseButtons(), de->keyboardModifiers());
			_mainWindow->dispatchDrop(d);
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

/** Get the playlist at index. */
Playlist* TabPlaylist::playlist(int index)
{
	return qobject_cast<Playlist*>(this->widget(index));
}

/** Setter. */
void TabPlaylist::setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
}

void TabPlaylist::setMainWindow(MainWindow *mainWindow)
{
	_mainWindow = mainWindow;
}

/** Retranslate tabs' name and all playlists in this widget. */
void TabPlaylist::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// No translation for the (+) tab button
		for (int i = 0; i < playlists().count(); i ++) {
			foreach (QLabel *label, widget(i)->findChildren<QLabel*>()) {
				if (label && !label->text().isEmpty()) {
					label->setText(QApplication::translate("TabPlaylist", label->text().toStdString().data()));
				}
			}
		}
		_closePlaylistPopup->retranslateUi(_closePlaylistPopup);
	}
}

void TabPlaylist::contextMenuEvent(QContextMenuEvent * event)
{
	int tab = tabBar()->tabAt(event->pos());
	if (tab >= 0 && tab < count()) {
		_contextMenu->move(mapToGlobal(event->pos()));
		_contextMenu->setProperty("mouseRightClickPos", event->pos());
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
	p->installEventFilter(this);
	if (!ba.isEmpty()) {
		p->horizontalHeader()->restoreState(ba);
	}

	// Always create an icon in Disabled mode. It will be enabled when one will provide some tracks
	int i = addTab(p, newPlaylistName);
	this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));

	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaRemoved, this, [=](int start, int) {
		if (_mediaPlayer.data()->playlist() == p->mediaPlaylist() && p->mediaPlaylist()->currentIndex() == start) {
			_mediaPlayer.data()->stop();
		}
	});

	// Forward from inner class to MainWindow the signals
	connect(p, &Playlist::aboutToSendToTagEditor, this, &TabPlaylist::aboutToSendToTagEditor);
	connect(p, &Playlist::selectionChanged, this, &TabPlaylist::selectionChanged);

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
			if (p->hash() != p->generateNewHash()) {
				this->setTabIcon(playlistTabIndex, this->defaultIcon(QIcon::Normal));
			}
		}
	});

	// Select the new empty playlist
	setCurrentIndex(i);
	uint hash = qHash(p);
	this->tabBar()->setTabData(i, hash);
	emit playlistCreated();
	return p;
}

/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	bool isEmpty = this->currentPlayList()->mediaPlaylist()->isEmpty();

	QStringList tracks;
	foreach (QDir folder, folders) {
		QDirIterator it(folder.absolutePath(), FileHelper::suffixes(FileHelper::Standard, true), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isFile()) {
				tracks << "file://" + it.fileInfo().absoluteFilePath();
			}
		}
	}
	this->insertItemsToPlaylist(-1, tracks);

	// Automatically plays the first track
	if (isEmpty) {
		this->mediaPlayer().data()->setPlaylist(this->currentPlayList()->mediaPlaylist());
		this->mediaPlayer().data()->play();
	}
}

/** Insert multiple tracks chosen by one from the library or the filesystem into a playlist. */
void TabPlaylist::insertItemsToPlaylist(int rowIndex, const QStringList &tracks)
{
	//uint oldHash = currentPlayList()->hash();
	currentPlayList()->insertMedias(rowIndex, tracks);
	//qDebug() << Q_FUNC_INFO << oldHash << currentPlayList()->generateNewHash();
	if (currentPlayList()->hash() != currentPlayList()->generateNewHash()) {
		this->setTabIcon(currentIndex(), this->defaultIcon(QIcon::Normal));
	}
	if (_mediaPlayer.data()->playlist() == NULL) {
		_mediaPlayer.data()->setPlaylist(currentPlayList()->mediaPlaylist());
	}
	if (currentPlayList()->mediaPlaylist()->currentIndex() == -1) {
		currentPlayList()->mediaPlaylist()->setCurrentIndex(0);
	}
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
	emit tabCloseRequested(currentIndex());
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
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 1)) {
		Playlist *p = playlist(index);
		if (_mediaPlayer.data()->playlist() == p->mediaPlaylist()) {
			_mediaPlayer.data()->stop();
		}
		if (!p->mediaPlaylist()->isEmpty()) {
			p->mediaPlaylist()->removeMedia(0, p->mediaPlaylist()->mediaCount() - 1);
		}
		this->removeTab(index);
		delete p;
	} else {
		// Clear the content of first tab
		Playlist *p = playlist(index);
		if (_mediaPlayer.data()->playlist() == p->mediaPlaylist()) {
			_mediaPlayer.data()->stop();
		}
		p->mediaPlaylist()->clear();
		p->model()->removeRows(0, p->model()->rowCount());
		p->setHash(0);
		tabBar()->setTabText(0, tr("Playlist %1").arg(1));
		uint hash = qHash(p);
		tabBar()->setTabData(0, hash);
		this->setTabIcon(index, this->defaultIcon(QIcon::Disabled));
	}
}

void TabPlaylist::updateRowHeight()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	for (int i = 0; i < count(); i++) {
		if (Playlist *p = playlist(i)) {
			p->verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(SettingsPrivate::FF_Playlist)).height());
		}
	}
}

void TabPlaylist::closePlaylist(int index)
{
	Playlist *p = playlists().at(index);
	if (!(p && p->mediaPlaylist())) {
		return;
	}

	// If playlist is a loaded one, and hasn't changed then just close it. As well if empty too
	uint newHash = p->generateNewHash();
	if (p->hash() == newHash || (p->mediaPlaylist()->isEmpty() && p->hash() == 0)) {
		this->removeTabFromCloseButton(index);
		p->setHash(0);
		this->setTabIcon(index, this->defaultIcon(QIcon::Disabled));
	} else {
		SettingsPrivate::PlaylistDefaultAction action = SettingsPrivate::instance()->playbackDefaultActionForClose();
		if (p->hash() != 0 && p->hash() != newHash) {
			qDebug() << Q_FUNC_INFO << "override default action and ask once again to user" << "old hash" << p->hash() << "new hash" << newHash;
			// Override default action and ask once again to user.
			action = SettingsPrivate::PL_AskUserForAction;
			if (p->mediaPlaylist()->isEmpty()) {
				_closePlaylistPopup->setDeleteMode(true);
			} else {
				_closePlaylistPopup->setOverwriteMode(true);
			}
		}
		switch (action) {
		case SettingsPrivate::PL_AskUserForAction:
			_closePlaylistPopup->setTabToClose(index);
			_closePlaylistPopup->setVisible(true);
			break;
		case SettingsPrivate::PL_SaveOnClose:
			emit aboutToSavePlaylist(index, false);
			break;
		case SettingsPrivate::PL_DiscardOnClose:
			this->removeTabFromCloseButton(index);
			break;
		}
	}
}

void TabPlaylist::execActionFromClosePopup(QAbstractButton *action)
{
	// Handle custom buttons (not QDialogButtonBox::StandardButton but QAbstractButton*)
	if (action == _closePlaylistPopup->replaceButton) {
		emit aboutToSavePlaylist(_closePlaylistPopup->index(), true);
	} else if (action == _closePlaylistPopup->deleteButton){
		int index = _closePlaylistPopup->index();
		emit aboutToDeletePlaylist(index, playlist(index));
	} else {
		// Standard enumeration
		switch(_closePlaylistPopup->buttonBox->standardButton(action)) {
		case QDialogButtonBox::Save:
			if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
				SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose);
			}
			emit aboutToSavePlaylist(_closePlaylistPopup->index(), false);
			break;
		case QDialogButtonBox::Discard:
			if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
				SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_DiscardOnClose);
			}
			this->removeTabFromCloseButton(_closePlaylistPopup->index());
			_closePlaylistPopup->setVisible(false);
			break;
		default:
			break;
		}
	}
}
