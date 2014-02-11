#ifndef PLAYLISTITEMDELEGATE_H
#define PLAYLISTITEMDELEGATE_H

#include <QStyledItemDelegate>

class Playlist;

class PlaylistItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	Playlist *_playlist;

public:
	explicit PlaylistItemDelegate(Playlist *playlist);

protected:
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // PLAYLISTITEMDELEGATE_H
