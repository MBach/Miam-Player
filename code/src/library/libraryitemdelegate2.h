#ifndef LIBRARYITEMDELEGATE2_H
#define LIBRARYITEMDELEGATE2_H

#include <QItemDelegate>
#include <QPainter>

#include "playlists/stareditor.h"

#include "libraryfilterproxymodel.h"
#include "librarymodel.h"

class LibraryItemDelegate2 : public QItemDelegate
{
	Q_OBJECT
private:
	LibraryFilterProxyModel* _proxy;
	LibraryModel* _libraryModel;

public:
	explicit LibraryItemDelegate2(LibraryFilterProxyModel *proxy);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/** Redefined to always display the same height for albums, even for those without one. */
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
};

#endif // LIBRARYITEMDELEGATE2_H
