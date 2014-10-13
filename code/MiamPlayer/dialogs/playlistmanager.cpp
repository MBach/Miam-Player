#include "playlistmanager.h"

#include <filehelper.h>
#include <settingsprivate.h>
#include "starrating.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(SqlDatabase *db, TabPlaylist *tabPlaylist) :
	QDialog(tabPlaylist, Qt::Tool),_db(db), playlists(tabPlaylist),
	_unsavedPlaylistModel(new QStandardItemModel(this)), _savedPlaylistModel(new QStandardItemModel(this))
{
	setupUi(this);
	delete groupBoxPreview->layout();
	_stackLayout = new QStackedLayout(groupBoxPreview);
	groupBoxPreview->setContentsMargins(11, 24, 11, 11);
	QLabel *icon = new QLabel();
	icon->setAlignment(Qt::AlignCenter);
	icon->setPixmap(QPixmap(":/icons/emptyPlaylist"));

	QLabel *label = new QLabel(tr("This preview area is empty.\nSelect a playlist to display the first 30 tracks."));
	label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);

	QVBoxLayout *vboxLayout = new QVBoxLayout();
	vboxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vboxLayout->addWidget(icon);
	vboxLayout->addWidget(label);
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

	connect(unsavedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromUnsaved);
	connect(savedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromSaved);
	/*connect(_savedPlaylistModel, &QStandardItemModel::itemChanged, [=](QStandardItem *item) {
		_db->open();
		QSqlQuery update(_db);
		update.prepare("UPDATE playlists SET name = :name WHERE absPath = :path");
		update.bindValue(":name", item->text());
		update.bindValue(":path", item->data(PlaylistPath).toString());
		qDebug() << "update?" << update.exec();
		_db->close();
	});*/
	connect(loadPlaylists, &QPushButton::clicked, this, &PlaylistManager::loadSelectedPlaylists);
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistManager::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsAboutToBeRemoved, this, &PlaylistManager::dropAutoSavePlaylists);
	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, [=]() {
		this->updatePlaylists();
	});
	connect(qApp, &QApplication::aboutToQuit, this, &PlaylistManager::savePlaylists);

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistManager::exportSelectedPlaylist);
	// Save playlist on close (if enabled)
	connect(playlists, &TabPlaylist::aboutToSavePlaylist, this, &PlaylistManager::saveAndRemovePlaylist);
	connect(this, &PlaylistManager::playlistSaved, playlists, &TabPlaylist::removeTabFromCloseButton);
}

bool PlaylistManager::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Drop || event->type() == QEvent::DragEnter) {
		return QDialog::eventFilter(obj, event);
	} else if (obj == playlists) {
		return QDialog::eventFilter(obj, event);
	} else if (event->type() == QEvent::Drop) {
		return QDialog::eventFilter(obj, event);
	} else {

		if (obj == this && event->type() == QEvent::Close) {
			SettingsPrivate::getInstance()->setValue("PlaylistManagerGeometry", this->saveGeometry());
		}

		// standard event processing
		return QDialog::eventFilter(obj, event);
	}
}

void PlaylistManager::init()
{
	playlists->blockSignals(true);
	if (SettingsPrivate::getInstance()->playbackRestorePlaylistsAtStartup()) {
		foreach (PlaylistDAO playlist, _db->selectPlaylists()) {
			this->loadPlaylist(playlist.id().toInt());
		}
	}
	if (playlists->playlists().isEmpty()) {
		playlists->addPlaylist();
	}
	playlists->blockSignals(false);
}

void PlaylistManager::saveAndRemovePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO << index;
	if (this->savePlaylist(index)) {
		emit playlistSaved(index);
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

void PlaylistManager::loadPlaylist(int playlistId)
{
	Playlist *playlist = NULL;
	PlaylistDAO remotePlaylist = _db->selectPlaylist(playlistId);
	if (playlists->playlist(0) && playlists->playlist(0)->mediaPlaylist()->isEmpty()) {
		playlist = playlists->playlist(0);
		playlists->tabBar()->setTabText(0, remotePlaylist.title());
	} else {
		playlist = playlists->addPlaylist();
		playlists->tabBar()->setTabText(playlists->count() - 2, remotePlaylist.title());
	}
	playlist->setProperty("dao", QVariant::fromValue(remotePlaylist));

	/// Reload tracks from filesystem of remote location, do not use outdated or incomplete data from cache!
	/// Use (host, id) or (absPath)
	QList<TrackDAO> tracks = _db->selectPlaylistTracks(playlistId);
	foreach (TrackDAO track, tracks) {
		QUrl url = QUrl(track.url());
		if (url.isLocalFile()) {
			QStringList l = QStringList() << url.toLocalFile();
			playlist->insertMedias(-1, l);
		} else {
			QList<TrackDAO> tracks2;
			tracks2 << track;
			playlist->insertMedias(-1, tracks2);
		}
	}
}

bool PlaylistManager::savePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO;
	Playlist *p = playlists->playlist(index);
	if (p && !p->mediaPlaylist()->isEmpty()) {
		QString playlistName = playlists->tabBar()->tabText(index);

		QString files;
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			files.append(p->mediaPlaylist()->media(j).canonicalUrl().toString());
		}
		uint hash = qHash(files);
		qDebug() << "savePlaylist() << new hash generated" << hash;

		PlaylistDAO playlist;
		playlist.setTitle(playlistName);
		playlist.setChecksum(QString::number(hash));
		int id = _db->insertIntoTablePlaylists(playlist);
		if (id > 0) {
			std::list<TrackDAO> tracks;

			const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(p->model());
			for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
				// Eeach track has been saved in a hidden column into the playlist
				TrackDAO t = model->index(j, p->COL_TRACK_DAO).data().value<TrackDAO>();
				tracks.push_back(std::move(t));
			}

			return _db->insertIntoTablePlaylistTracks(id, tracks);
		}
	}
	return false;
}

