#include "playlistmanager.h"

#include <filehelper.h>
#include <settings.h>

#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <QtDebug>

PlaylistManager::PlaylistManager(TabPlaylist *tabPlaylist) :
	QDialog(tabPlaylist), playlists(tabPlaylist)
{
	setupUi(this);
	this->setWindowFlags(Qt::Tool);

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

	//connect(unsavedPlaylists->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
	//		this, SLOT(test(const QModelIndex &, int, int)));
	connect(unsavedPlaylists->model(), &QStandardItemModel::rowsRemoved, this, &PlaylistManager::test);

	/*connect(this, &PlaylistManager::close, [=]() {
		int r = 0;
		qDebug() << "closing";
		this->done(r);
	});*/
	connect(qApp, &QApplication::aboutToQuit, this, &PlaylistManager::savePlaylists);
}

void PlaylistManager::clearPlaylist(int i)
{
	delete map.value(i);
	map.remove(i);
}

#include <QDateTime>

void PlaylistManager::savePlaylists()
{
	Settings *settings = Settings::getInstance();
	if (settings->playbackKeepPlaylists()) {
		QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
		for (int i = 0; i < playlists->count(); i++) {
			Playlist *p = playlists->playlist(i);
			if (p && !p->mediaPlaylist()->isEmpty()) {
				QString playlistName = playlists->tabBar()->tabText(i);
				QVariant playlistData = playlists->tabBar()->tabData(i);

				// New playlists have no existing file stored as Data member
				if (playlistData.isNull()) {
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
						settings->setValue(newPlaylistPath, playlistName);
						playlistFile.close();
						settings->setValue(newPlaylistPath + "_lastModified", QFileInfo(playlistFile).lastModified());
					}
				} else {
					// Ensure that file still exists meanwhile
					QString existingPath = playlistData.toString();
					if (QFile::exists(existingPath)) {

						// Ensure that previous playlist wasn't modified to avoid overwriting every time the player is closed
						QFileInfo playlistFileInfo(existingPath);
						QDateTime lastModified = settings->value(existingPath + "_lastModified").toDateTime();

						// We need to replace content if the date was modified
						qDebug() << lastModified << playlistFileInfo.lastModified();
						if (lastModified != playlistFileInfo.lastModified()) {
							QFile existingFile(existingPath);
							existingFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
							if (p->mediaPlaylist()->save(&existingFile, "m3u8")) {
								settings->setValue(existingPath, playlistName);
								existingFile.close();
								settings->setValue(existingPath + "_lastModified", QFileInfo(existingPath).lastModified());
							}
						}
					} else {
						qDebug() << "you have deleted the playlist!";
					}
				}
			}
		}
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
	//Settings *settings = Settings::getInstance();
	//settings->save(p)
	for (int i = 0; i < playlists->count()-1; i++) {
		//Playlist *p = playlists->playlist(i);
		if (!map.contains(i)) {
			//QListWidgetItem *item;
			QStandardItem *item;
			/*if (p->isSaved()) {
				item = new QListWidgetItem(playlists->tabText(i), savedPlaylists);
				savedPlaylists->addItem(item);
			} else {*/
				//item = new QListWidgetItem(playlists->tabText(i), unsavedPlaylists);
				item = new QStandardItem(playlists->tabText(i));
				QStandardItemModel* m = qobject_cast<QStandardItemModel*>(unsavedPlaylists->model());
				m->appendRow(item);
			//}*/
			map.insert(i, item);
		} else {
			QStandardItem *item = map.value(i);
			if (item->text() != playlists->tabText(i)) {
				item->setText(playlists->tabText(i));
			}
		}
	}
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
			QString playlistName = Settings::getInstance()->value(absFilePath).toString();
			playlists->tabBar()->setTabText(playlists->playlists().count() - 1, playlistName);
			playlists->tabBar()->setTabData(playlists->playlists().count() - 1, absFilePath);
			playlist->mediaPlaylist()->load(QUrl::fromLocalFile(absFilePath), fileInfo.suffix().toStdString().data());
		}
	}
	if (playlists->playlists().isEmpty()) {
		playlists->addPlaylist();
	}
}

void PlaylistManager::open()
{
	this->updatePlaylists();
	QDialog::open();
}

void PlaylistManager::loadPreviewPlaylist(QListView *list)
{
	QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
	int index = map.key(m->itemFromIndex(list->currentIndex()));
	Playlist *p = playlists->playlist(index);
	for (int i = 0; i < p->mediaPlaylist()->mediaCount(); i++) {
		FileHelper fh(p->mediaPlaylist()->media(i));
		previewPlaylist->insertItem(i, QString("%1 (%2 - %3)").arg(fh.title(), fh.artist(), fh.album()));
	}
}

void PlaylistManager::deleteSavedPlaylists()
{
	qDebug() << "todo deleteSavedPlaylists";
}

void PlaylistManager::feedPreviewFromSaved(QItemSelection, QItemSelection)
{
	previewPlaylist->clear();
	if (savedPlaylists->selectionModel()->selectedIndexes().size() == 1) {

		loadPlaylists->setEnabled(true);
		deletePlaylists->setEnabled(true);
		this->loadPreviewPlaylist(savedPlaylists);

	} else if (savedPlaylists->selectionModel()->selectedIndexes().isEmpty()) {

		loadPlaylists->setEnabled(false);
		deletePlaylists->setEnabled(false);

	}
}

void PlaylistManager::feedPreviewFromUnsaved(QItemSelection, QItemSelection)
{
	previewPlaylist->clear();
	if (unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1) {
		this->loadPreviewPlaylist(unsavedPlaylists);
	}
}

void PlaylistManager::loadSavedPlaylists()
{
	qDebug() << "todo loadSavedPlaylists";
}
