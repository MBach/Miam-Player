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
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistDialog::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsAboutToBeRemoved, this, &PlaylistDialog::dropAutoSavePlaylists);
	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, [=](const QModelIndex &, int, int) {
		this->updatePlaylists();
	});

	connect(_unsavedPlaylistModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item) {
		if (item) {
			if (Playlist *p = _unsaved.value(item)) {
				p->setTitle(item->text());
				p->setModified(true);
				emit aboutToRenamePlaylist(p);
			}
		}
	});

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistDialog::exportSelectedPlaylist);
}

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
		if (p && p->isModified()) {
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

	QList<PlaylistDAO> playlists;
	for (QModelIndex index : indexes) {
		PlaylistDAO tmpPlaylist;
		tmpPlaylist.setId(index.data(PlaylistID).toString());
		playlists << tmpPlaylist;
	}
	//
	if (SqlDatabase::instance()->removePlaylists(playlists)) {
		emit aboutToRemoveTabs(playlists);
	}

	this->clearPreview(false);
}

void PlaylistDialog::dropAutoSavePlaylists(const QModelIndex &, int start, int end)
{
	for (int i = start; i <= end; i++) {
		auto item = _unsavedPlaylistModel->item(start);
		if (item) {
			//uint playlistObjectPointer = item->data(PlaylistObjectPointer).toUInt();
			/// XXX: it's not really easy to read...
			// Find the playlist in the TabWidget
			/*for (int i = 0; i < _tabPlaylists->playlists().count(); i++) {
				if (playlistObjectPointer == _tabPlaylists->tabBar()->tabData(i).toUInt()) {
					if (this->savePlaylist(i) > 0) {
						_tabPlaylists->setTabIcon(i, _tabPlaylists->defaultIcon(QIcon::Disabled));
					}
				}
			}*/
		}
	}
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
		qDebug() << Q_FUNC_INFO << "playlistId" << playlistId;

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

/** Update saved playlists when one is adding a new one. */
void PlaylistDialog::updatePlaylists()
{
	// Populate saved playlists area
	_savedPlaylistModel->clear();
	_savedPlaylistModel->blockSignals(true);

	for (PlaylistDAO playlist : SqlDatabase::instance()->selectPlaylists()) {
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
