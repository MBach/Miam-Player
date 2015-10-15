#ifndef LIBRARYSCROLLBAR_H
#define LIBRARYSCROLLBAR_H

#include "scrollbar.h"
#include "miamlibrary_global.hpp"

/**
 * \brief		The LibraryScrollBar class is used to hide covers when scrolling.
 * \details     When covers are enabled and scroolling onto a large library, it can produce lags. It happens because covers are
 *				lazily loaded so accessing to the hard drive needs to be done. In order to improve user experience, it's better
 *				to temporarily disable these covers when one is using this vertical scroll bar.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryScrollBar : public ScrollBar
{
	Q_OBJECT

private:
	/** Keep a track of cover state between mouse events. */
	bool _hasNotEmittedYet;

	QTimer *_timer;

public:
	explicit LibraryScrollBar(QWidget *parent);

protected:
	/** Redefined to temporarily hide covers when moving. */
	virtual void mouseMoveEvent(QMouseEvent *e) override;

	/** Redefined to temporarily hide covers when moving. */
	virtual void mousePressEvent(QMouseEvent *e) override;

	/** Redefined to restore covers when move events are finished. */
	virtual void mouseReleaseEvent(QMouseEvent *e) override;

signals:
	/** Tell the view to toggle covers. */
	void aboutToDisplayItemDelegate(bool);
};

#endif // LIBRARYSCROLLBAR_H
