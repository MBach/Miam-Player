#ifndef MIAMITEMMODEL_H
#define MIAMITEMMODEL_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <model/genericdao.h>
#include "separatoritem.h"

#include "miamlibrary_global.hpp"

/**
 * \brief		The MiamItemModel class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY MiamItemModel : public QStandardItemModel
{
	Q_OBJECT
protected:
	/** This hash is a cache, used to insert nodes in this tree at the right location. */
	QHash<uint, QStandardItem*> _hash;

	/** Letters are items to groups separate of top levels items (items without parent). */
	QHash<QString, SeparatorItem*> _letters;

	/** Letter L returns all Artists (e.g.) starting with L. */
	QMultiHash<SeparatorItem*, QModelIndex> _topLevelItems;

	QHash<QString, QStandardItem*> _tracks;

public:
	MiamItemModel(QObject *parent = 0);

	virtual QChar currentLetter(const QModelIndex &index) const = 0;

	inline QStandardItem* letterItem(const QString &letter) const { return _letters.value(letter); }

	virtual QSortFilterProxyModel* proxy() const = 0;

protected:
	SeparatorItem *insertSeparator(const QStandardItem *node);

	/** Recursively remove node and its parent if the latter has no more children. */
	void removeNode(const QModelIndex &node);

public slots:
	virtual void insertNode(GenericDAO *node) = 0;

	virtual void updateNode(GenericDAO *node);
};

#endif // MIAMITEMMODEL_H
