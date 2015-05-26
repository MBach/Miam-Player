#include "playlistmanager.h"

#include <model/playlistdao.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include "starrating.h"
#include "styling/miamstyleditemdelegate.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(SqlDatabase *db, TabPlaylist *tabPlaylist) :
	QDialog(tabPlaylist, Qt::Tool),_db(db), _tabPlaylists(tabPlaylist),
	_unsavedPlaylistModel(new QStandardItemModel(this)), _savedPlaylistModel(new QStandardItemModel(this))
{
	setupUi(this);
	unsavedPlaylists->setAttribute(Qt::WA_MacShowFocusRect, false);
	savedPlaylists->setAttribute(Qt::WA_MacShowFocusRect, false);
	previewPlaylist->setAttribute(Qt::WA_MacShowFocusRect, false);

	delete groupBoxPreview->layout();
	_stackLayout = new QStackedLayout(groupBoxPreview);
	groupBoxPreview->setContentsMargins(11, 24, 11, 11);
	QLabel *icon = new QLabel();
	icon->setAlignment(Qt::AlignCenter);
	icon->setPixmap(QPixmap(":/icons/emptyPlaylist"));

	_labelEmptyPreview = new QLabel(tr("This preview area is empty.\nSelect a playlist to display the first 30 tracks."));
	_labelEmptyPreview->setAlignment(Qt::AlignCenter);
	_labelEmptyPreview->setWordWrap(true);

	QVBoxLayout *vboxLayout = new QVBoxLayout();
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vboxLayout->addWidget(icon);
	vboxLayout->addWidget(_labelEmptyPreview);
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QWidget *widget = new QWidget();
	widget->setLayout(vboxLayout);

	_stackLayout->addWidget(widget);
	_stackLayout->addWidget(previewPlaylist);

	loadPlaylists->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
	deletePlaylists->setIcon(this->style()->standardIcon(QStyle::SP_DialogCloseButton));
	exportPlaylists->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));

	unsavedPlaylists->installEventFilter(this);
	savedPlaylists->installEventFilter(this);
	savedPlaylists->setDragDropMode(QListView::DropOnly);
	this->installEventFilter(this);

	unsavedPlaylists->setModel(_unsavedPlaylistModel);
	savedPlaylists->setModel(_savedPlaylistModel);
	/// XXX: MiamStyledItemDelegate should be improved!
	// savedPlaylists->setItemDelegate(new MiamStyledItemDelegate(savedPlaylists, false));

	connect(unsavedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromUnsaved);
	connect(savedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromSaved);
	connect(loadPlaylists, &QPushButton::clicked, this, &PlaylistManager::loadSelectedPlaylists);
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistManager::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsAboutToBeRemoved, this, &PlaylistManager::dropAutoSavePlaylists);
	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, [=](const QModelIndex &, int, int) {
		this->updatePlaylists(false, true);
	});

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistManager::exportSelectedPlaylist);

	// Save playlist on close (if enabled)
	connect(_tabPlaylists, &TabPlaylist::aboutToSavePlaylist, this, &PlaylistManager::saveAndRemovePlaylist);
	connect(_tabPlaylists, &TabPlaylist::aboutToDeletePlaylist, this, &PlaylistManager::deletePlaylist);
	connect(this, &PlaylistManager::aboutToRemovePlaylist, _tabPlaylists, &TabPlaylist::removeTabFromCloseButton);
}

/** Add drag & drop processing. */
bool PlaylistManager::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Drop || event->type() == QEvent::DragEnter) {
		return QDialog::eventFilter(obj, event);
	} else if (obj == _tabPlaylists) {
		return QDialog::eventFilter(obj, event);
	} else if (event->type() == QEvent::Drop) {
		return QDialog::eventFilter(obj, event);
	} else if (obj == savedPlaylists && event->type() == QEvent::KeyPress) {
		// Bind common keys to useful actions
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Delete) {
			this->deleteSavedPlaylists();
		} else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
			this->loadSelectedPlaylists();
		}
		return true;
	} else {

		if (obj == this && event->type() == QEvent::Close) {
			SettingsPrivate::instance()->setValue("PlaylistManagerGeometry", this->saveGeometry());
		}

		// standard event processing
		return QDialog::eventFilter(obj, event);
	}
}

void PlaylistManager::init()
{
	_tabPlaylists->blockSignals(true);
	if (SettingsPrivate::instance()->playbackRestorePlaylistsAtStartup()) {
		for (PlaylistDAO playlist : _db->selectPlaylists()) {
			this->loadPlaylist(playlist.id().toUInt());
		}
	}
	if (_tabPlaylists->playlists().isEmpty()) {
		_tabPlaylists->addPlaylist();
	}
	_tabPlaylists->blockSignals(false);
}

