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
	virtual QSize sectionSizeFromContents(int logicalIndex) const;

	/** Redefined for dynamic translation. */
	virtual void changeEvent(QEvent *event) override;

	/** Redefined. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	/** Redefined. */
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // PLAYLISTHEADERVIEW_H
