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
			this->loadPlaylist(savedModel->item(i, 0)->data(Qt::UserRole + 1).toString());
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
		savedItem->setData(it.filePath(), Qt::UserRole + 1);
	}
}

void PlaylistManager::loadPlaylist(const QString &path)
{
	qDebug() << Q_FUNC_INFO << path;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		Playlist *playlist = NULL;
		if (playlists->playlist(0) && playlists->playlist(0)->mediaPlaylist()->isEmpty()) {
			playlist = playlists->playlist(0);
			playlists->tabBar()->setTabText(0, this->getPlaylistName(path));
		} else {
			playlist = playlists->addPlaylist();
			playlists->tabBar()->setTabText(playlists->count() - 2, this->getPlaylistName(path));
		}
		playlist->mediaPlaylist()->load(QUrl::fromLocalFile(path), "m3u8");
		file.close();
	}
}

bool PlaylistManager::savePlaylist(int index)
{
	qDebug() << Q_FUNC_INFO;
	bool result = false;
	Playlist *p = playlists->playlist(index);
	uint hash = 0;
	if (p && !p->mediaPlaylist()->isEmpty()) {
		QString files;
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			files.append(p->mediaPlaylist()->media(j).canonicalUrl().toLocalFile());
		}
		hash = qHash(files);

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

void PlaylistManager::deleteSavedPlaylists()
{
	qDebug() << "todo deleteSavedPlaylists";
}

/** Load every selected playlists. */
void PlaylistManager::loadSavedPlaylists()
{
	qDebug() << Q_FUNC_INFO;
	QStandardItemModel *saved = qobject_cast<QStandardItemModel*>(savedPlaylists->model());
	foreach (QModelIndex index, savedPlaylists->selectionModel()->selectedIndexes()) {
		QStandardItem *item = saved->itemFromIndex(index);
		if (item) {
			this->loadPlaylist(item->data(Qt::UserRole + 1).toString());
		}
	}
}

void PlaylistManager::dropAutoSavePlaylists(const QModelIndex &, int start, int end)
{
	qDebug() << "yay" << start << end;
	QString path = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
	//Playlist *p;
	//playlists->mediaPlayer().data()->playlist();
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
		QString path = saved->itemFromIndex(indexes.first())->data(Qt::UserRole + 1).toString();
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
	exportPlaylists->setDisabled(empty);
}

void PlaylistManager::feedPreviewFromUnsaved(QItemSelection, QItemSelection)
{
	this->clearPreview();

	if (unsavedPlaylists->selectionModel()->selectedIndexes().size() == 1) {

		/// TODO

	}
}


