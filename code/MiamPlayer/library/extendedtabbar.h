#ifndef EXTENDEDTABBAR_H
#define EXTENDEDTABBAR_H

#include <QTabBar>
#include <QtDebug>

class ExtendedTabBar : public QTabBar
{
	Q_OBJECT
public:
	explicit ExtendedTabBar(QWidget *parent = 0);

protected:
	QSize tabSizeHint(int) const
	{
		return QSize(rect().width() / 2, rect().height());
	}

	virtual void paintEvent(QPaintEvent *);
};

#endif // EXTENDEDTABBAR_H