int PlaylistManager::savePlaylist(int index, bool isOverwriting)
{
	Playlist *p = _tabPlaylists->playlist(index);
	int id = -1;
	if (p && !p->mediaPlaylist()->isEmpty()) {
		QString playlistName = _tabPlaylists->tabBar()->tabText(index);

		uint generateNewHash = p->generateNewHash();

		// Check first if one has the same playlist in database
		PlaylistDAO playlist;
		for (PlaylistDAO dao : _db->selectPlaylists()) {
			if (dao.checksum().toUInt() == generateNewHash) {
				// Playlist exists in database and user is not exiting application -> showing a popup
				QMessageBox mb;
				mb.setIcon(QMessageBox::Information);
				mb.setText(tr("There is exactly the same playlist in the Playlist Manager (known as '%1'), "\
							  "therefore it's not possible to add it twice.").arg(dao.title()));
				mb.addButton(QMessageBox::Cancel);
				mb.addButton(QMessageBox::Discard);
				mb.setDefaultButton(QMessageBox::Cancel);

				int ret = mb.exec();
				switch (ret) {
				case QMessageBox::Cancel:
					return 0;
				case QMessageBox::Discard:
					return 1;
				}
				break;
			} else if (isOverwriting && p->hash() == dao.checksum().toUInt()) {
				playlist = dao;
				break;
			}
		}

		playlist.setTitle(playlistName);
		playlist.setChecksum(QString::number(generateNewHash));

		std::list<TrackDAO> tracks;
		const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(p->model());
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			// Eeach track has been saved in a hidden column into the playlist
			/// FIXME
			TrackDAO t = model->index(j, p->COL_TRACK_DAO).data().value<TrackDAO>();
			tracks.push_back(std::move(t));
		}

		id = _db->insertIntoTablePlaylists(playlist, tracks, isOverwriting);
		p->setHash(generateNewHash);
	}
	return id;
}

void PlaylistManager::retranslateUi(PlaylistManager *dialog)
{
	_labelEmptyPreview->setText(QApplication::translate("PlaylistManager", "This preview area is empty.\nSelect a playlist to display the first 30 tracks.", 0));
	Ui::PlaylistManager::retranslateUi(dialog);
}

void PlaylistManager::saveAndRemovePlaylist(int index, bool isOverwriting)
{
	if (this->savePlaylist(index, isOverwriting)) {
		emit aboutToRemovePlaylist(index);
	}
}

void PlaylistManager::clearPreview(bool aboutToInsertItems)
{
	previewPlaylist->clear();
	if (aboutToInsertItems) {
		_stackLayout->setCurrentIndex(1);
		_stackLayout->setStackingMode(QStackedLayout::StackOne);
	} else {
		_stackLayout->setCurrentIndex(0);
		_stackLayout->setStackingMode(QStackedLayout::StackAll);
	}
}

/** Remove all special characters for Windows, Unix, OSX. */
QString PlaylistManager::convertNameToValidFileName(QString &name)
{
	/// XXX: should be improved?
	static const QRegularExpression re("[/\\:*?\"<>|]");
	name.replace(re, "_");
	return name;
}

void PlaylistManager::loadPlaylist(uint playlistId)
{
	Playlist *playlist = NULL;
	PlaylistDAO playlistDao = _db->selectPlaylist(playlistId);

	qDebug() << Q_FUNC_INFO << playlistId;

	// Do not load the playlist if it's already displayed
	for (int i = 0; i < _tabPlaylists->playlists().count(); i++) {
		Playlist *p = _tabPlaylists->playlist(i);
		bool ok = false;
		uint checksum = playlistDao.checksum().toUInt(&ok);
		if (ok && checksum == p->hash()) {
			/// TODO: ask one if if want to reload the playlist or not
			_tabPlaylists->setCurrentIndex(i);
			return;
		}
	}

	int index = _tabPlaylists->currentIndex();
	if (index >= 0) {
		playlist = _tabPlaylists->playlist(index);
		qDebug() << playlist->hash() << playlist->generateNewHash() << playlist->mediaPlaylist()->isEmpty();
		if (!playlist->mediaPlaylist()->isEmpty()) {
			playlist = _tabPlaylists->addPlaylist();
			_tabPlaylists->tabBar()->setTabText(_tabPlaylists->count() - 1, playlistDao.title());
		} else {
			_tabPlaylists->tabBar()->setTabText(index, playlistDao.title());
		}
	} else {
		playlist = _tabPlaylists->addPlaylist();
		_tabPlaylists->tabBar()->setTabText(_tabPlaylists->count() - 1, playlistDao.title());
	}
	playlist->setHash(playlistDao.checksum().toUInt());
	qDebug() << Q_FUNC_INFO << playlistDao.title() << playlist->hash();

	/// Reload tracks from filesystem of remote location, do not use outdated or incomplete data from cache!
	/// Use (host, id) or (uri)
	QList<TrackDAO> tracks = _db->selectPlaylistTracks(playlistId);
	playlist->insertMedias(-1, tracks);
	playlist->setId(playlistId);

	_tabPlaylists->setTabIcon(index, _tabPlaylists->defaultIcon(QIcon::Disabled));
}

