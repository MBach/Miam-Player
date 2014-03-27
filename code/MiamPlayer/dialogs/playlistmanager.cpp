#include "playlistmanager.h"

#include <filehelper.h>
#include <settings.h>

#include <QDirIterator>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(const QSqlDatabase &db, TabPlaylist *tabPlaylist) :
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

	unsavedPlaylists->setModel(_unsavedPlaylistModel);
	savedPlaylists->setModel(_savedPlaylistModel);

	connect(unsavedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromUnsaved);
	connect(savedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::populatePreviewFromSaved);
	connect(_savedPlaylistModel, &QStandardItemModel::itemChanged, [=](QStandardItem *item) {
		_db.open();
		QSqlQuery update(_db);
		update.prepare("UPDATE playlists SET name = :name WHERE absPath = :path");
		update.bindValue(":name", item->text());
		update.bindValue(":path", item->data(PlaylistPath).toString());
		qDebug() << "update?" << update.exec();
		_db.close();
	});
	connect(loadPlaylists, &QPushButton::clicked, this, &PlaylistManager::loadSelectedPlaylists);
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistManager::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, &PlaylistManager::dropAutoSavePlaylists);
	connect(qApp, &QApplication::aboutToQuit, this, &PlaylistManager::savePlaylists);

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistManager::exportSelectedPlaylist);
	// Save playlist on close (if enabled)
	connect(playlists, &TabPlaylist::aboutToSavePlaylist, this, &PlaylistManager::saveAndRemovePlaylist);
	connect(this, &PlaylistManager::playlistSaved, playlists, &TabPlaylist::removeTabFromCloseButton);
}

bool PlaylistManager::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Drop || event->type() == QEvent::DragEnter) {
		//qDebug() << "todo deplacer" << event->type();
		return QDialog::eventFilter(obj, event);
	} else if (obj == playlists) {
		//qDebug() << "playlists changed!" << event->type();
		return QDialog::eventFilter(obj, event);
	} else if (event->type() == QEvent::Drop) {
		qDebug() << "savedPlaylists";
		return QDialog::eventFilter(obj, event);
	} else {
		// standard event processing
		return QDialog::eventFilter(obj, event);
	}
}

