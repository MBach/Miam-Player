#ifndef PLAYLISTHEADERVIEW_H
#define PLAYLISTHEADERVIEW_H

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>

#include "playlist.h"
#include "miamtabplaylists_global.hpp"

/**
 * \brief		The PlaylistHeaderView class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY PlaylistHeaderView : public QHeaderView
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
	virtual void setModel(QAbstractItemModel *model) override;

protected:
	/** Redefined for dynamic translation. */
	virtual void changeEvent(QEvent *event) override;

	/** Redefined. */
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	/** Redefined. */
	virtual void paintEvent(QPaintEvent *) override;

	virtual QSize sectionSizeFromContents(int logicalIndex) const override;
};

#endif // PLAYLISTHEADERVIEW_H
