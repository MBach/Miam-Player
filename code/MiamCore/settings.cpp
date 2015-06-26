#include "settings.h"

#include <QDateTime>
#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTabWidget>

#include <QtDebug>

Settings* Settings::settings = nullptr;

/** Private constructor. */
Settings::Settings(const QString &organization, const QString &application)
	: QSettings(organization, application)
{

}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
Settings* Settings::instance()
{
	if (settings == nullptr) {
		settings = new Settings;
	}
	return settings;
}

QString Settings::lastActiveView() const
{
	QString l = value("lastActiveView").toString();
	if (l.isEmpty()) {
		return "actionViewPlaylists";
	} else {
		return l;
	}
}

/** Returns the actual theme name. */
QString Settings::theme() const
{
	QString theme = value("theme").toString();
	if (theme.isEmpty()) {
		return "oxygen";
	} else {
		return theme;
	}
}

/** Returns volume from the slider. */
int Settings::volume() const
{
   if (value("volume").isNull()) {
	   return 90;
   } else {
	   return value("volume").toInt();
   }
}

/// Slots

/** Sets the last view activated by the user. Used when reopening the player. */
void Settings::setLastActiveView(const QString &viewName)
{
	setValue("lastActiveView", viewName);
}

void Settings::setThemeName(const QString &theme)
{
	setValue("theme", theme.toLower());
	emit themeHasChanged();
}


void Settings::setVolume(int v)
{
	setValue("volume", v);
}
