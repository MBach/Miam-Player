#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include "../styling/miamstyleditemdelegate.h"

#include "stareditor.h"

class Playlist;

class PlaylistItemDelegate : public MiamStyledItemDelegate
{
	Q_OBJECT
private:
	Playlist *_playlist;

	QMap<int, StarEditor*> _editors;

public:
	enum EditMode { Editable, NoStarsYet, ReadOnly };

	explicit PlaylistItemDelegate(Playlist *playlist);

	/** Redefined. */
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;

	/** Redefined. */
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

protected:
	/** Redefined. */
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private slots:
	void commitAndClose();
};

#endif // PLAYLISTITEMDELEGATE_H
