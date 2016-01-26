#ifndef EXTENDEDTABBAR_H
#define EXTENDEDTABBAR_H

#include <QTabBar>
#include <QtDebug>
#include "miamtabplaylists_global.hpp"

/**
 * \brief		The ExtendedTabBar class is only used in left-pane to display 2 tabs (Library - FileSystem) where each has maximum length.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY ExtendedTabBar : public QTabBar
{
	Q_OBJECT
public:
	/** Default constructor. */
	explicit ExtendedTabBar(QWidget *parent = nullptr);

protected:
	virtual QSize tabSizeHint(int) const override;

	/** Redefined to be style-aware at runtime. */
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // EXTENDEDTABBAR_H