void PlaylistManager::init()
{
	playlists->blockSignals(true);
	if (Settings::getInstance()->playbackRestorePlaylistsAtStartup()) {	

		// Populate saved playlists area
		QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
		QStringList supportedPlaylistTypes = QStringList() << "*.m3u" << "*.m3u8";
		QDir dataLocation(path);
		dataLocation.setNameFilters(supportedPlaylistTypes);
		QDirIterator it(dataLocation);
		while (it.hasNext()) {
			this->loadPlaylist(it.next());
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
	static const QRegularExpression re("[/\\:*?\"<>|]");
	name.replace(re, "_");
	return name;
}

QString PlaylistManager::getPlaylistName(const QString &path)
{
	_db.open();
	QSqlQuery selectName(_db);
	selectName.prepare("SELECT name FROM playlists WHERE absPath = :path");
	selectName.bindValue(":path", path);
	QString name = "";
	if (selectName.exec() && selectName.next()) {
		name = selectName.record().value(0).toString();
	} else {
		name = QFileInfo(path).baseName();
	}
	_db.close();
	return name;
}

/** Load a playlist (*.m3u8) saved on the filesystem. */
void PlaylistManager::loadPlaylist(const QString &path)
{
	qDebug() << Q_FUNC_INFO << path;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		Playlist *playlist = NULL;
		if (playlists->playlist(0) && playlists->playlist(0)->mediaPlaylist()->isEmpty()) {
			playlist = playlists->playlist(0);
			playlists->tabBar()->setTabText(0, this->getPlaylistName(path));
			playlists->tabBar()->setTabData(0, path);
		} else {
			playlist = playlists->addPlaylist();
			playlists->tabBar()->setTabText(playlists->count() - 2, this->getPlaylistName(path));
			playlists->tabBar()->setTabData(playlists->count() - 2, path);
		}
		connect(playlist->mediaPlaylist(), &QMediaPlaylist::loaded, [=]() {
			playlists->setTabIcon(playlists->currentIndex(), playlists->defaultIcon(QIcon::Disabled));
		});

		playlist->mediaPlaylist()->load(QUrl::fromLocalFile(path), "m3u8");
		file.close();

		// Get the previously generated hash to be able to know if this playlist will be modified later
		_db.open();
		QSqlQuery hash(_db);
		hash.prepare("SELECT hash FROM playlists WHERE absPath = :path");
		hash.bindValue(":path", path);
		if (hash.exec() && hash.next()) {
			playlist->setHash(hash.record().value(0).toUInt());
			qDebug() << "loadPlaylist() hash" << hash.record().value(0).toUInt();
		}
		_db.close();
	}
}

bool PlaylistManager::savePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO;
	bool result = false;
	Playlist *p = playlists->playlist(index);
	if (p && !p->mediaPlaylist()->isEmpty()) {
		_db.open();
		QString playlistName = playlists->tabBar()->tabText(index);
		QVariant playlistData = playlists->tabBar()->tabData(index);

		QString playlistPath;
		// New playlists have no existing file stored as Data member
		if (playlistData.toUInt() != 0) {
			QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
			playlistName = this->convertNameToValidFileName(playlistName);
			playlistPath = path + QDir::separator() + playlistName + ".m3u8";
			int count = 0;
			// In case one has decided to create at least two playlists with the same name
			while (QFile::exists(playlistPath)) {
				playlistPath = path + QDir::separator() + playlistName + " (%1).m3u8";
				playlistPath = playlistPath.arg(++count);
			}

		} else if (!playlistData.toString().isEmpty()) {
			playlistPath = playlistData.toString();
			qDebug() << "overwriting" << playlistName << playlistPath;

			QSqlQuery deleteExistingPlaylist(_db);
			deleteExistingPlaylist.prepare("DELETE FROM playlists WHERE absPath = :path");
			deleteExistingPlaylist.bindValue(":path", playlistPath);
			deleteExistingPlaylist.exec();
		}

		// Then create or overwrite a file on the hard drive
		QFile playlistFile(playlistPath);
		playlistFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
		if (p->mediaPlaylist()->save(&playlistFile, "m3u8")) {

			playlistFile.close();
			QFileInfo fileInfo(playlistFile);
			QString files;
			for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
				files.append(p->mediaPlaylist()->media(j).canonicalUrl().toLocalFile());
			}
			uint hash = qHash(files);
			qDebug() << "savePlaylist() << new hash generated" << hash;

			/// XXX: extract every sql query elsewhere or not?
			QSqlQuery insertNewPlaylist("INSERT INTO playlists(absPath, name, hash) VALUES(?, ?, ?)", _db);
			insertNewPlaylist.addBindValue(fileInfo.absoluteFilePath());
			insertNewPlaylist.addBindValue(playlistName);
			insertNewPlaylist.addBindValue(hash);
			result = insertNewPlaylist.exec();
		}
		_db.close();
	}
	return result;
}

/** Redefined: clean preview area, populate once again lists. */
void PlaylistManager::open()
{
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

	// Delete every selected playlist on FS
	foreach (QModelIndex index, indexes) {
		QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
		QString path = item->data(PlaylistPath).toString();
		QFile playlistToRemove(path);
		if (playlistToRemove.exists()) {
			playlistToRemove.remove();
		}
	}
	this->updatePlaylists();
}

void PlaylistManager::dropAutoSavePlaylists(const QModelIndex &, int start, int end)
{
	qDebug() << "todo drop autosave" << start << end;
	QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
	//Playlist *p;
	//playlists->mediaPlayer().data()->playlist();
}

