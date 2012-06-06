#include "playlistmanager.h"

#include <fileref.h>
#include <tag.h>

#include <QtDebug>

#include "settings.h"

#include <QStandardItemModel>
#include <QAbstractItemModel>

PlaylistManager::PlaylistManager(TabPlaylist *tabPlaylist, QWidget *parent) :
	QDialog(parent, Qt::Window), playlists(tabPlaylist)
{
	setupUi(this);

	unsavedPlaylists->installEventFilter(this);
	savedPlaylists->installEventFilter(this);

	QStandardItemModel *modelU = new QStandardItemModel(this);
	QStandardItemModel *modelS = new QStandardItemModel(this);
	unsavedPlaylists->setModel(modelU);
	savedPlaylists->setModel(modelS);

	unsavedPlaylists->selectionModel();

	connect(unsavedPlaylists->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(feedPreviewFromUnsaved(QItemSelection,QItemSelection)));
	connect(savedPlaylists->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(feedPreviewFromSaved(QItemSelection,QItemSelection)));
	connect(loadPlaylists, SIGNAL(clicked()), this, SLOT(loadSavedPlaylists()));
	connect(deletePlaylists, SIGNAL(clicked()), this, SLOT(deleteSavedPlaylists()));

	connect(playlists, SIGNAL(destroyed(int)), this, SLOT(clearPlaylist(int)));
	connect(playlists, SIGNAL(created()), this, SLOT(updatePlaylists()));

	connect(unsavedPlaylists->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
			this, SLOT(test(const QModelIndex &, int, int)));
}

void PlaylistManager::clearPlaylist(int i)
{
	delete map.value(i);
	map.remove(i);
}

void PlaylistManager::test(const QModelIndex &, int, int)
{
	qDebug() << "yay";
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

void PlaylistManager::open()
{
	this->updatePlaylists();
	QDialog::open();
}

void PlaylistManager::loadPreviewPlaylist(QListView *list)
{
	QModelIndex modelIndex = list->currentIndex();
	QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
	int index = map.key(m->itemFromIndex(modelIndex));
	Playlist *p = playlists->playlist(index);
	for (int i = 0; i < p->tracks().size(); i++) {
		TagLib::FileRef f(p->tracks().at(i).fileName().toLocal8Bit().data());

		// Build the item: "title (artist - album)"
		QString text = f.tag()->title().toCString();
		text.append(" (").append( f.tag()->artist().toCString()).append(" - ");
		text.append(f.tag()->album().toCString()).append(')');
		previewPlaylist->insertItem(i, text);
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
