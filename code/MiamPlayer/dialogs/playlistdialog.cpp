#include "playlistdialog.h"

#include <model/playlistdao.h>
#include <model/trackdao.h>
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

PlaylistDialog::PlaylistDialog(QWidget *parent) :
	QDialog(parent, Qt::Tool),
	_unsavedPlaylistModel(new QStandardItemModel(this)), _savedPlaylistModel(new QStandardItemModel(this))
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);
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

	connect(unsavedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistDialog::populatePreviewFromUnsaved);
	connect(savedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistDialog::populatePreviewFromSaved);
	connect(loadPlaylists, &QPushButton::clicked, this, &PlaylistDialog::loadSelectedPlaylists);
	connect(savePlaylists, &QPushButton::clicked, this, [=]() {
		for (QModelIndex idx : savedPlaylists->selectionModel()->selectedIndexes()) {
			uint playlistId = idx.data(PlaylistID).toUInt();
			for (int i = 0; i < _playlists.count(); i++) {
				Playlist *p = _playlists.at(i);
				if (playlistId == p->id()) {
					emit aboutToSavePlaylist(p, true);
					break;
				}
			}
		}
		this->updatePlaylists();
	});
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistDialog::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsAboutToBeRemoved, this, &PlaylistDialog::dropAutoSavePlaylists);
	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, [=](const QModelIndex &, int, int) {
		this->updatePlaylists();
	});

	connect(_unsavedPlaylistModel, &QStandardItemModel::itemChanged, this, &PlaylistDialog::renameItem);
	connect(_savedPlaylistModel, &QStandardItemModel::itemChanged, this, &PlaylistDialog::renameItem);

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistDialog::exportSelectedPlaylist);
}

#include <QLineEdit>

/** Add drag & drop processing. */
bool PlaylistDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Drop || event->type() == QEvent::DragEnter) {
		return QDialog::eventFilter(obj, event);
	} else if (event->type() == QEvent::Drop) {
		return QDialog::eventFilter(obj, event);
	} else if (obj == savedPlaylists && event->type() == QEvent::KeyPress) {
		// Bind common keys to useful actions
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Delete) {
			this->deleteSavedPlaylists();
		} else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {

			// If one is editing an item, don't load the playlist but rename it instead
			QList<QLineEdit*> ol = this->findChildren<QLineEdit*>();
			if (ol.isEmpty()) {
				this->loadSelectedPlaylists();
			} else {
				// Only one item can be edited at the same time
				if (auto item = _savedPlaylistModel->itemFromIndex(savedPlaylists->currentIndex())) {
					this->renameItem(item);
				}
			}
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

void PlaylistDialog::clearPreview(bool aboutToInsertItems)
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
QString PlaylistDialog::convertNameToValidFileName(QString &name)
{
	/// XXX: should be improved?
	static const QRegularExpression re("[/\\:*?\"<>|]");
	name.replace(re, "_");
	return name;
}

/** Redefined: clean preview area, populate once again lists. */
void PlaylistDialog::open()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->value("PlaylistManagerGeometry").isValid()) {
		this->restoreGeometry(settings->value("PlaylistManagerGeometry").toByteArray());
	}
	this->clearPreview(false);

	for (int i = 0; i < _playlists.count(); i++) {
		Playlist *p = _playlists.at(i);
		if (p && p->id() == 0 && !p->mediaPlaylist()->isEmpty()) {
			QStandardItem *item = new QStandardItem(p->title());
			_unsavedPlaylistModel->appendRow(item);
			_unsaved.insert(item, p);
		}
	}
	this->updatePlaylists();

	QDialog::open();
	this->activateWindow();
}

/** Delete from the file system every selected playlists. Cannot be canceled. */
void PlaylistDialog::deleteSavedPlaylists()
{
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	if (indexes.isEmpty()) {
		return;
	}

	QString deleteMessage = QApplication::translate("PlaylistManager", "You're about to delete %n playlist. Are you sure you want to continue?", "", indexes.size());
	if (QMessageBox::Ok != QMessageBox::warning(this, tr("Warning"), deleteMessage, QMessageBox::Ok, QMessageBox::Cancel)) {
		return;
	}

	for (QModelIndex index : indexes) {
		qDebug() << Q_FUNC_INFO << index.data(PlaylistID).toUInt() << index.data(PlaylistID).toString();
		emit aboutToDeletePlaylist(index.data(PlaylistID).toUInt());
	}

	this->clearPreview(false);
	this->updatePlaylists();
}

void PlaylistDialog::dropAutoSavePlaylists(const QModelIndex &, int start, int end)
{
	for (int i = start; i <= end; i++) {
		if (auto item = _unsavedPlaylistModel->item(start)) {
			Playlist *playlist = _unsaved.value(item);
			emit aboutToSavePlaylist(playlist, false);
			_unsaved.remove(item);
		}
	}
	this->updatePlaylists();
}

