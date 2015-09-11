#ifndef MIAMITEMDELEGATE_H
#define MIAMITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QTimer>
#include "libraryitemmodel.h"

#include "miamlibrary_global.h"

class MIAMLIBRARY_LIBRARY MiamItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
protected:
	static qreal _iconOpacity;

	QStandardItemModel *_libraryModel;
	QSortFilterProxyModel *_proxy;
	bool _showCovers;

	/** Cache for covers displayed in the tree view.
	 * This field is mutable because it's modified in paint() which is const by design.*/
	//mutable QHash<AlbumItem*, bool> _loadedCovers;

	/** This timer is used to animate album cover when one is scrolling.
	 * It improves reactivity of the UI by temporarily disabling painting events.
	 * When covers are becoming visible once again, they are redisplayed with a nice fading effect. */
	QTimer *_timer;

	int _coverSize;

public:
	MiamItemDelegate(QSortFilterProxyModel *proxy);
};

#endif // MIAMITEMDELEGATE_H
