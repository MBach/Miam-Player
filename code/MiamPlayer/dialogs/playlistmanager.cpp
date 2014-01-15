#include "playlistmanager.h"

#include <filehelper.h>
#include <settings.h>

#include <QDirIterator>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(const QSqlDatabase &db, TabPlaylist *tabPlaylist) :
	QDialog(tabPlaylist, Qt::Tool), _db(db), playlists(tabPlaylist)
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

	QStandardItemModel *modelU = new QStandardItemModel(this);
	QStandardItemModel *modelS = new QStandardItemModel(this);
	unsavedPlaylists->setModel(modelU);
	savedPlaylists->setModel(modelS);

	connect(unsavedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::feedPreviewFromUnsaved);
	connect(savedPlaylists->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PlaylistManager::feedPreviewFromSaved);
	connect(loadPlaylists, &QPushButton::clicked, this, &PlaylistManager::loadSavedPlaylists);
	connect(deletePlaylists, &QPushButton::clicked, this, &PlaylistManager::deleteSavedPlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, &PlaylistManager::dropAutoSavePlaylists);
	connect(qApp, &QApplication::aboutToQuit, this, &PlaylistManager::savePlaylists);

	connect(exportPlaylists, &QPushButton::clicked, this, &PlaylistManager::exportSelectedPlaylist);
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
		this->updatePlaylists();
		QStandardItemModel* savedModel = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
		for (int i = 0; i < savedModel->rowCount(); i++) {
			QStandardItem *item = savedModel->item(i, 0);
			this->loadPlaylist(item->data(PlaylistPath).toString(), item->data(IsPlaylistRegistered).toBool());
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

void PlaylistManager::loadPlaylist(const QString &path, bool isRegistered)
{
	qDebug() << Q_FUNC_INFO << path;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		Playlist *playlist = NULL;
		if (playlists->playlist(0) && playlists->playlist(0)->mediaPlaylist()->isEmpty()) {
			playlist = playlists->playlist(0);
			playlists->tabBar()->setTabText(0, this->getPlaylistName(path));
			//playlists->removeTabFromCloseButton(0);
		} else {
			playlist = playlists->addPlaylist();
			playlists->tabBar()->setTabText(playlists->count() - 2, this->getPlaylistName(path));
		}
		connect(playlist->mediaPlaylist(), &QMediaPlaylist::loaded, this, &PlaylistManager::checkPlaylistIntegrity);
		playlist->mediaPlaylist()->load(QUrl::fromLocalFile(path), "m3u8");
		file.close();
	}
}

bool PlaylistManager::savePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO;
	bool result = false;
	Playlist *p = playlists->playlist(index);
	if (p && !p->mediaPlaylist()->isEmpty()) {

		QString playlistName = playlists->tabBar()->tabText(index);
		QVariant playlistData = playlists->tabBar()->tabData(index);

		// New playlists have no existing file stored as Data member
		if (playlistData.isNull()) {
			QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
			playlistName = this->convertNameToValidFileName(playlistName);
			QString newPlaylistPath = path + QDir::separator() + playlistName + ".m3u8";
			int count = 0;
			// In case one has decided to create at least two playlists with the same name
			while (QFile::exists(newPlaylistPath)) {
				newPlaylistPath = path + QDir::separator() + playlistName + " (%1).m3u8";
				newPlaylistPath = newPlaylistPath.arg(++count);
			}
			QFile playlistFile(newPlaylistPath);
			playlistFile.open(QIODevice::WriteOnly);
			if (p->mediaPlaylist()->save(&playlistFile, "m3u8")) {

				playlistFile.close();
				QFileInfo fileInfo(playlistFile);
				bool o = _db.open();
				QString files;
				for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
					files.append(p->mediaPlaylist()->media(j).canonicalUrl().toLocalFile());
				}
				uint hash = qHash(files);
				QSqlQuery insertNewPlaylist("INSERT INTO playlists VALUES(?, ?, ?)", _db);
				insertNewPlaylist.addBindValue(fileInfo.absoluteFilePath());
				insertNewPlaylist.addBindValue(playlistName);
				insertNewPlaylist.addBindValue(hash);
				qDebug() << _db << o;
				bool b = insertNewPlaylist.exec();
				qDebug() << "inserting new playlist?" << b;
				qDebug() << "absPath" << fileInfo.absoluteFilePath();
				qDebug() << "name" << playlistName;
				qDebug() << "hash" << hash;
				if (b) {
					result = true;
				} // else?
				_db.close();
			}
		}
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

