#ifndef PLAYLISTHEADERVIEW_H
#define PLAYLISTHEADERVIEW_H

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>

#include "playlist.h"

class PlaylistHeaderView : public QHeaderView
{
	Q_OBJECT

private:
	QMenu *_columns;

public:
	static QStringList labels;

	explicit PlaylistHeaderView(Playlist *parent);

	/** Redefined to initialize values from settings. */
	void setFont(const QFont &newFont);

	/** Redefined. */
	void setModel(QAbstractItemModel *model);

protected:
	/** Redefined for dynamic translation. */
	virtual void changeEvent(QEvent *event);

	/** Redefined. */
	virtual void contextMenuEvent(QContextMenuEvent *event);

	/** Redefined. */
	virtual void paintEvent(QPaintEvent *);
};

#endif // PLAYLISTHEADERVIEW_H
