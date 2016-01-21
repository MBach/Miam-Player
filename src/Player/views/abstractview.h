#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include <QWidget>

class AbstractView : public QWidget
{
	Q_OBJECT
public:
	AbstractView(QWidget *parent = nullptr) : QWidget(parent) {}

public slots:
	virtual void volumeSliderIncrease() = 0;

	virtual void volumeSliderDecrease() = 0;
};

#endif // ABSTRACTVIEW_H
