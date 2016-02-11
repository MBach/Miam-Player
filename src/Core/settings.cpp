#include "settings.h"

#include <QAction>
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
	: QSettings(IniFormat, UserScope, organization, application)
{}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
Settings* Settings::instance()
{
	if (settings == nullptr) {
		settings = new Settings;
		settings->initShortcuts();
	}
	return settings;
}

/** Return the actual size of media buttons. */
int Settings::buttonsSize() const
{
	return value("buttonsSize", 36).toInt();
}

/** Returns true if the button in parameter is visible or not. */
bool Settings::isMediaButtonVisible(const QString & buttonName) const
{
   QVariant ok = value(buttonName);
   if (ok.isValid()) {
	   return ok.toBool();
   } else {
	   // For the first run, show buttons anyway except seek back|forward buttons
	   if (buttonName == "seekBackwardButton" || buttonName == "seekForwardButton") {
		   return false;
	   }
	   return (QString::compare(buttonName, "pauseButton") != 0);
   }
}

/** Sets if the button in parameter is visible or not. */
void Settings::setMediaButtonVisible(const QString & buttonName, const bool &value)
{
	setValue(buttonName, value);
	emit mediaButtonVisibilityChanged(buttonName, value);
}

QMap<QString, QVariant> Settings::shortcuts() const
{
	return value("shortcuts").toMap();
}

Settings::RequestSqlModel Settings::sqlModel() const
{
	if (value("requestSqlModel").isNull()) {
		return RSM_Hierarchical;
	} else {
		int i = value("requestSqlModel").toInt();
		return (Settings::RequestSqlModel)i;
	}
}

/** Returns the actual theme name. */
QString Settings::theme() const
{
	return value("theme", "oxygen").toString();
}

/** Returns volume from the slider. */
qreal Settings::volume() const
{
	return value("volume", 0.9).toReal();
}

void Settings::initShortcuts()
{
	if (value("shortcuts").isNull()) {
		QMap<QString, QVariant> shortcuts;
		shortcuts.insert("openFiles", "Ctrl+O");
		shortcuts.insert("openFolders", "Ctrl+Shift+O");
		shortcuts.insert("showCustomize", "F9");
		shortcuts.insert("showOptions", "F12");
		shortcuts.insert("exit", "Ctrl+Q");
		shortcuts.insert("scanLibrary", "Ctrl+Shift+Q");
		shortcuts.insert("showHelp", "F1");
		shortcuts.insert("showDebug", "F2");
		shortcuts.insert("viewPlaylists", "H");
		shortcuts.insert("showTabLibrary", "1");
		shortcuts.insert("showTabFilesystem", "2");
		shortcuts.insert("search", "Ctrl+F");
		shortcuts.insert("viewTagEditor", "J");
		shortcuts.insert("skipBackward", "Z");
		shortcuts.insert("seekBackward", "X");
		shortcuts.insert("play", "C");
		shortcuts.insert("stop", "V");
		shortcuts.insert("seekForward", "B");
		shortcuts.insert("skipForward", "N");
		shortcuts.insert("playbackSequential", "K");
		shortcuts.insert("playbackRandom", "L");
		shortcuts.insert("mute", "M");
		shortcuts.insert("increaseVolume", "Up");
		shortcuts.insert("decreaseVolume", "Down");
		shortcuts.insert("addPlaylist", "Ctrl+T");
		shortcuts.insert("deleteCurrentPlaylist", "Ctrl+W");
		shortcuts.insert("moveTracksUp", "Maj+Up");
		shortcuts.insert("moveTracksDown", "Maj+Down");
		shortcuts.insert("removeSelectedTracks", "Delete");
		setValue("shortcuts", shortcuts);
	}
}

void Settings::setThemeName(const QString &theme)
{
	setValue("theme", theme.toLower());
	emit themeHasChanged(theme);
}

void Settings::setVolume(qreal v)
{
	setValue("volume", v);
}

/** Returns true if the volume value in percent is always visible in the upper left corner of the widget. */
bool Settings::isVolumeBarTextAlwaysVisible() const
{
	return value("volumeBarTextAlwaysVisible", false).toBool();
}

/** Sets a new button size. */
void Settings::setButtonsSize(int s)
{
	setValue("buttonsSize", s);
	emit viewPropertyChanged(VP_MediaControls, s);
}

void Settings::setVolumeBarTextAlwaysVisible(bool b)
{
	setValue("volumeBarTextAlwaysVisible", b);
	emit viewPropertyChanged(VP_VolumeIndicatorToggled, b);
}