/** Redefined: clean preview area, populate once again lists. */
void PlaylistManager::open()
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	if (settings->value("PlaylistManagerGeometry").isValid()) {
		this->restoreGeometry(settings->value("PlaylistManagerGeometry").toByteArray());
	}
	this->clearPreview(false);
	this->updatePlaylists();
	QDialog::open();
	this->activateWindow();
}

/** Delete from the file system every selected playlists. Cannot be canceled. */
void PlaylistManager::deleteSavedPlaylists()
{
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	QString deleteMessage = QApplication::translate("PlaylistManager", "You're about to delete %n playlist. Are you sure you want to continue?", "", indexes.size());
	if (QMessageBox::Ok != QMessageBox::warning(this, tr("Warning"), deleteMessage, QMessageBox::Ok, QMessageBox::Cancel)) {
		return;
	}

	QList<PlaylistDAO> playlists;
	foreach (QModelIndex index, indexes) {
		PlaylistDAO tmpPlaylist;
		tmpPlaylist.setId(index.data(PlaylistID).toString());
		playlists << tmpPlaylist;
	}
	_db->removePlaylists(playlists);

	this->clearPreview(false);
	this->updatePlaylists();
}

void PlaylistManager::dropAutoSavePlaylists(const QModelIndex &parent, int start, int)
{
	QModelIndex index = _unsavedPlaylistModel->index(start, 0, parent);
	uint playlistObjectPointer = index.data(PlaylistObjectPointer).toUInt();
	int idx = -1;
	for (int i = 0; i < playlists->playlists().count(); i++) {
		Playlist *p = playlists->playlist(i);
		uint hash = qHash(p);
		if (hash == playlistObjectPointer) {
			idx = i;
			break;
		}
	}
	if (idx >= 0) {
		QString playlistPath = this->savePlaylist(idx);
		playlists->tabBar()->setTabData(idx, playlistPath);
		playlists->setTabIcon(idx, playlists->defaultIcon(QIcon::Disabled));
	}
}

/** Export one playlist at a time. */
void PlaylistManager::exportSelectedPlaylist()
{
	QString exportedPlaylistLocation;
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	if (settings->value("locationForExportedPlaylist").isNull()) {
		exportedPlaylistLocation = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
	} else {
		exportedPlaylistLocation = settings->value("locationForExportedPlaylist").toString();
	}
	QModelIndex index = savedPlaylists->selectionModel()->selectedIndexes().first();
	QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
	/// FIXME
	//QString pPath = item->data(PlaylistPath).toString();
	QString pName = item->data(Qt::DisplayRole).toString();

	// Open a file dialog and ask the user to choose a location
	QString newName = QFileDialog::getSaveFileName(this, tr("Export playlist"), exportedPlaylistLocation + QDir::separator() + pName, tr("Playlist (*.m3u8)"));
	if (QFile::exists(newName)) {
		QFile removePreviousOne(newName);
		if (!removePreviousOne.remove()) {
			qDebug() << "Cannot remove" << newName;
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
	foreach (QModelIndex index, savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
		if (item) {
			this->loadPlaylist(item->data(PlaylistID).toInt());
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
		int playlistId = _savedPlaylistModel->itemFromIndex(indexes.first())->data(PlaylistID).toInt();
		qDebug() << "playlistId" << playlistId;

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
	this->clearPreview();
	if (unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1) {

		QStandardItem *item = _unsavedPlaylistModel->itemFromIndex(unsavedPlaylists->selectionModel()->selectedIndexes().first());
		/// FIXME
		uint playlistObjectPointer = item->data(PlaylistObjectPointer).toUInt();
		for (int i = 0; i < playlists->playlists().count(); i++) {
			Playlist *p = playlists->playlist(i);
			uint hash = qHash(p);
			if (hash == playlistObjectPointer) {
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

/** Save all playlists when exiting the application (if enabled). */
void PlaylistManager::savePlaylists()
{
	if (SettingsPrivate::getInstance()->playbackKeepPlaylists()) {
		_db->open();
		for (int i = 0; i < playlists->count(); i++) {
			this->savePlaylist(i);
		}
		_db->close();
	}
}

/** Update saved and unsaved playlists when one is adding a new one. */
void PlaylistManager::updatePlaylists()
{
	// Populate unsaved playlists area
	_unsavedPlaylistModel->clear();
	for (int i = 0; i < playlists->playlists().count(); i++) {
		if (!playlists->playlist(i)->mediaPlaylist()->isEmpty()) {
			uint playlistObjectPointer = playlists->tabBar()->tabData(i).toUInt();
			if (playlistObjectPointer != 0) {
				QStandardItem *item = new QStandardItem(playlists->tabBar()->tabText(i));
				item->setData(playlistObjectPointer, PlaylistObjectPointer);
				_unsavedPlaylistModel->appendRow(item);
			}
		}
	}

	// Populate saved playlists area
	_savedPlaylistModel->clear();
	_savedPlaylistModel->blockSignals(true);

	QList<PlaylistDAO> playlists = _db->selectPlaylists();
	foreach (PlaylistDAO playlist, playlists) {
		QStandardItem *item = new QStandardItem(playlist.title());
		item->setData(playlist.id(), PlaylistID);
		if (playlist.iconPath().isEmpty()) {
			item->setIcon(QIcon(":/icons/playlist"));
		} else {
			item->setIcon(QIcon(playlist.iconPath()));
		}
		_savedPlaylistModel->appendRow(item);
	}
	_savedPlaylistModel->blockSignals(false);
}
