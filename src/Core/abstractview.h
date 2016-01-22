#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include "miamcore_global.h"
#include <QWidget>

class MIAMCORE_LIBRARY AbstractView : public QWidget
{
	Q_OBJECT
public:
	AbstractView(QWidget *parent = nullptr) : QWidget(parent) {}

	virtual bool hasPlaylistFeature() const = 0;

public slots:
	virtual void volumeSliderIncrease() = 0;

	virtual void volumeSliderDecrease() = 0;
};

#endif // ABSTRACTVIEW_H
