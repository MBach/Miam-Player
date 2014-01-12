#include "tabplaylist.h"

#include <QFileSystemModel>
#include <QStackedLayout>

#include "tabbar.h"
#include "settings.h"

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

/** Get the current playlist. */
Playlist* TabPlaylist::currentPlayList() const
{
	return this->widget(this->tabBar()->currentIndex())->findChild<Playlist*>();
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
	QString newPlaylistName = tr("Playlist %1").arg(count());

	// Then append a new empty playlist to the others
	QWidget *stackedWidget = new QWidget(this);
	stackedWidget->setObjectName("stackedWidget");
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

	QWidget *w = new QWidget(this);
	QVBoxLayout *vboxLayout = new QVBoxLayout(w);
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vboxLayout->addWidget(icon);
	vboxLayout->addWidget(label);
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	stackedLayout->addWidget(w);
	stackedLayout->addWidget(p);

	int i = insertTab(count(), stackedWidget, newPlaylistName);

	// Select the new empty playlist
	setCurrentIndex(i);
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
	QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(widget(currentIndex())->layout());
	stackedLayout->setCurrentIndex(1);
	stackedLayout->setStackingMode(QStackedLayout::StackOne);
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
		if (currentPlayList()->mediaPlaylist()->isEmpty()) {
			QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(widget(currentIndex())->layout());
			stackedLayout->setCurrentIndex(0);
			stackedLayout->setStackingMode(QStackedLayout::StackAll);
		}
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
		QStackedLayout *stackedLayout = qobject_cast<QStackedLayout*>(widget(0)->layout());
		stackedLayout->setCurrentIndex(0);
		stackedLayout->setStackingMode(QStackedLayout::StackAll);
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