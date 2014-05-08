#include "tabplaylist.h"

#include <QFileSystemModel>
#include <QStackedLayout>

#include "settings.h"
#include "tabbar.h"

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent) :
	QTabWidget(parent), _tabIndex(-1), _closePlaylistPopup(new ClosePlaylistPopup(this))
{
	this->setDocumentMode(true);
	this->setTabBar(new TabBar(this));
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
		Playlist *p = playlists().at(index);
		QString hash;
		for (int i = 0; i < p->mediaPlaylist()->mediaCount(); i++) {
			hash += p->mediaPlaylist()->media(i).canonicalUrl().toLocalFile();
		}
		// If playlist is a loaded one, and hasn't changed then just close it. As well if empty too
		if (p->hash() == qHash(hash) || playlists().at(index)->mediaPlaylist()->isEmpty()) {
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


/** Get the current playlist. */
Playlist* TabPlaylist::currentPlayList() const
{
	return this->widget(currentIndex())->findChild<Playlist*>();
}

QIcon TabPlaylist::defaultIcon(QIcon::Mode mode)
{
	static QIcon icon(":/icons/playlistIcon");
	QIcon grayIcon(icon.pixmap(QSize(tabBar()->fontMetrics().ascent(), tabBar()->fontMetrics().ascent()), QIcon::Disabled));
	if (mode == QIcon::Normal) {
		return icon;
	} else {
		return grayIcon;
	}
}

/** Redefined to forward events to children. */
bool TabPlaylist::eventFilter(QObject *obj, QEvent *event)
{
	//qDebug() << Q_FUNC_INFO << obj->objectName() << event->type();
	if (event->type() == QEvent::DragEnter) {
		event->accept();
		return true;
	} else if (event->type() == QEvent::Drop) {
		/// FIXME
		currentPlayList()->forceDrop(static_cast<QDropEvent*>(event));
		return true;
	}
	return QTabWidget::eventFilter(obj, event);
}

/** Get the playlist at index. */
Playlist* TabPlaylist::playlist(int index)
{
	return this->widget(index)->findChild<Playlist*>();
}

/** Setter. */
void TabPlaylist::setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
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

void TabPlaylist::dropEvent(QDropEvent *event)
{
	qDebug() << (event->source() == NULL);
	QTabWidget::dropEvent(event);
}

void TabPlaylist::displayEmptyArea(bool isEmpty)
{
	if (isEmpty) {
		QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(widget(currentIndex())->layout());
		stackedLayout->setCurrentIndex(0);
		stackedLayout->setStackingMode(QStackedLayout::StackAll);
		setTabIcon(currentIndex(), this->defaultIcon(QIcon::Disabled));
	} else {
		QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(widget(currentIndex())->layout());
		stackedLayout->setCurrentIndex(1);
		stackedLayout->setStackingMode(QStackedLayout::StackOne);
		setTabIcon(currentIndex(), this->defaultIcon());
	}
}

/** Add a new playlist tab. */
Playlist* TabPlaylist::addPlaylist()
{
	QString newPlaylistName = tr("Playlist %1").arg(count());

	// Then append a new empty playlist to the others
	QWidget *stackedWidget = new QWidget(this);

	stackedWidget->setAcceptDrops(true);
	stackedWidget->installEventFilter(this);
	Playlist *p = new Playlist(_mediaPlayer, this);
	p->setAcceptDrops(true);
	p->installEventFilter(this);

	QStackedLayout *stackedLayout = new QStackedLayout(stackedWidget);
	stackedLayout->setStackingMode(QStackedLayout::StackAll);

	QLabel *icon = new QLabel;
	icon->setAlignment(Qt::AlignCenter);
	icon->setPixmap(QPixmap(":/icons/emptyPlaylist"));

	QLabel *label = new QLabel(tr("This playlist is empty.\nSelect or drop tracks from your library or any external location."));
	label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);

	PlaylistFrame *w = new PlaylistFrame(this);
	QVBoxLayout *vboxLayout = new QVBoxLayout(w);
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vboxLayout->addWidget(icon);
	vboxLayout->addWidget(label);
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	stackedLayout->addWidget(w);
	stackedLayout->addWidget(p);

	// Always create an icon in Disabled mode. It will be enabled when one will provide some tracks
	int i = insertTab(count(), stackedWidget, newPlaylistName);
	this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));

	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaInserted, [=]() {
		this->displayEmptyArea(p->mediaPlaylist()->isEmpty());
	});
	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaRemoved, [=]() {
		this->displayEmptyArea(p->mediaPlaylist()->isEmpty());
	});
	// Forward from inner class to MainWindow the signal
	connect(p, &Playlist::aboutToSendToTagEditor, this, &TabPlaylist::aboutToSendToTagEditor);

	// Select the new empty playlist
	setCurrentIndex(i);
	uint hash = qHash(p);
	this->tabBar()->setTabData(i, hash);
	return p;
}

/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	qDebug() << Q_FUNC_INFO;
	bool isEmpty = this->currentPlayList()->mediaPlaylist()->isEmpty();
	foreach (QDir folder, folders) {
		QDirIterator it(folder, QDirIterator::Subdirectories);
		QStringList tracks;
		while (it.hasNext()) {
			tracks << it.next();
		}
		this->insertItemsToPlaylist(currentPlayList()->model()->rowCount(), tracks);
	}
	// Automatically plays the first track
	if (isEmpty) {
		this->mediaPlayer().data()->setPlaylist(this->currentPlayList()->mediaPlaylist());
		this->mediaPlayer().data()->play();
	}
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
	this->setTabIcon(currentIndex(), this->defaultIcon());
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
		displayEmptyArea(currentPlayList()->mediaPlaylist()->isEmpty());
	}
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 2)) {
		// If the playlist we want to delete has no more right tab, then pick the left tab
		if (index + 1 > count() - 2) {
			setCurrentIndex(index - 1);
		}
		Playlist *p = playlist(index);
		this->removeTab(index);
		delete p;
	} else {
		// Clear the content of first tab
		currentPlayList()->mediaPlaylist()->clear();
		currentPlayList()->model()->removeRows(0, currentPlayList()->model()->rowCount());
		tabBar()->setTabText(0, tr("Playlist %1").arg(1));
		this->displayEmptyArea();
	}
}

void TabPlaylist::updateRowHeight()
{
	Settings *settings = Settings::getInstance();
	for (int i = 0; i < count() - 1; i++) {
		Playlist *p = playlist(i);
		p->verticalHeader()->setDefaultSectionSize(QFontMetrics(settings->font(Settings::PLAYLIST)).height());
	}
}

/** When the user is clicking on the (+) button to add a new playlist. */
void TabPlaylist::checkAddPlaylistButton(int i)
{
	// The (+) button is the last tab
	if (i == count() - 1) {
		this->addPlaylist();
	} else {
		//currentPlayList()->countSelectedItems();
		emit updatePlaybackModeButton();
	}
}

void TabPlaylist::execActionFromClosePopup(QAbstractButton *action)
{
	switch(_closePlaylistPopup->buttonBox->standardButton(action)) {
	case QDialogButtonBox::Save:
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			Settings::getInstance()->setPlaybackDefaultActionForClose(Settings::SaveOnClose);
		}
		emit aboutToSavePlaylist(_closePlaylistPopup->index());
		break;
	case QDialogButtonBox::Discard:
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			Settings::getInstance()->setPlaybackDefaultActionForClose(Settings::DiscardOnClose);
		}
		this->removeTabFromCloseButton(_closePlaylistPopup->index());
		_closePlaylistPopup->hide();
		break;
	default:
		break;
	}
}
