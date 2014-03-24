#ifndef PLAYLISTHEADERVIEW_H
#define PLAYLISTHEADERVIEW_H

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>

class PlaylistHeaderView : public QHeaderView
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

	/** Redefined. */
	virtual void paintSection(QPainter *, const QRect &rect, int logicalIndex) const;
};

#endif // PLAYLISTHEADERVIEW_H
