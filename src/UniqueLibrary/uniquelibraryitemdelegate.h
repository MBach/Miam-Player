#ifndef UNIQUELIBRARYITEMDELEGATE_H
#define UNIQUELIBRARYITEMDELEGATE_H

#include <library/jumptowidget.h>
#include <miamitemdelegate.h>
#include "tableview.h"
#include "miamuniquelibrary_global.hpp"

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
	explicit UniqueLibraryItemDelegate(TableView *tableView);

	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
	virtual void drawAlbum(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const override;

	virtual void drawArtist(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const override;

	void drawCover(QPainter *painter, const QStyleOptionViewItem &option, const QString &coverPath) const;

	virtual void drawDisc(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *item) const override;

	virtual void drawTrack(QPainter *painter, QStyleOptionViewItem &option, QStandardItem *track) const override;
};

#endif // UNIQUELIBRARYITEMDELEGATE_H