// After every playlist load,
void PlaylistManager::checkPlaylistIntegrity()
{
	qDebug() << "todo playlist integrity";
	/*_db.open();
	// Register new files if they are not in Playlists Table
	//QSqlQuery existingPlaylist = _db.exec("INSERT INTO playlists WHERE absPath = '" + path + "'");
	QString files;
	for (int j = 0; j < playlist->mediaPlaylist()->mediaCount(); j++) {
		files.append(playlist->mediaPlaylist()->media(j).canonicalUrl().toLocalFile());
	}
	uint hash = qHash(files);
	qDebug() << "new hash (to save?)" << hash;
	_db.close();*/
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
	QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	foreach (QModelIndex index, indexes) {
		QStandardItem *item = saved->itemFromIndex(index);
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
	QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	QStandardItem *item = saved->itemFromIndex(index);
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

void PlaylistManager::feedPreviewFromSaved(QItemSelection, QItemSelection)
{
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	bool empty = indexes.isEmpty();
	qDebug() << Q_FUNC_INFO << "empty ?" << empty;
	this->clearPreview(!empty);
	if (indexes.size() == 1) {
		QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
		QString path = saved->itemFromIndex(indexes.first())->data(PlaylistPath).toString();
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

void PlaylistManager::feedPreviewFromUnsaved(QItemSelection, QItemSelection)
{
	this->clearPreview();

	if (unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1) {

		/// TODO

	}
}

/** Load every selected playlists. */
void PlaylistManager::loadSavedPlaylists()
{
	qDebug() << Q_FUNC_INFO;
	QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	foreach (QModelIndex index, savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = saved->itemFromIndex(index);
		if (item) {
			this->loadPlaylist(item->data(PlaylistPath).toString());
		}
	}
}

/** Save all playlists when exiting the application (if enabled). */
void PlaylistManager::savePlaylists()
{
	Settings *settings = Settings::getInstance();
	if (settings->playbackKeepPlaylists()) {
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
	QStandardItemModel* unsavedModel = qobject_cast<QStandardItemModel*>(unsavedPlaylists->model());
	unsavedModel->clear();
	for (int i = 0; i < playlists->playlists().count(); i++) {
		if (playlists->tabBar()->tabData(i).isNull() && !playlists->playlist(i)->mediaPlaylist()->isEmpty()) {
			unsavedModel->appendRow(new QStandardItem(playlists->tabBar()->tabText(i)));
		}
	}

	// Populate saved playlists area
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QStringList supportedPlaylistTypes = QStringList() << "*.m3u" << "*.m3u8";
	QDir dataLocation(path);
	dataLocation.setNameFilters(supportedPlaylistTypes);
	QDirIterator it(dataLocation);

	QStandardItemModel* savedModel = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	savedModel->clear();

	while (it.hasNext()) {
		it.next();
		QString name = this->getPlaylistName(it.fileInfo().absoluteFilePath());
		QStandardItem *savedItem = new QStandardItem;
		if (name == "") {
			savedItem->setText(it.fileInfo().baseName());
		} else {
			savedItem->setText(name);
		}
		savedModel->appendRow(savedItem);
		savedItem->setData(it.filePath(), PlaylistPath);
		savedItem->setData(name == "", IsPlaylistRegistered);
	}
}
