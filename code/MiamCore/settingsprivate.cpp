#include "settingsprivate.h"

#include <QDateTime>
#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QTabWidget>

#include <QtDebug>

SettingsPrivate* SettingsPrivate::settings = NULL;

/** Private constructor. */
SettingsPrivate::SettingsPrivate(const QString &organization, const QString &application)
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
		setValue("defaultPalette", p);
	}
}

/** Singleton pattern to be able to easily use SettingsPrivate everywhere in the app. */
SettingsPrivate* SettingsPrivate::instance()
{
	if (settings == NULL) {
		settings = new SettingsPrivate;
	}
	return settings;
}

qreal SettingsPrivate::bigCoverOpacity() const
{
	QVariant vOpacity = value("bigCoverOpacity");
	if (vOpacity.isNull()) {
		return 0.66;
	} else {
		return vOpacity.toReal();
	}
}

/** Return the actual size of media buttons. */
int SettingsPrivate::buttonsSize() const
{
	int s = value("buttonsSize").toInt();
	if (s == 0) {
		return 36;
	} else {
		return s;
	}
}

/** Returns true if buttons are displayed without any border. */
bool SettingsPrivate::buttonsFlat() const
{
	QVariant ok = value("buttonsFlat");
	if (ok.isValid()) {
		return ok.toBool();
	} else {
		return true;
	}
}

/** Returns true if the background color in playlist is using alternatative colors. */
bool SettingsPrivate::colorsAlternateBG() const
{
	QVariant b = value("colorsAlternateBG");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

bool SettingsPrivate::copyTracksFromPlaylist() const
{
	return value("copyTracksFromPlaylist").toBool();
}

/** Returns the size of a cover. */
int SettingsPrivate::coverSize() const
{
	int size = value("coverSize").toInt();
	if (size == 0) {
		size = 48;
	}
	return size;
}

QColor SettingsPrivate::customColors(QPalette::ColorRole cr) const
{
	QMap<QString, QVariant> customCo = value("customColorsMap").toMap();
	QColor color = customCo.value(QString(cr)).value<QColor>();
	if (color.isValid()) {
		return color;
	} else {
		return QApplication::palette().color(cr);
	}
}

const QString SettingsPrivate::customIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).toString();
}

SettingsPrivate::DragDropAction SettingsPrivate::dragDropAction() const
{
	return static_cast<SettingsPrivate::DragDropAction>(value("dragDropAction").toInt());
}

/** Returns the font of the application. */
QFont SettingsPrivate::font(const FontFamily fontFamily)
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
int SettingsPrivate::fontSize(const FontFamily fontFamily)
{
	fontPointSizeMap = this->value("fontPointSizeMap").toMap();
	int pointSize = fontPointSizeMap.value(QString(fontFamily)).toInt();
	if (pointSize == 0) {
		pointSize = 12;
	}
	return pointSize;
}

bool SettingsPrivate::hasCustomIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).isValid() && value("customIcons/" + buttonName).toBool();
}

SettingsPrivate::InsertPolicy SettingsPrivate::insertPolicy() const
{
	if (value("insertPolicy").isNull()) {
		return SettingsPrivate::IP_Artists;
	} else {
		int i = value("insertPolicy").toInt();
		return (SettingsPrivate::InsertPolicy)i;
	}
}

/** Returns true if big and faded covers are displayed in the library when an album is expanded. */
bool SettingsPrivate::isBigCoverEnabled() const
{
	if (value("bigCovers").isNull()) {
		return false;
	} else {
		return value("bigCovers").toBool();
	}
}

/** Returns true if covers are displayed in the library. */
bool SettingsPrivate::isCoversEnabled() const
{
	if (value("covers").isNull()) {
		return true;
	} else {
		return value("covers").toBool();
	}
}

