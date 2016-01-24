#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include "miamcore_global.h"
#include <QDir>
#include <QWidget>

class MIAMCORE_LIBRARY AbstractView : public QWidget
{
	Q_OBJECT
public:
	AbstractView(QWidget *parent = nullptr) : QWidget(parent) {}

	virtual bool hasFileExplorerFeature() const { return false; }

	virtual bool hasPlaylistFeature() const = 0;

	virtual bool hasOwnWindow() const = 0;

public slots:
	virtual void initFileExplorer(const QDir &) {}

	virtual void volumeSliderIncrease() {}

	virtual void volumeSliderDecrease() {}
};

#endif // ABSTRACTVIEW_H
