#ifndef PLAYLISTHEADERVIEW_H
#define PLAYLISTHEADERVIEW_H

#include "headerview.h"
#include <QContextMenuEvent>
#include <QMenu>

class PlaylistHeaderView : public HeaderView
{
	Q_OBJECT

private:
	QMenu *columns;

public:
	static QStringList labels;

	explicit PlaylistHeaderView(QWidget *parent = 0);

	/** Redefined. */
	void setModel(QAbstractItemModel *model);

protected:
	/** Redefined for dynamic translation. */
	virtual void changeEvent(QEvent *event);

	/** Redefined. */
	virtual void contextMenuEvent(QContextMenuEvent *event);
};

#endif // PLAYLISTHEADERVIEW_H
