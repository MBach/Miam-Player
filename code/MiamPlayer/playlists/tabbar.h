#ifndef TABBAR_H
#define TABBAR_H

#include <QLineEdit>
#include <QMouseEvent>
#include <QTabBar>
#include <QTimer>

#include "playlist.h"
#include "tabplaylist.h"

/**
 * \brief		The TabBar class is used to be able to rename a tab, e.g. a Playlist.
 * \details		By double clicking or using the contextual menu, one can interact with the name of every playlists in this Widget.
 *				It also forwards drag & drop events to automatically switch from one tab to another.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TabBar : public QTabBar
{
	Q_OBJECT
private:
	QLineEdit *lineEdit;

	TabPlaylist *tabPlaylist;

	QRect _targetRect;
	bool _cursorOverSameTab;
	QTimer *_timer;

public:
	explicit TabBar(TabPlaylist *parent);

	/** Trigger a double click to rename a tab. */
	void editTab(int indexTab);

protected:
	/** Redefined to accept D&D from another playlist or the library. */
	virtual void dragEnterEvent(QDragEnterEvent *event);

	/** Redefined to accept D&D from another playlist or the library. */
	virtual void dragMoveEvent(QDragMoveEvent *event);

	/** Redefined to accept D&D from another playlist or the library. */
	virtual void dropEvent(QDropEvent *event);

	/** Redefined to validate new tab name if the focus is lost. */
	virtual bool eventFilter(QObject *, QEvent *);

	/** Redefined to display an editable area. */
	virtual void mouseDoubleClickEvent(QMouseEvent *);

	/** Redefined to validate new tab name without pressing return. */
	virtual void mousePressEvent(QMouseEvent *);

	virtual void paintEvent(QPaintEvent *);

	/** Redefined to return a square for the last tab which is the [+] button. */
	virtual QSize tabSizeHint(int index) const;

private:
	void paintRectTabs(QStylePainter &p);

	void paintRoundedTabs(QStylePainter &p, int dist);

private slots:
	/** Rename a tab. */
	void renameTab();
};

#endif // TABBAR_H