/** Export one playlist at a time. */
void PlaylistManager::exportSelectedPlaylist()
{
	QString exportedPlaylistLocation;
	Settings *settings = Settings::getInstance();
	if (settings->value("locationForExportedPlaylist").isNull()) {
		exportedPlaylistLocation = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
	} else {
		exportedPlaylistLocation = settings->value("locationForExportedPlaylist").toString();
	}
	QModelIndex index = savedPlaylists->selectionModel()->selectedIndexes().first();
	QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
	QString pPath = item->data(PlaylistPath).toString();
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
	// After copy, keeps the directory chosen by one. It's quite convenient when you are far from QStandardPaths::MusicLocation
	if (QFile::copy(pPath, newName)) {
		settings->setValue("locationForExportedPlaylist", QFileInfo(newName).absolutePath());
	} else {
		QString error = tr("Unfortunately, an error occured when MmeMiamMiam tried to export playlist '%1' to '%2'.\n"\
						   "Please, would you be nice to check if the file isn't opened elsewhere?")
				.arg(item->data(Qt::DisplayRole).toString(), QDir::toNativeSeparators(newName));
		QMessageBox::warning(this, tr("Cannot export the selected playlist"), error);
	}
}

/** Load every saved playlists. */
void PlaylistManager::loadSelectedPlaylists()
{
	qDebug() << Q_FUNC_INFO;
	foreach (QModelIndex index, savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = _savedPlaylistModel->itemFromIndex(index);
		if (item) {
			this->loadPlaylist(item->data(PlaylistPath).toString());
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
		QString path = _savedPlaylistModel->itemFromIndex(indexes.first())->data(PlaylistPath).toString();
		QFile file(path);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			int trackCount = 0;
			while (trackCount < MAX_TRACKS_PREVIEW_AREA) {
				char buf[1024];
				file.readLine(buf, sizeof(buf));
				QUrl url = QUrl::fromUserInput(buf);
				if (url.isValid()) {
					trackCount++;
					QMediaContent mc(url);
					FileHelper fh(mc);
					QTreeWidgetItem *item = new QTreeWidgetItem;
					item->setText(0, QString("%1 (%2 - %3)").arg(fh.title(), fh.artist(), fh.album()));
					previewPlaylist->addTopLevelItem(item);
				}
				if (!file.canReadLine()) {
					break;
				}
			}
			if (trackCount == MAX_TRACKS_PREVIEW_AREA) {
				QTreeWidgetItem *item = new QTreeWidgetItem;
				item->setText(0, tr("And more tracks..."));
				previewPlaylist->addTopLevelItem(item);
			}
			file.close();
		} else {
			// seriously, one has deleted it meanwhile? (QLockFile?)
			qDebug() << "unable to open" << path;
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
		uint playlistObjectPointer = item->data(PlaylistObjectPointer).toUInt();
		qDebug() << "now, finds the playlist with hash" << playlistObjectPointer;
		for (int i = 0; i < playlists->playlists().count(); i++) {
			Playlist *p = playlists->playlist(i);
			uint hash = qHash(p);
			if (hash == playlistObjectPointer) {
				qDebug() << "feed";
				int max = qMin(p->mediaPlaylist()->mediaCount(), MAX_TRACKS_PREVIEW_AREA);
				for (int idxTrack = 0; idxTrack < max; idxTrack++) {
					QMediaContent mc(p->mediaPlaylist()->media(idxTrack).canonicalUrl());
					FileHelper fh(mc);
					QTreeWidgetItem *item = new QTreeWidgetItem;
					item->setText(0, QString("%1 (%2 - %3)").arg(fh.title(), fh.artist(), fh.album()));
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
	if (Settings::getInstance()->playbackKeepPlaylists()) {
		_db.open();
		for (int i = 0; i < playlists->count(); i++) {
			this->savePlaylist(i);
		}
		_db.close();
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
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QStringList supportedPlaylistTypes = QStringList() << "*.m3u" << "*.m3u8";
	QDir dataLocation(path);
	dataLocation.setNameFilters(supportedPlaylistTypes);
	QDirIterator it(dataLocation);

	_savedPlaylistModel->clear();
	_savedPlaylistModel->blockSignals(true);
	while (it.hasNext()) {
		it.next();
		QString name = this->getPlaylistName(it.fileInfo().absoluteFilePath());
		QStandardItem *savedItem = new QStandardItem;
		if (name == "") {
			savedItem->setText(it.fileInfo().baseName());
		} else {
			savedItem->setText(name);
		}
		_savedPlaylistModel->appendRow(savedItem);
		savedItem->setData(it.filePath(), PlaylistPath);
		savedItem->setData(name == "", IsPlaylistRegistered);
	}
	_savedPlaylistModel->blockSignals(false);
}
