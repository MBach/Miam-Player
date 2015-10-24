#ifndef MIAMSORTFILTERPROXYMODEL_H
#define MIAMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "miamcore_global.h"

/// Forward declaration
class SeparatorItem;

/**
 * \brief		The MiamSortFilterProxyModel class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MiamSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
protected:
	/** Top levels items are specific items, like letters 'A', 'B', ... in the library. Each letter has a reference to all items beginning with this letter. */
	QMultiHash<SeparatorItem*, QModelIndex> _topLevelItems;

public:
	MiamSortFilterProxyModel(QObject *parent = 0);

	inline void setTopLevelItems(const QMultiHash<SeparatorItem*, QModelIndex> &topLevelItems) { _topLevelItems = topLevelItems; }

	void findMusic(const QString &text);

	/** Highlight items in the Tree when one has activated this option in settings. */
	void highlightMatchingText(const QString &text);

protected:
	virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;

private:
	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

signals:
	void aboutToHighlightLetters(const QSet<QChar> &letters);
};

#endif // MIAMSORTFILTERPROXYMODEL_H
