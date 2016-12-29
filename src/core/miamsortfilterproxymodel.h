#ifndef MIAMSORTFILTERPROXYMODEL_H
#define MIAMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "miamcore_global.h"

/// Forward declaration
class SeparatorItem;

/**
 * \brief		The MiamSortFilterProxyModel class provides support for the MiamItemModel class.
 * \details		This class has 2 ways to filter music in a library when one is typing a string.
 *				-# By excluding all terms which are not matching the filter
 *				-# By keeping everything visible to the user, and highlighting (in bold) the terms that are matching input
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
	explicit MiamSortFilterProxyModel(QObject *parent = nullptr);

	virtual ~MiamSortFilterProxyModel() {}

	inline void setTopLevelItems(const QMultiHash<SeparatorItem*, QModelIndex> &topLevelItems) { _topLevelItems = topLevelItems; }

	/** Single entry point for filtering library, and dispatch to the chosen operation defined in settings. */
	void findMusic(const QString &text);

	/** Highlight items in the Tree when one has activated this option in settings. */
	void highlightMatchingText(const QString &text);

	/** For classes that are subclassing this filter, allow to change sort column (for models based on a Table for example). */
	virtual int defaultSortColumn() const { return 0; }

private:
	/** Reduce the size of the library when the user is typing text. */
	void filterLibrary(const QString &filter);

signals:
	void aboutToHighlightLetters(const QSet<QChar> &letters);
};

#endif // MIAMSORTFILTERPROXYMODEL_H
