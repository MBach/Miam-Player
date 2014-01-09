#include "playlistmanager.h"

#include <filehelper.h>
#include <settings.h>

#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(const QSqlDatabase &db, TabPlaylist *tabPlaylist) :
	QDialog(tabPlaylist), _db(db), playlists(tabPlaylist)
{
	setupUi(this);
	this->setWindowFlags(Qt::Tool);

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

	connect(playlists, &TabPlaylist::destroyed, this, &PlaylistManager::clearPlaylist);
	connect(playlists, &TabPlaylist::created, this, &PlaylistManager::updatePlaylists);

	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, &PlaylistManager::test);
	connect(qApp, &QApplication::aboutToQuit, this, &PlaylistManager::savePlaylists);
}

void PlaylistManager::clearPlaylist(int i)
{
	delete map.value(i);
	map.remove(i);
}

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

void PlaylistManager::test(const QModelIndex &, int start, int end)
{
	qDebug() << "yay" << start << end;
	QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
	//Playlist *p;
	//playlists->mediaPlayer().data()->playlist();
}

void PlaylistManager::updatePlaylists()
{
	// Populate unsaved playlists area
	QStandardItemModel* unsavedModel = qobject_cast<QStandardItemModel*>(unsavedPlaylists->model());
	unsavedModel->clear();

	// Populate saved playlists area
	QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QStringList supportedPlaylistTypes = QStringList() << "*.m3u" << "*.m3u8";
	QDir dataLocation(path);
	dataLocation.setNameFilters(supportedPlaylistTypes);
	QDirIterator it(dataLocation);

	QStandardItemModel* savedModel = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	savedModel->clear();

	_db.open();
	while (it.hasNext()) {
		it.next();
		QSqlQuery selectName(_db);
		selectName.prepare("SELECT name FROM playlists WHERE absPath = :path");
		selectName.bindValue(":path", it.fileInfo().absoluteFilePath());
		QStandardItem *savedItem = new QStandardItem();
		if (selectName.exec()) {
			selectName.next();
			QString name = selectName.record().value(0).toString();
			savedItem->setText(name);
			qDebug() << "name extracted from database for file" << it.fileName() << name;
		} else {
			savedItem->setText(it.fileName());
		}
		savedModel->appendRow(savedItem);
		savedItem->setData(path, Qt::UserRole + 1);
	}
	_db.close();
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
	if (Settings::getInstance()->playbackRestorePlaylistsAtStartup()) {
		QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
		QDir dataLocation(path);
		QDirIterator it(dataLocation);
		QStringList supportedPlaylistTypes = QStringList() << "m3u" << "m3u8";
		while (it.hasNext()) {
			it.next();
			QFileInfo fileInfo(it.fileInfo());
			if (supportedPlaylistTypes.contains(fileInfo.suffix())) {
				Playlist *playlist = playlists->addPlaylist();
				QString absFilePath = fileInfo.absoluteFilePath();
				//QString playlistName = Settings::getInstance()->playlistLoad(fileInfo);
				//playlists->tabBar()->setTabText(playlists->playlists().count() - 1, playlistName);
				//playlists->tabBar()->setTabData(playlists->playlists().count() - 1, absFilePath);
				//playlist->mediaPlaylist()->load(QUrl::fromLocalFile(absFilePath), fileInfo.suffix().toStdString().data());
			}
		}
	}
	if (playlists->playlists().isEmpty()) {
		playlists->addPlaylist();
	}
}

void PlaylistManager::saveAndRemovePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO << index;
	if (this->savePlaylist(index)) {
		emit playlistSaved(index);
	}
}

void PlaylistManager::open()
{
	qDebug() << Q_FUNC_INFO;
	this->updatePlaylists();
	QDialog::open();
}

bool PlaylistManager::savePlaylist(int index)
{
	bool result = false;
	Playlist *p = playlists->playlist(index);
	uint hash = 0;
	if (p) {
		QString files;
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			files.append(p->mediaPlaylist()->media(j).canonicalUrl().toLocalFile());
		}
		hash = qHash(files);
	}

	QString playlistName = playlists->tabBar()->tabText(index);
	QVariant playlistData = playlists->tabBar()->tabData(index);

	// New playlists have no existing file stored as Data member
	if (playlistData.isNull()) {
		QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
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
	} else {

	}
	return result;
}

void PlaylistManager::deleteSavedPlaylists()
{
	qDebug() << "todo deleteSavedPlaylists";
}

void PlaylistManager::feedPreviewFromSaved(QItemSelection, QItemSelection)
{
	qDebug() << Q_FUNC_INFO;
	previewPlaylist->clear();
	QModelIndexList indexes = savedPlaylists->selectionModel()->selectedIndexes();
	static const int MAX_TRACKS_PREVIEW_AREA = 30;
	if (indexes.size() == 1) {
		QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
		QString path = saved->itemFromIndex(indexes.first())->data(Qt::UserRole + 1).toString();
		//playlists->mediaPlayer().data()->playlist()->load();
		QFile file(path);
		if (file.open(QIODevice::ReadOnly)) {
			int trackCount = 0;
			char buf[1024];
			while (trackCount < MAX_TRACKS_PREVIEW_AREA || file.canReadLine()) {
				QString line = QString::fromUtf8(buf.constData() + startIndex, length).trimmed();
			}
		} else {
			// seriously, one has deleted it meanwhile? (QLockFile?)
		}
	}
	bool empty = indexes.isEmpty();
	loadPlaylists->setDisabled(empty);
	deletePlaylists->setDisabled(empty);
	exportPlaylists->setDisabled(empty);
}

void PlaylistManager::feedPreviewFromUnsaved(QItemSelection, QItemSelection)
{
	previewPlaylist->clear();
	if (unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1) {

		QStandardItemModel *unsaved = qobject_cast<QStandardItemModel*>(unsavedPlaylists->model());
		int index = map.key(unsaved->itemFromIndex(unsavedPlaylists->currentIndex()));
		Playlist *p = playlists->playlist(index);
		for (int i = 0; i < p->mediaPlaylist()->mediaCount(); i++) {
			FileHelper fh(p->mediaPlaylist()->media(i));
			previewPlaylist->insertItem(i, QString("%1 (%2 - %3)").arg(fh.title(), fh.artist(), fh.album()));
		}

	}
}

void PlaylistManager::loadSavedPlaylists()
{
	qDebug() << "todo loadSavedPlaylists";
}
