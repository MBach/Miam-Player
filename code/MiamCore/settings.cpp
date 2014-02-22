#include "settings.h"

#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTabWidget>

#include <QtDebug>

Settings* Settings::settings = NULL;

/** Private constructor. */
Settings::Settings(const QString &organization, const QString &application)
	: QSettings(organization, application)
{}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
Settings* Settings::getInstance()
{
	if (settings == NULL) {
		settings = new Settings;
	}
	return settings;
}

/** Return the actual size of media buttons. */
int Settings::buttonsSize() const
{
	int s = value("buttonsSize").toInt();
	if (s == 0) {
		return 36;
	} else {
		return s;
	}
}

/** Returns true if buttons are displayed without any border. */
bool Settings::buttonsFlat() const
{
	QVariant ok = value("buttonsFlat");
	if (ok.isValid()) {
		return ok.toBool();
	} else {
		return true;
	}
}

/** Returns true if the background color in playlist is using alternatative colors. */
bool Settings::colorsAlternateBG() const
{
	QVariant b = value("colorsAlternateBG");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

bool Settings::copyTracksFromPlaylist() const
{
	return value("copyTracksFromPlaylist").toBool();
}

/** Returns the size of a cover. */
int Settings::coverSize() const
{
	int size = value("coverSize").toInt();
	if (size == 0) {
		size = 48;
	}
	return size;
}

QColor Settings::customColors(CustomColors cc) const
{
	QMap<QString, QVariant> customCo = value("customColorsMap").toMap();
	return customCo.value(QString(cc)).value<QColor>();
}

const QString Settings::customIcon(QPushButton *b, bool toggled) const
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	if (toggled) {
		return customIcons.value("pauseButton").toString();
	} else {
		return customIcons.value(b->objectName()).toString();
	}
}

const QString Settings::dragAndDropBehaviour() const
{
	return value("dragAndDropBehaviour").toString();
}

/** Returns the font of the application. */
QFont Settings::font(const FontFamily fontFamily)
{
	fontFamilyMap = this->value("fontFamilyMap").toMap();
	QFont font;
	QVariant vFont;
	switch(fontFamily) {
	case LIBRARY:
		vFont = fontFamilyMap.value(QString(fontFamily));
		if (vFont.isNull()) {
			#ifdef Q_OS_WIN
			font = QFont("Segoe UI Light");
			#else
			font = QGuiApplication::font();
			#endif
		} else {
			font = QFont(vFont.toString());
		}
		break;
	case MENUS:
	case PLAYLIST:
		vFont = fontFamilyMap.value(QString(fontFamily));
		if (vFont.isNull()) {
			#ifdef Q_OS_WIN
			font = QFont("Segoe UI");
			#else
			font = QGuiApplication::font();
			#endif
		} else {
			font = QFont(vFont.toString());
		}
	}
	font.setPointSize(this->fontSize(fontFamily));
	return font;
}

/** Sets the font of the application. */
int Settings::fontSize(const FontFamily fontFamily)
{
	fontPointSizeMap = this->value("fontPointSizeMap").toMap();
	int pointSize = fontPointSizeMap.value(QString(fontFamily)).toInt();
	if (pointSize == 0) {
		pointSize = 12;
	}
	return pointSize;
}

bool Settings::hasCustomIcon(QPushButton *b) const
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	return customIcons.value(b->objectName()).toBool();
}

/** Returns true if covers are displayed in the library. */
bool Settings::isCoversEnabled() const
{
	if (value("covers").isNull()) {
		return true;
	} else {
		return value("covers").toBool();
	}
}

