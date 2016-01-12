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

QString Settings::lastActiveView() const
{
	return value("lastActiveView", "actionViewPlaylists").toString();
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

void Settings::setVolume(qreal v)
{
	setValue("volume", v);
}
