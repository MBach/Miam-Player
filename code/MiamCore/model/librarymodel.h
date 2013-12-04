#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QFileInfo>
#include <QMap>

#include <filehelper.h>
#include <settings.h>

#include "libraryitemalbum.h"
#include "libraryitemartist.h"
#include "libraryitemdiscnumber.h"
#include "libraryitemletter.h"
#include "libraryitemtrack.h"
#include "persistentitem.h"

#include <QSet>
#include <QStandardItemModel>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY LibraryModel : public QStandardItemModel
{
	Q_OBJECT
private:
	QSet<PersistentItem*> _persistentItems;

public:

	LibraryModel(QObject *parent = 0);

	/** Removes everything. */
	void clear();

	//void removeNode(const QModelIndex &index);

	LibraryItem *itemFromIndex(const QModelIndex &index) const {
		return static_cast<LibraryItem*>(QStandardItemModel::itemFromIndex(index));
	}

	//QSet<PersistentItem*> persistentItems() const { return _persistentItems; }

	//QMap<QString, LibraryItemArtist*> artists() const { return _artists; }

signals:
	/** A flat file on your computer was successfully loaded. */
	void loadedFromFile();

	void progressChanged(int);

	void searchHasEnded();

	void trackCreated(PersistentItem *item);

public slots:
	/** Add (a path to) an icon to every album. */
	void addCoverPathToAlbum(const QString &fileName);

	/** Build a tree from a flat file saved on disk. */
	void loadFromFile();

	/** Read a file from the filesystem and adds it into the library. */
	void readFile(const QString &absFilePath);

	/** Save a tree to a flat file on disk. */
	void saveToFile();
};

#endif // LIBRARYMODEL_H
