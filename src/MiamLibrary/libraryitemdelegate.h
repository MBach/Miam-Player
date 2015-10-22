#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include "miamitemdelegate.h"
#include "libraryfilterproxymodel.h"
#include "discitem.h"
#include "separatoritem.h"
#include "trackitem.h"
#include "yearitem.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QStandardItem>
#include "miamlibrary_global.hpp"

class LibraryTreeView;

class MIAMLIBRARY_LIBRARY LibraryItemDelegate : public MiamItemDelegate
{
	Q_OBJECT
private:
	LibraryTreeView *_libraryTreeView;

public:
	explicit LibraryItemDelegate(LibraryTreeView *libraryTreeView, QSortFilterProxyModel *proxy);

	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	/** Redefined to always display the same height for albums, even for those without one. */
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
	/** Albums have covers usually. */
	void drawAlbum(QPainter *painter, QStyleOptionViewItem &option, AlbumItem *item) const;

	virtual void drawArtist(QPainter *painter, QStyleOptionViewItem &option, ArtistItem *item) const override;

	void drawDisc(QPainter *painter, QStyleOptionViewItem &option, DiscItem *item) const;

	virtual void drawTrack(QPainter *painter, QStyleOptionViewItem &option, TrackItem *track) const override;

	void paintCoverOnTrack(QPainter *painter, const QStyleOptionViewItem &option, const TrackItem *track) const;

	/** Check if color needs to be inverted then paint text. */
	void paintText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rectText, const QString &text, const QStandardItem *item) const;

public slots:
	void displayIcon(bool b);

	void updateCoverSize();
};

#endif // LIBRARYITEMDELEGATE_H
