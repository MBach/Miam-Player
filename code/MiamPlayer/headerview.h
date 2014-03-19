#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>

class HeaderView : public QHeaderView
{
	Q_OBJECT
public:
	explicit HeaderView(QWidget *parent = 0);

protected:
	/** Redefined. */
	virtual void paintSection(QPainter * painter, const QRect & rect, int logicalIndex) const;
};

#endif // HEADERVIEW_H