/** Export one playlist at a time. */
void PlaylistDialog::exportSelectedPlaylist()
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
void PlaylistDialog::loadSelectedPlaylists()
{
	for (QModelIndex index : savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
		if (item) {
			emit aboutToLoadPlaylist(item->data(PlaylistID).toUInt());
		}
	}
	this->close();
}

void PlaylistDialog::populatePreviewFromSaved(const QItemSelection &, const QItemSelection &)
{
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	bool empty = indexes.isEmpty();
	this->clearPreview(!empty);
	if (indexes.size() == 1) {
		uint playlistId = _savedPlaylistModel->itemFromIndex(indexes.first())->data(PlaylistID).toUInt();
		QList<TrackDAO> tracks = SqlDatabase::instance()->selectPlaylistTracks(playlistId);
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

	// Some work for the Save button
	if (empty) {
		savePlaylists->setEnabled(false);
	} else {
		bool allPlaylistsAreModified = true;
		QMap<uint, Playlist*> map;
		for (int i = 0; i < _playlists.count(); i++) {
			Playlist *p = _playlists.at(i);
			map.insert(p->id(), p);
		}
		for (QModelIndex idx : indexes) {
			uint playlistId = idx.data(PlaylistID).toUInt();
			if (map.contains(playlistId)) {
				Playlist *p = map.value(playlistId);
				allPlaylistsAreModified = allPlaylistsAreModified && p->isModified();
			}
		}
		savePlaylists->setEnabled(allPlaylistsAreModified);
	}
}

void PlaylistDialog::populatePreviewFromUnsaved(const QItemSelection &, const QItemSelection &)
{
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	bool b = unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1;
	this->clearPreview(b);
	if (b) {
		QStandardItem *item = _unsavedPlaylistModel->itemFromIndex(unsavedPlaylists->selectionModel()->selectedIndexes().first());
		Playlist *p = _unsaved.value(item);
		int max = qMin(p->mediaPlaylist()->mediaCount(), MAX_TRACKS_PREVIEW_AREA);
		for (int idxTrack = 0; idxTrack < max; idxTrack++) {
			QString title = p->model()->index(idxTrack, Playlist::COL_TITLE).data().toString();
			QString artist = p->model()->index(idxTrack, Playlist::COL_ARTIST).data().toString();
			QString album = p->model()->index(idxTrack, Playlist::COL_ALBUM).data().toString();
			QTreeWidgetItem *item = new QTreeWidgetItem;
			item->setText(0, QString("%1 (%2 - %3)").arg(title, artist, album));
			previewPlaylist->addTopLevelItem(item);
		}
		if (p->mediaPlaylist()->mediaCount() > MAX_TRACKS_PREVIEW_AREA) {
			QTreeWidgetItem *item = new QTreeWidgetItem;
			item->setText(0, tr("And more tracks..."));
			previewPlaylist->addTopLevelItem(item);
		}
	}
}

void PlaylistDialog::renameItem(QStandardItem *item)
{
	if (item) {
		if (Playlist *p = _unsaved.value(item)) {
			p->setTitle(item->text());
			emit aboutToRenamePlaylist(p);
		} else {
			PlaylistDAO dao = _saved.value(item);
			dao.setTitle(item->text());
			emit aboutToRenameDAO(dao);
		}
	}
}

/** Update saved playlists when one is adding a new one. */
void PlaylistDialog::updatePlaylists()
{
	// Populate saved playlists area
	_savedPlaylistModel->clear();
	_savedPlaylistModel->blockSignals(true);

	QMap<uint, Playlist*> map;
	for (int i = 0; i < _playlists.count(); i++) {
		Playlist *p = _playlists.at(i);
		if (p->id() != 0) {
			map.insert(p->id(), p);
		}
	}

	for (PlaylistDAO playlist : SqlDatabase::instance()->selectPlaylists()) {
		QStandardItem *item = new QStandardItem(playlist.title());
		item->setData(playlist.id(), PlaylistID);
		if (playlist.icon().isEmpty()) {
			Playlist *p = map.value(playlist.id().toUInt());
			if (p && p->isModified()) {
				item->setIcon(QIcon(":/icons/playlist_modified"));
				item->setData(true, PlaylistModified);
				item->setToolTip(tr("This playlist has changed"));
			} else {
				item->setIcon(QIcon(":/icons/playlist"));
			}
		} else {
			item->setIcon(QIcon(playlist.icon()));
		}
		_savedPlaylistModel->appendRow(item);
		_saved.insert(item, playlist);
	}
	_savedPlaylistModel->blockSignals(false);

	// Reset buttons status
	loadPlaylists->setEnabled(false);
	savePlaylists->setEnabled(false);
	deletePlaylists->setEnabled(false);
	exportPlaylists->setEnabled(false);
}
