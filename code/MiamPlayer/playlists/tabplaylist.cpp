#include "tabplaylist.h"

#include <QDirIterator>
#include <QStackedLayout>

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

	SettingsPrivate *settings = SettingsPrivate::instance();

	// Add a new playlist
	connect(this, &QTabWidget::currentChanged, this, [=]() {
		emit updatePlaybackModeButton();
	});

	// Removing a playlist
	connect(_closePlaylistPopup->buttonBox, &QDialogButtonBox::clicked, this, &TabPlaylist::execActionFromClosePopup);
	connect(this, &QTabWidget::tabCloseRequested, this, &TabPlaylist::closePlaylist);

	connect(settings, &SettingsPrivate::fontHasChanged, this, [=](const SettingsPrivate::FontFamily ff, const QFont &) {
		if (ff == SettingsPrivate::FF_Playlist) {
			for (int i = 0; i < count() - 1; i++) {
				if (playlist(i)->mediaPlaylist()->isEmpty()) {
					this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));
				} else {
					this->setTabIcon(i, this->defaultIcon(QIcon::Normal));
				}
			}
		}
	});

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
	connect(corner, &QPushButton::clicked, this, &TabPlaylist::addPlaylist);
	corner->installEventFilter(this);
}

TabPlaylist::~TabPlaylist()
{}

/** Get the current playlist. */
Playlist* TabPlaylist::currentPlayList() const
{
	return this->widget(currentIndex())->findChild<Playlist*>();
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
			this->displayEmptyArea(currentPlayList()->mediaPlaylist()->isEmpty());
			return true;
		}
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
		setTabIcon(currentIndex(), this->defaultIcon(QIcon::Normal));
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
	QWidget *stackedWidget = new QWidget(this);
	stackedWidget->setAcceptDrops(true);
	stackedWidget->installEventFilter(this);
	Playlist *p = new Playlist(_mediaPlayer, this);
	p->installEventFilter(this);
	if (!ba.isEmpty()) {
		p->horizontalHeader()->restoreState(ba);
	}

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
	w->setMinimumHeight(vboxLayout->sizeHint().height() + p->horizontalHeader()->height());

	stackedLayout->addWidget(w);
	stackedLayout->addWidget(p);

	// Always create an icon in Disabled mode. It will be enabled when one will provide some tracks
	int i = addTab(stackedWidget, newPlaylistName);
	this->setTabIcon(i, this->defaultIcon(QIcon::Disabled));

	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaInserted, this, [=]() {
		this->displayEmptyArea(p->mediaPlaylist()->isEmpty());
	});
	connect(p->mediaPlaylist(), &QMediaPlaylist::mediaRemoved, this, [=](int start, int) {
		bool empty = p->mediaPlaylist()->isEmpty();
		this->displayEmptyArea(empty);
		if (empty || p->mediaPlaylist()->currentIndex() == start) {
		   _mediaPlayer.data()->stop();
		}
	});

	// Forward from inner class to MainWindow the signals
	connect(p, &Playlist::aboutToSendToTagEditor, this, &TabPlaylist::aboutToSendToTagEditor);
	connect(p, &Playlist::selectionChanged, this, &TabPlaylist::selectionChanged);

	// Select the new empty playlist
	setCurrentIndex(i);
	uint hash = qHash(p);
	this->tabBar()->setTabData(i, hash);
	return p;
}

/** Add external folders (from a drag and drop) to the current playlist. */
void TabPlaylist::addExtFolders(const QList<QDir> &folders)
{
	bool isEmpty = this->currentPlayList()->mediaPlaylist()->isEmpty();

	QStringList tracks;
	foreach (QDir folder, folders) {
		QDirIterator it(folder.absolutePath(), FileHelper::suffixes(true), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
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
	currentPlayList()->insertMedias(rowIndex, tracks);
	this->setTabIcon(currentIndex(), this->defaultIcon(QIcon::Normal));
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
		displayEmptyArea(currentPlayList()->mediaPlaylist()->isEmpty());
	}
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 1)) {
		Playlist *p = playlist(index);
		if (!p->mediaPlaylist()->isEmpty()) {
			p->mediaPlaylist()->removeMedia(0, p->mediaPlaylist()->mediaCount() - 1);
		}
		this->removeTab(index);
		delete p;
	} else {
		// Clear the content of first tab
		currentPlayList()->mediaPlaylist()->clear();
		currentPlayList()->model()->removeRows(0, currentPlayList()->model()->rowCount());
		tabBar()->setTabText(0, tr("Playlist %1").arg(1));
		uint hash = qHash(currentPlayList());
		tabBar()->setTabData(0, hash);
		this->displayEmptyArea();
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
	QString hash;
	for (int i = 0; i < p->mediaPlaylist()->mediaCount(); i++) {
		hash += p->mediaPlaylist()->media(i).canonicalUrl().toString();
	}
	// If playlist is a loaded one, and hasn't changed then just close it. As well if empty too
	if (p->hash() == qHash(hash) || playlists().at(index)->mediaPlaylist()->isEmpty()) {
		this->removeTabFromCloseButton(index);
	} else {
		SettingsPrivate::PlaylistDefaultAction action = SettingsPrivate::instance()->playbackDefaultActionForClose();
		switch (action) {
		case SettingsPrivate::PL_AskUserForAction:
			_closePlaylistPopup->setTabToClose(index);
			_closePlaylistPopup->show();
			break;
		case SettingsPrivate::PL_SaveOnClose:
			emit aboutToSavePlaylist(index);
			break;
		case SettingsPrivate::PL_DiscardOnClose:
			this->removeTabFromCloseButton(index);
			break;
		}
	}
}

void TabPlaylist::execActionFromClosePopup(QAbstractButton *action)
{
	switch(_closePlaylistPopup->buttonBox->standardButton(action)) {
	case QDialogButtonBox::Save:
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose);
		}
		emit aboutToSavePlaylist(_closePlaylistPopup->index());
		break;
	case QDialogButtonBox::Discard:
		if (_closePlaylistPopup->checkBoxRememberChoice->isChecked()) {
			SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_DiscardOnClose);
		}
		this->removeTabFromCloseButton(_closePlaylistPopup->index());
		_closePlaylistPopup->hide();
		break;
	default:
		break;
	}
}
