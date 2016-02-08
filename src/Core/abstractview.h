#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include "miamcore_global.h"
#include <settingsprivate.h>
//#include <model/sqldatabase.h>
#include <musicsearchengine.h>

#include <QDir>
#include <QWidget>

/**
 * \brief		The AbstractView class is the base class for all views in Miam-Player.
 * \details		Every view in the player should inherit from this hierarchy in order to provide minimal functionalities
 *				to have a ready-to-work player. Usually, a view in a media player has a seekbar, a volume slider and some specific areas.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY AbstractView : public QWidget
{
	Q_OBJECT
public:
	AbstractView(QWidget *parent = nullptr) : QWidget(parent) {}

	virtual ~AbstractView() {}

	virtual QPair<QString, QObjectList> extensionPoints() const { return qMakePair(QString(), QObjectList()); }

	virtual void setMusicSearchEngine(MusicSearchEngine *) {}

	virtual bool viewProperty(SettingsPrivate::ViewProperty) const { return false; }

public slots:
	virtual void initFileExplorer(const QDir &) {}

	virtual void setViewProperty(SettingsPrivate::ViewProperty vp, QVariant value) = 0;

	virtual void volumeSliderIncrease() {}

	virtual void volumeSliderDecrease() {}
};

#endif // ABSTRACTVIEW_H