/** Redefined: clean preview area, populate once again lists. */
void PlaylistManager::open()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->value("PlaylistManagerGeometry").isValid()) {
		this->restoreGeometry(settings->value("PlaylistManagerGeometry").toByteArray());
	}
	this->clearPreview(false);
	this->updatePlaylists();
	QDialog::open();
	this->activateWindow();
}

void PlaylistManager::deletePlaylist(int index, Playlist *p)
{
	QList<PlaylistDAO> playlists;
	PlaylistDAO dao;
	dao.setId(QString::number(p->id()));
	playlists << dao;

	// Try to remove the playlist then call the view to reset or close the current tab
	if (_db->removePlaylists(playlists)) {
		emit aboutToRemovePlaylist(index);
	}
}

/** Delete from the file system every selected playlists. Cannot be canceled. */
void PlaylistManager::deleteSavedPlaylists()
{
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	if (indexes.isEmpty()) {
		return;
	}

	QString deleteMessage = QApplication::translate("PlaylistManager", "You're about to delete %n playlist. Are you sure you want to continue?", "", indexes.size());
	if (QMessageBox::Ok != QMessageBox::warning(this, tr("Warning"), deleteMessage, QMessageBox::Ok, QMessageBox::Cancel)) {
		return;
	}

	QList<PlaylistDAO> playlists;
	for (QModelIndex index : indexes) {
		PlaylistDAO tmpPlaylist;
		tmpPlaylist.setId(index.data(PlaylistID).toString());
		playlists << tmpPlaylist;
	}
	_db->removePlaylists(playlists);

	this->clearPreview(false);
	this->updatePlaylists();
}

void PlaylistManager::dropAutoSavePlaylists(const QModelIndex &, int start, int end)
{
	for (int i = start; i <= end; i++) {
		auto item = _unsavedPlaylistModel->item(start);
		if (item) {
			uint playlistObjectPointer = item->data(PlaylistObjectPointer).toUInt();
			/// XXX: it's not really easy to read...
			// Find the playlist in the TabWidget
			for (int i = 0; i < _tabPlaylists->playlists().count(); i++) {
				if (playlistObjectPointer == _tabPlaylists->tabBar()->tabData(i).toUInt()) {
					if (this->savePlaylist(i) > 0) {
						_tabPlaylists->setTabIcon(i, _tabPlaylists->defaultIcon(QIcon::Disabled));
					}
				}
			}
		}
	}
}

/** Export one playlist at a time. */
void PlaylistManager::exportSelectedPlaylist()
{
	QString exportedPlaylistLocation;
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->value("locationForExportedPlaylist").isNull()) {
		exportedPlaylistLocation = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
	} else {
		exportedPlaylistLocation = settings->value("locationForExportedPlaylist").toString();
	}
	auto indexes = savedPlaylists->selectionModel()->selectedIndexes();
	if (indexes.isEmpty()) {
		return;
	}
	QModelIndex index = indexes.first();
	QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
	/// FIXME
	//QString pPath = item->data(PlaylistPath).toString();
	QString pName = item->data(Qt::DisplayRole).toString();

	// Open a file dialog and ask the user to choose a location
	QString newName = QFileDialog::getSaveFileName(this, tr("Export playlist"), exportedPlaylistLocation + QDir::separator() + pName, tr("Playlist (*.m3u8)"));
	if (QFile::exists(newName)) {
		QFile removePreviousOne(newName);
		if (!removePreviousOne.remove()) {
			qDebug() << Q_FUNC_INFO << "Cannot remove" << newName;
		}
	}
	if (newName.isEmpty()) {
		return;
	}
}

/** Load every saved playlists. */
void PlaylistManager::loadSelectedPlaylists()
{
	qDebug() << Q_FUNC_INFO;
	for (QModelIndex index : savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
		if (item) {
			this->loadPlaylist(item->data(PlaylistID).toUInt());
		}
	}
	this->close();
}

