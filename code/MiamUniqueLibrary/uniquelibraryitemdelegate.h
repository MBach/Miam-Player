#ifndef UNIQUELIBRARYITEMDELEGATE_H
#define UNIQUELIBRARYITEMDELEGATE_H

#include <miamitemdelegate.h>
#include <library/jumptowidget.h>
#include "miamuniquelibrary_global.h"

#include <trackitem.h>

/**
 * \brief		The UniqueLibraryItemDelegate class is used to render item in a specific way.
 * \details		This delegate is able to draw a cover on the left edge of a cover for example.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibraryItemDelegate : public MiamItemDelegate
{
	Q_OBJECT
private:
	JumpToWidget *_jumpTo;

public:
	explicit UniqueLibraryItemDelegate(JumpToWidget *jumpTo, QSortFilterProxyModel *proxy);

	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
	virtual void drawAlbum(QPainter *painter, QStyleOptionViewItem &option, AlbumItem *item) const override;

	virtual void drawArtist(QPainter *painter, QStyleOptionViewItem &option, ArtistItem *item) const override;

	virtual void drawTrack(QPainter *painter, QStyleOptionViewItem &option, TrackItem *track) const override;
};

#endif // UNIQUELIBRARYITEMDELEGATE_H