bool SettingsPrivate::isCustomColors() const
{
	QVariant b = value("customColors");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

bool SettingsPrivate::isExtendedSearchVisible() const
{
	QVariant b = value("extendedSearchVisible");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if background process is active to keep library up-to-date. */
bool SettingsPrivate::isFileSystemMonitored() const
{
	QVariant b = value("monitorFileSystem");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns the hierarchical order of the library tree view. */
bool SettingsPrivate::isLibraryFilteredByArticles() const
{
	QVariant b = value("isLibraryFilteredByArticles");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

/** Returns true if the button in parameter is visible or not. */
bool SettingsPrivate::isMediaButtonVisible(const QString & buttonName) const
{
   QVariant ok = value(buttonName);
   if (ok.isValid()) {
	   return ok.toBool();
   } else {
	   // For the first run, show buttons anyway
	   return (QString::compare(buttonName, "pauseButton") != 0);
   }
}

bool SettingsPrivate::isPlaylistResizeColumns() const
{
	QVariant b = value("playlistResizeColumns");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if tabs should be displayed like rectangles. */
bool SettingsPrivate::isRectTabs() const
{
	QVariant b = value("rectangularTabs");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

bool SettingsPrivate::isSearchAndExcludeLibrary() const
{
	QVariant b = value("searchAndExcludeLibrary");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Returns true if stars are visible and active. */
bool SettingsPrivate::isStarDelegates() const
{
	return value("delegates").toBool();
}

/** Returns true if a user has modified one of defaults theme. */
bool SettingsPrivate::isThemeCustomized() const
{
	return value("themeCustomized").toBool();
}

/** Returns true if the volume value in percent is always visible in the upper left corner of the widget. */
bool SettingsPrivate::isVolumeBarTextAlwaysVisible() const
{
	return value("volumeBarTextAlwaysVisible").toBool();
}
/** Returns the language of the application. */
QString SettingsPrivate::language()
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

/** Returns the last active playlist header state. */
QByteArray SettingsPrivate::lastActivePlaylistGeometry() const
{
	return value("lastActivePlaylistGeometry").toByteArray();
}

QString SettingsPrivate::lastActiveView() const
{
	QString l = value("lastActiveView").toString();
	if (l.isEmpty()) {
		return "library";
	} else {
		return l;
	}
}

QStringList SettingsPrivate::libraryFilteredByArticles() const
{
	QVariant vArticles = value("libraryFilteredByArticles");
	if (vArticles.isValid()) {
		return vArticles.toStringList();
	} else {
		return QStringList();
	}
}

QStringList SettingsPrivate::musicLocations() const
{
	QStringList list;
	list.append(value("musicLocations").toStringList());
	return list;
}

int SettingsPrivate::tabsOverlappingLength() const
{
	QVariant v = value("tabsOverlappingLength");
	if (v.isValid()) {
		return v.toInt();
	} else {
		return 10;
	}
}

/// PlayBack options
qint64 SettingsPrivate::playbackSeekTime() const
{
	qint64 t = value("playbackSeekTime").toLongLong();
	if (t == 0) {
		return 5000;
	} else {
		return t;
	}
}

SettingsPrivate::PlaylistDefaultAction SettingsPrivate::playbackDefaultActionForClose() const
{
	return static_cast<SettingsPrivate::PlaylistDefaultAction>(value("playbackDefaultActionForClose").toInt());
}

bool SettingsPrivate::playbackKeepPlaylists() const
{
	QVariant b = value("playbackKeepPlaylists");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

bool SettingsPrivate::playbackRestorePlaylistsAtStartup() const
{
	QVariant b = value("playbackRestorePlaylistsAtStartup");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

void SettingsPrivate::setCustomColorRole(QPalette::ColorRole cr, const QColor &color)
{
	QMap<QString, QVariant> colors = value("customColorsMap").toMap();
	colors.insert(QString::number(cr), color);
	QPalette palette = QGuiApplication::palette();
	palette.setColor(cr, color);

	if (cr == QPalette::Base) {

		palette.setColor(QPalette::Button, color);
		colors.insert(QString::number(QPalette::Button), color);

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

void SettingsPrivate::setCustomIcon(const QString &buttonName, const QString &iconPath)
{
	if (iconPath.isEmpty()) {
		remove("customIcons/" + buttonName);
	} else {
		setValue("customIcons/" + buttonName, iconPath);
	}
}

void SettingsPrivate::setLanguage(const QString &lang)
{
	setValue("language", lang);
}

void SettingsPrivate::setMusicLocations(const QStringList &locations)
{
	setValue("musicLocations", locations);
}

void SettingsPrivate::setShortcut(const QString &objectName, const QKeySequence &keySequence)
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	shortcuts.insert(objectName, keySequence.toString());
	setValue("shortcuts", shortcuts);
}

QKeySequence SettingsPrivate::shortcut(const QString &objectName) const
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	return QKeySequence(shortcuts.value(objectName).toString());
}

QMap<QString, QVariant> SettingsPrivate::shortcuts() const
{
	return value("shortcuts").toMap();
}

int SettingsPrivate::volumeBarHideAfter() const
{
	if (value("volumeBarHideAfter").isNull()) {
		return 1;
	} else {
		return value("volumeBarHideAfter").toInt();
	}
}

/** Define the hierarchical order of the library tree view. */
void SettingsPrivate::setInsertPolicy(SettingsPrivate::InsertPolicy ip)
{
	setValue("insertPolicy", ip);
}

/// SLOTS
void SettingsPrivate::setBigCoverOpacity(int v)
{
	setValue("bigCoverOpacity", (qreal)(v / 100.0));
}

void SettingsPrivate::setBigCovers(bool b)
{
	setValue("bigCovers", b);
}

/** Sets a new button size. */
void SettingsPrivate::setButtonsSize(const int &s)
{
	setValue("buttonsSize", s);
}

void SettingsPrivate::setButtonsFlat(bool b)
{
	setValue("buttonsFlat", b);
}

/// Colors
void SettingsPrivate::setColorsAlternateBG(bool b)
{
	setValue("colorsAlternateBG", b);
}

void SettingsPrivate::setCopyTracksFromPlaylist(bool b)
{
	setValue("copyTracksFromPlaylist", b);
}

void SettingsPrivate::setCovers(bool b)
{
	setValue("covers", b);
}

void SettingsPrivate::setCoverSize(int s)
{
	setValue("coverSize", s);
}

void SettingsPrivate::setCustomColors(bool b)
{
	setValue("customColors", b);
	if (b) {
		QApplication::setPalette(value("customPalette").value<QPalette>());
	} else {
		QApplication::setPalette(value("defaultPalette").value<QPalette>());
	}
}

/** Sets if stars are visible and active. */
void SettingsPrivate::setDelegates(const bool &value)
{
	setValue("delegates", value);
}

void SettingsPrivate::setDragDropAction(DragDropAction action)
{
	setValue("dragDropAction", action);
}

void SettingsPrivate::setExtendedSearchVisible(bool b)
{
	setValue("extendedSearchVisible", b);
}

void SettingsPrivate::setFont(const FontFamily &fontFamily, const QFont &font)
{
	fontFamilyMap.insert(QString(fontFamily), font.family());
	setValue("fontFamilyMap", fontFamilyMap);
	emit fontHasChanged(fontFamily, font);
}

/** Sets the font size of a part of the application. */
void SettingsPrivate::setFontPointSize(const FontFamily &fontFamily, int i)
{
	fontPointSizeMap.insert(QString(fontFamily), i);
	setValue("fontPointSizeMap", fontPointSizeMap);
	emit fontHasChanged(fontFamily, font(fontFamily));
}

void SettingsPrivate::setIsLibraryFilteredByArticles(bool b)
{
	setValue("isLibraryFilteredByArticles", b);
}

/** Save the last active playlist header state. */
void SettingsPrivate::setLastActivePlaylistGeometry(const QByteArray &ba)
{
	setValue("lastActivePlaylistGeometry", ba);
}

/** Sets the last view activated by the user. Used when reopening the player. */
void SettingsPrivate::setLastActiveView(const QString &viewName)
{
	setValue("lastActiveView", viewName);
}

void SettingsPrivate::setLibraryFilteredByArticles(const QStringList &tagList)
{
	setValue("libraryFilteredByArticles", tagList);
}

/** Sets if the button in parameter is visible or not. */
void SettingsPrivate::setMediaButtonVisible(const QString & buttonName, const bool &value)
{
	setValue(buttonName, value);
}

/** Sets if MiamPlayer should launch background process to keep library up-to-date. */
void SettingsPrivate::setMonitorFileSystem(bool b)
{
	setValue("monitorFileSystem", b);
}

/// PlayBack options
void SettingsPrivate::setPlaybackSeekTime(int t)
{
	setValue("playbackSeekTime", t*1000);
}

void SettingsPrivate::setPlaybackCloseAction(PlaylistDefaultAction action)
{
	setValue("playbackDefaultActionForClose", action);
}

void SettingsPrivate::setPlaybackKeepPlaylists(bool b)
{
	setValue("playbackKeepPlaylists", b);
}

void SettingsPrivate::setSearchAndExcludeLibrary(bool b)
{
	setValue("searchAndExcludeLibrary", b);
}

void SettingsPrivate::setPlaybackRestorePlaylistsAtStartup(bool b)
{
	setValue("playbackRestorePlaylistsAtStartup", b);
}

void SettingsPrivate::setTabsOverlappingLength(int l)
{
	setValue("tabsOverlappingLength", l);
}

void SettingsPrivate::setTabsRect(bool b)
{
	setValue("rectangularTabs", b);
}

void SettingsPrivate::setThemeCustomized(bool b)
{
	setValue("themeCustomized", b);
}

void SettingsPrivate::setVolumeBarHideAfter(int seconds)
{
	setValue("volumeBarHideAfter", seconds);
}

void SettingsPrivate::setVolumeBarTextAlwaysVisible(bool b)
{
	setValue("volumeBarTextAlwaysVisible", b);
}