bool Settings::isCustomColors() const
{
	QVariant b = value("customColors");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if the button in parameter is visible or not. */
bool Settings::isMediaButtonVisible(const QString & buttonName) const
{
   QVariant ok = value(buttonName);
   if (ok.isValid()) {
	   return ok.toBool();
   } else {
	   // For the first run, show buttons anyway
	   return (QString::compare(buttonName, "pauseButton") != 0);
   }
}

bool Settings::isStarDelegates() const
{
	return value("delegates").toBool();
}

/** Returns the language of the application. */
QString Settings::language()
{
	QString l = value("language").toString();
	if (l.isEmpty()) {
		l = QLocale::system().uiLanguages().first().left(2);
		setValue("language", l);
		return l;
	} else {
		return l;
	}
}

QStringList Settings::musicLocations() const
{
	QStringList list;
	foreach (QVariant v, value("musicLocations").toList()) {
		list.append(v.toString());
	}
	return list;
}

/// PlayBack options
qint64 Settings::playbackSeekTime() const
{
	qint64 t = value("playbackSeekTime").toLongLong();
	if (t == 0) {
		return 5000;
	} else {
		return t;
	}
}

Settings::PlaylistDefaultAction Settings::playbackDefaultActionForClose() const
{
	return static_cast<Settings::PlaylistDefaultAction>(value("playbackDefaultActionForClose").toInt());
}

bool Settings::playbackKeepPlaylists() const
{
	QVariant b = value("playbackKeepPlaylists");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

bool Settings::playbackRestorePlaylistsAtStartup() const
{
	QVariant b = value("playbackRestorePlaylistsAtStartup");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

QByteArray Settings::restoreColumnStateForPlaylist(int playlistIndex) const
{
	return this->value("columnStateForPlaylist").toMap().value(QString::number(playlistIndex)).toByteArray();
}

void Settings::saveColumnStateForPlaylist(int playlistIndex, const QByteArray &state)
{
	columnStates = this->value("columnStateForPlaylist").toMap();
	columnStates.insert(QString::number(playlistIndex), state);
	this->setValue("columnStateForPlaylist", columnStates);
}

void Settings::setCustomColors(CustomColors cc, const QColor &color)
{
	QMap<QString, QVariant> colors = value("customColorsMap").toMap();
	colors.insert(QString(cc), color);
	setValue("customColorsMap", colors);
}

void Settings::setCustomIcon(QPushButton *b, const QString &iconPath)
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	if (iconPath.isEmpty()) {
		customIcons.remove(b->objectName());
	} else {
		customIcons.insert(b->objectName(), iconPath);
	}
	setValue("customIcons", customIcons);
}

void Settings::setLanguage(const QString &lang)
{
	setValue("language", lang);
}

void Settings::setMusicLocations(const QStringList &locations)
{
	setValue("musicLocations", locations);
}

void Settings::setShortcut(const QString &objectName, int keySequence)
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	if (keySequence == 0) {
		shortcuts.remove(objectName);
	} else {
		shortcuts.insert(objectName, keySequence);
	}
	if (shortcuts.isEmpty()) {
		remove("shortcuts");
	} else {
		setValue("shortcuts", shortcuts);
	}
}

int Settings::shortcut(const QString &objectName) const
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	return shortcuts.value(objectName).toInt();
}

QMap<QString, QVariant> Settings::shortcuts() const
{
	return value("shortcuts").toMap();
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

/// SLOTS

/** Sets a new button size. */
void Settings::setButtonsSize(const int &s)
{
	setValue("buttonsSize", s);
}

void Settings::setButtonsFlat(bool b)
{
	setValue("buttonsFlat", b);
}

/// Colors
void Settings::setColorsAlternateBG(bool b)
{
	setValue("colorsAlternateBG", b);
}

void Settings::setCopyTracksFromPlaylist(bool b)
{
	setValue("copyTracksFromPlaylist", b);
}

void Settings::setCovers(bool b)
{
	setValue("covers", b);
}

void Settings::setCoverSize(int s)
{
	setValue("coverSize", s);
}

void Settings::setCustomColors(bool b)
{
	setValue("customColors", b);
}

void Settings::setCustomStyleSheet(QWidget *w)
{
	QMap<QString, QVariant> map = value("styleSheet").toMap();
	map.insert(w->metaObject()->className(), w->styleSheet());
	this->setValue("styleSheet", map);
}

/** Sets if stars are visible and active. */
void Settings::setDelegates(const bool &value)
{
	setValue("delegates", value);
}

void Settings::setDragAndDropBehaviour()
{
	setValue("dragAndDropBehaviour", sender()->objectName());
}

void Settings::setFont(const FontFamily &fontFamily, const QFont &font)
{
	fontFamilyMap.insert(QString(fontFamily), font.family());
	setValue("fontFamilyMap", fontFamilyMap);
}

/** Sets the font size of a part of the application. */
void Settings::setFontPointSize(const FontFamily &fontFamily, int i)
{
	fontPointSizeMap.insert(QString(fontFamily), i);
	setValue("fontPointSizeMap", fontPointSizeMap);
}

/** Sets if the button in parameter is visible or not. */
void Settings::setMediaButtonVisible(const QString & buttonName, const bool &value)
{
	this->setValue(buttonName, value);
}

/// PlayBack options
void Settings::setPlaybackSeekTime(int t)
{
	setValue("playbackSeekTime", t*1000);
}

void Settings::setPlaybackDefaultActionForClose(PlaylistDefaultAction action)
{
	setValue("playbackDefaultActionForClose", action);
}

void Settings::setPlaybackKeepPlaylists(bool b)
{
	setValue("playbackKeepPlaylists", b);
}

void Settings::setPlaybackRestorePlaylistsAtStartup(bool b)
{
	setValue("playbackRestorePlaylistsAtStartup", b);
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
