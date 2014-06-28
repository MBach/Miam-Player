#include "settings.h"

#include <QDateTime>
#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTabWidget>

#include <QtDebug>

Settings* Settings::settings = NULL;

/** Private constructor. */
Settings::Settings(const QString &organization, const QString &application)
	: QSettings(organization, application)
{
	if (isCustomColors()) {
		QMapIterator<QString, QVariant> it(value("customColorsMap").toMap());
		QPalette p = QApplication::palette();
		while (it.hasNext()) {
			it.next();
			QColor color = it.value().value<QColor>();
			if (color.isValid()) {
				p.setColor(static_cast<QPalette::ColorRole>(it.key().toInt()), color);
			}
		}
		QApplication::setPalette(p);
		setValue("customPalette", p);
	} else {
		QPalette p = QApplication::palette();
		if ((p.highlight().color().lighter(160).value() - p.highlightedText().color().value()) < 128) {
			p.setColor(QPalette::HighlightedText, Qt::black);
		}
		setValue("defaultPalette", p);
	}
}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
Settings* Settings::getInstance()
{
	if (settings == NULL) {
		settings = new Settings;
	}
	return settings;
}

qreal Settings::bigCoverOpacity() const
{
	QVariant vOpacity = value("bigCoverOpacity");
	if (vOpacity.isNull()) {
		return 0.66;
	} else {
		return vOpacity.toReal();
	}
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

QColor Settings::customColors(QPalette::ColorRole cr) const
{
	QMap<QString, QVariant> customCo = value("customColorsMap").toMap();
	QColor color = customCo.value(QString(cr)).value<QColor>();
	if (color.isValid()) {
		return color;
	} else {
		return QApplication::palette().color(cr);
	}
}

const QString Settings::customIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).toString();
}

Settings::DragDropAction Settings::dragDropAction() const
{
	return static_cast<Settings::DragDropAction>(value("dragDropAction").toInt());
}

/** Returns the font of the application. */
QFont Settings::font(const FontFamily fontFamily)
{
	fontFamilyMap = this->value("fontFamilyMap").toMap();
	QFont font;
	QVariant vFont;
	switch(fontFamily) {
	case FF_Library:
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
	case FF_Menu:
	case FF_Playlist:
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

bool Settings::hasCustomIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).isValid() && value("customIcons/" + buttonName).toBool();
}

