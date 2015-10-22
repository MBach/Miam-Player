#ifndef STARSWIDGET_H
#define STARSWIDGET_H

#include "starrating.h"
#include <QWidget>

class StarsWidget : public QWidget
{
	Q_OBJECT
private:
	StarRating _starRating;

public:
	explicit StarsWidget(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // STARSWIDGET_H