void PlaylistManager::populatePreviewFromSaved(QItemSelection, QItemSelection)
{
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	bool empty = indexes.isEmpty();
	this->clearPreview(!empty);
	if (indexes.size() == 1) {
		uint playlistId = _savedPlaylistModel->itemFromIndex(indexes.first())->data(PlaylistID).toUInt();
		qDebug() << Q_FUNC_INFO << "playlistId" << playlistId;

		QList<TrackDAO> tracks = _db->selectPlaylistTracks(playlistId);
		for (int i = 0; i < tracks.size(); i++) {
			TrackDAO track = tracks.at(i);
			QTreeWidgetItem *item = new QTreeWidgetItem;
			item->setText(0, QString("%1 (%2 - %3)").arg(track.title(), track.artist(), track.album()));
			previewPlaylist->addTopLevelItem(item);

			if (i + 1 == MAX_TRACKS_PREVIEW_AREA) {
				QTreeWidgetItem *item = new QTreeWidgetItem;
				item->setText(0, tr("And more tracks..."));
				previewPlaylist->addTopLevelItem(item);
				break;
			}
		}
	}
	loadPlaylists->setDisabled(empty);
	deletePlaylists->setDisabled(empty);
	exportPlaylists->setEnabled(indexes.size() == 1);
}

void PlaylistManager::populatePreviewFromUnsaved(QItemSelection, QItemSelection)
{
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	bool b = unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1;
	this->clearPreview(b);
	if (b) {

		QStandardItem *item = _unsavedPlaylistModel->itemFromIndex(unsavedPlaylists->selectionModel()->selectedIndexes().first());
		/// FIXME
		uint hash = item->data(PlaylistObjectPointer).toUInt();
		for (int i = 0; i < _tabPlaylists->playlists().count(); i++) {
			uint playlistPointer = _tabPlaylists->tabBar()->tabData(i).toUInt();

			if (playlistPointer == hash) {
				Playlist *p = _tabPlaylists->playlist(i);
				int max = qMin(p->mediaPlaylist()->mediaCount(), MAX_TRACKS_PREVIEW_AREA);
				for (int idxTrack = 0; idxTrack < max; idxTrack++) {
					QString title = p->model()->index(idxTrack, p->COL_TITLE).data().toString();
					QString artist = p->model()->index(idxTrack, p->COL_ARTIST).data().toString();
					QString album = p->model()->index(idxTrack, p->COL_ALBUM).data().toString();
					QTreeWidgetItem *item = new QTreeWidgetItem;
					item->setText(0, QString("%1 (%2 - %3)").arg(title, artist, album));
					previewPlaylist->addTopLevelItem(item);
				}
				if (p->mediaPlaylist()->mediaCount() > MAX_TRACKS_PREVIEW_AREA) {
					QTreeWidgetItem *item = new QTreeWidgetItem;
					item->setText(0, tr("And more tracks..."));
					previewPlaylist->addTopLevelItem(item);
				}
				break;
			}
		}
	}
}

/** Update saved and unsaved playlists when one is adding a new one. */
void PlaylistManager::updatePlaylists(bool unsaved, bool saved)
{
	qDebug() << Q_FUNC_INFO;

	if (unsaved) {
		// Populate unsaved playlists area
		_unsavedPlaylistModel->clear();
		for (int i = 0; i < _tabPlaylists->playlists().count(); i++) {
			Playlist *p = _tabPlaylists->playlist(i);
			uint hash = p->hash();
			qDebug() << _tabPlaylists->tabBar()->tabText(i) << hash << p->generateNewHash();
			if (!p->mediaPlaylist()->isEmpty() && hash != p->generateNewHash()) {
				QStandardItem *item = new QStandardItem(_tabPlaylists->tabBar()->tabText(i));
				uint playlistObjectPointer = _tabPlaylists->tabBar()->tabData(i).toUInt();
				item->setData(playlistObjectPointer, PlaylistObjectPointer);
				_unsavedPlaylistModel->appendRow(item);
			}
		}
	}

	if (saved) {
		// Populate saved playlists area
		_savedPlaylistModel->clear();
		_savedPlaylistModel->blockSignals(true);

		for (PlaylistDAO playlist : _db->selectPlaylists()) {
			QStandardItem *item = new QStandardItem(playlist.title());
			item->setData(playlist.id(), PlaylistID);
			if (playlist.icon().isEmpty()) {
				item->setIcon(QIcon(":/icons/playlist"));
			} else {
				item->setIcon(QIcon(playlist.icon()));
			}
			_savedPlaylistModel->appendRow(item);
		}
		_savedPlaylistModel->blockSignals(false);
	}
}
