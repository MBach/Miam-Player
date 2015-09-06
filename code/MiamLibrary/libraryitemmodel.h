#ifndef LIBRARYITEMMODEL_H
#define LIBRARYITEMMODEL_H

#include <QStandardItemModel>
#include <QSet>
#include <model/genericdao.h>
#include <filehelper.h>
#include "separatoritem.h"
#include "miamlibrary_global.h"

/**
 * \brief		The LibraryItemModel class is used to cache information from the database, in order to increase performance.
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryItemModel : public QStandardItemModel
{
	Q_OBJECT

	/** This hash is a cache, used to insert nodes in this tree at the right location. */
	QHash<uint, QStandardItem*> _hash;

	/** Letters are items to groups separate of top levels items (items without parent). */
	QHash<QString, SeparatorItem*> _letters;

	/** Letter L returns all Artists (e.g.) starting with L. */
	QMultiHash<SeparatorItem*, QModelIndex> _topLevelItems;

	QHash<QString, QStandardItem*> _tracks;

public:
	explicit LibraryItemModel(QObject *parent = nullptr);

	void clearCache();

	inline QStandardItem* letterItem(const QString &letter) const { return _letters.value(letter); }

	/** Rebuild the list of separators when one has changed grammatical articles in options. */
	void rebuildSeparators();

	void reset();

	inline QMultiHash<SeparatorItem*, QModelIndex> topLevelItems() const { return _topLevelItems; }

private:
	SeparatorItem *insertSeparator(const QStandardItem *node);

	/** Recursively remove node and its parent if the latter has no more children. */
	void removeNode(const QModelIndex &node);

public slots:
	void cleanDanglingNodes();

	/** Find and insert a node in the hierarchy of items. */
	void insertNode(GenericDAO *node);

	void updateNode(GenericDAO *node);
};

#endif // LIBRARYITEMMODEL_H
