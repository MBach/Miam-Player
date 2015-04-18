#ifndef EXTENDEDTABBAR_H
#define EXTENDEDTABBAR_H

#include <QTabBar>
#include <QtDebug>

/**
 * \brief		The ExtendedTabBar class is only used in left-pane to display 2 tabs (Library - FileSystem) where each has maximum length.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ExtendedTabBar : public QTabBar
{
	Q_OBJECT
public:
	/** Default constructor. */
	explicit ExtendedTabBar(QWidget *parent = 0);

protected:
	/** Get half the size of the bounding rectangle. */
	/*inline QSize tabSizeHint(int) const
	{
		return QSize(parentWidget()->rect().width() / 2, parentWidget()->rect().height());
	}*/

	/** Redefined to be style-aware at runtime. */
	virtual void paintEvent(QPaintEvent *);
};

#endif // EXTENDEDTABBAR_H
