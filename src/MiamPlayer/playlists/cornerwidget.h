#ifndef CORNERWIDGET_H
#define CORNERWIDGET_H

#include <QPushButton>

#include "tabplaylist.h"

class CornerWidget : public QPushButton
{
	Q_OBJECT
public:
	explicit CornerWidget(TabPlaylist *parent);

protected:
	void mouseMoveEvent(QMouseEvent *e);

	void paintEvent(QPaintEvent *);

signals:
	void innerButtonClicked();
};

#endif // CORNERWIDGET_H