/** Returns true if big and faded covers are displayed in the library when an album is expanded. */
bool Settings::isBigCoverEnabled() const
{
	if (value("bigCovers").isNull()) {
		return false;
	} else {
		return value("bigCovers").toBool();
	}
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
		return false;
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

bool Settings::isPlaylistResizeColumns() const
{
	QVariant b = value("playlistResizeColumns");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if tabs should be displayed like rectangles. */
bool Settings::isRectTabs() const
{
	QVariant b = value("rectangularTabs");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

bool Settings::isSearchAndExcludeLibrary() const
{
	QVariant b = value("searchAndExcludeLibrary");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if stars are visible and active. */
bool Settings::isStarDelegates() const
{
	return value("delegates").toBool();
}

/** Returns true if a user has modified one of defaults theme. */
bool Settings::isThemeCustomized() const
{
	return value("themeCustomized").toBool();
}

/** Returns true if the volume value in percent is always visible in the upper left corner of the widget. */
bool Settings::isVolumeBarTextAlwaysVisible() const
{
	return value("volumeBarTextAlwaysVisible").toBool();
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

int Settings::tabsOverlappingLength() const
{
	QVariant v = value("tabsOverlappingLength");
	if (v.isValid()) {
		return v.toInt();
	} else {
		return 10;
	}
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

void Settings::setCustomColorRole(QPalette::ColorRole cr, const QColor &color)
{
	QMap<QString, QVariant> colors = value("customColorsMap").toMap();
	colors.insert(QString::number(cr), color);
	QPalette palette = QGuiApplication::palette();
	palette.setColor(cr, color);

	if (cr == QPalette::Base) {

		palette.setColor(QPalette::Button, color);
		colors.insert(QString::number(QPalette::Button), color);

		//qDebug() << "base color" << color.value();
		// Check if text color should be inverted when the base is too dark
		QColor text;
		if (color.value() < 128) {
			text = Qt::white;

			/*palette.setColor(QPalette::Light, color);
			palette.setColor(QPalette::Midlight, color);
			palette.setColor(QPalette::Dark, color);
			palette.setColor(QPalette::Mid, color);
			palette.setColor(QPalette::Shadow, color);

			colors.insert(QString::number(QPalette::Light), color);
			colors.insert(QString::number(QPalette::Midlight), color);
			colors.insert(QString::number(QPalette::Dark), color);
			colors.insert(QString::number(QPalette::Mid), color);
			colors.insert(QString::number(QPalette::Shadow), color);*/

		} else {
			text = Qt::black;
		}
		//qDebug() << "base" << color.value() << "windowText" << palette.windowText().color().value();
		palette.setColor(QPalette::BrightText, text);
		palette.setColor(QPalette::ButtonText, text);
		palette.setColor(QPalette::Text, text);
		palette.setColor(QPalette::WindowText, text);

		colors.insert(QString::number(QPalette::BrightText), text);
		colors.insert(QString::number(QPalette::ButtonText), text);
		colors.insert(QString::number(QPalette::Text), text);
		colors.insert(QString::number(QPalette::WindowText), text);

		// Automatically create a window color from the base one
		QColor windowColor = color;
		if (windowColor.value() > 128) {
			windowColor = windowColor.darker(115);
		} else {
			windowColor = windowColor.lighter(115);
		}
		palette.setColor(QPalette::Window, windowColor);
		colors.insert(QString::number(QPalette::Window), windowColor);
	} else if (cr == QPalette::Highlight) {
		QColor highlightedText;
		if (qAbs(color.value() - QColor(Qt::white).value()) < 128) {
			highlightedText = Qt::black;
		} else {
			highlightedText = Qt::white;
		}
		palette.setColor(QPalette::HighlightedText, highlightedText);
		colors.insert(QString::number(QPalette::HighlightedText), highlightedText);
	}

	QApplication::setPalette(palette);
	setValue("customColorsMap", colors);
}

void Settings::setCustomIcon(const QString &buttonName, const QString &iconPath)
{
	if (iconPath.isEmpty()) {
		remove("customIcons/" + buttonName);
	} else {
		setValue("customIcons/" + buttonName, iconPath);
	}
}

void Settings::setLanguage(const QString &lang)
{
	setValue("language", lang);
}

void Settings::setMusicLocations(const QStringList &locations)
{
	setValue("musicLocations", locations);
}

void Settings::setShortcut(const QString &objectName, const QKeySequence &keySequence)
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	shortcuts.insert(objectName, keySequence.toString());
	setValue("shortcuts", shortcuts);
}

QKeySequence Settings::shortcut(const QString &objectName) const
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	return QKeySequence(shortcuts.value(objectName).toString());
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

int Settings::volumeBarHideAfter() const
{
	if (value("volumeBarHideAfter").isNull()) {
		return 1;
	} else {
		return value("volumeBarHideAfter").toInt();
	}
}

/// SLOTS
void Settings::setBigCoverOpacity(int v)
{
	setValue("bigCoverOpacity", (qreal)(v / 100.0));
}

void Settings::setBigCovers(bool b)
{
	setValue("bigCovers", b);
}

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
	if (b) {
		QApplication::setPalette(value("customPalette").value<QPalette>());
	} else {
		QApplication::setPalette(value("defaultPalette").value<QPalette>());
	}
}

/** Sets if stars are visible and active. */
void Settings::setDelegates(const bool &value)
{
	setValue("delegates", value);
}

void Settings::setDragDropAction(DragDropAction action)
{
	setValue("dragDropAction", action);
}

void Settings::setFont(const FontFamily &fontFamily, const QFont &font)
{
	fontFamilyMap.insert(QString(fontFamily), font.family());
	setValue("fontFamilyMap", fontFamilyMap);
	emit fontHasChanged(fontFamily, font);
}

/** Sets the font size of a part of the application. */
void Settings::setFontPointSize(const FontFamily &fontFamily, int i)
{
	fontPointSizeMap.insert(QString(fontFamily), i);
	setValue("fontPointSizeMap", fontPointSizeMap);
	emit fontHasChanged(fontFamily, font(fontFamily));
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

void Settings::setPlaybackCloseAction(PlaylistDefaultAction action)
{
	setValue("playbackDefaultActionForClose", action);
}

void Settings::setPlaybackKeepPlaylists(bool b)
{
	setValue("playbackKeepPlaylists", b);
}

void Settings::setSearchAndExcludeLibrary(bool b)
{
	setValue("searchAndExcludeLibrary", b);
}

void Settings::setPlaybackRestorePlaylistsAtStartup(bool b)
{
	setValue("playbackRestorePlaylistsAtStartup", b);
}

void Settings::setTabsOverlappingLength(int l)
{
	setValue("tabsOverlappingLength", l);
}

void Settings::setTabsRect(bool b)
{
	setValue("rectangularTabs", b);
}

void Settings::setThemeCustomized(bool b)
{
	setValue("themeCustomized", b);
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

void Settings::setVolumeBarHideAfter(int seconds)
{
	setValue("volumeBarHideAfter", seconds);
}

void Settings::setVolumeBarTextAlwaysVisible(bool b)
{
	setValue("volumeBarTextAlwaysVisible", b);
}
