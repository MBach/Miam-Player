#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include <styling/miamstyleditemdelegate.h>

#include "stareditor.h"
#include "miamtabplaylists_global.hpp"

class Playlist;

/**
 * \brief		The PlaylistItemDelegate class is a delegate used to display rows in a table.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY PlaylistItemDelegate : public MiamStyledItemDelegate
{
	Q_OBJECT
private:
	Playlist *_playlist;

public:
	enum EditMode { Editable, NoStarsYet, ReadOnly };

	explicit PlaylistItemDelegate(Playlist *playlist);

	/** Redefined. */
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

	virtual bool eventFilter(QObject *object, QEvent *event) override;

	/** Redefined. */
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
	void commitAndClose();
};

#endif // PLAYLISTITEMDELEGATE_H
