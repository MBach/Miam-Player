#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <QPainter>

#include "libraryfilterproxymodel.h"
#include "librarymodel.h"

class LibraryItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	LibraryFilterProxyModel* _proxy;
	LibraryModel* _libraryModel;

public:
	LibraryItemDelegate(LibraryFilterProxyModel *proxy);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/** Redefined to always display the same height for albums, even for those without one. */
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;

private:
	void drawAlbum(QPainter *painter, const QStyleOptionViewItem &o, LibraryItem *item) const;

	void drawArtist(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void drawLetter(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const;

	void drawTrack(QPainter *painter, QStyleOptionViewItem &option, const LibraryItem *item) const;
};

#endif // LIBRARYITEMDELEGATE_H
