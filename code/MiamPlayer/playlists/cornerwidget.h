#ifndef CORNERWIDGET_H
#define CORNERWIDGET_H

#include <QPushButton>

class CornerWidget : public QPushButton
{
	Q_OBJECT
public:
	explicit CornerWidget(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *);
};

#endif // CORNERWIDGET_H
