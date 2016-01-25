#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include "miamcore_global.h"
#include <settingsprivate.h>

#include <QDir>
#include <QWidget>

class MIAMCORE_LIBRARY AbstractView : public QWidget
{
	Q_OBJECT
public:
	AbstractView(QWidget *parent = nullptr) : QWidget(parent) {}

	virtual bool viewProperty(SettingsPrivate::ViewProperty) const { return false; }

public slots:
	virtual void initFileExplorer(const QDir &) {}

	virtual void setViewProperty(SettingsPrivate::ViewProperty vp, QVariant value) = 0;

	virtual void volumeSliderIncrease() {}

	virtual void volumeSliderDecrease() {}
};

#endif // ABSTRACTVIEW_H
