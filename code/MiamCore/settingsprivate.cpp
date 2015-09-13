#include "settingsprivate.h"

#include <QDateTime>
#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTabWidget>

#include <QtDebug>

SettingsPrivate* SettingsPrivate::settings = nullptr;

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
	}
}

/** Singleton pattern to be able to easily use SettingsPrivate everywhere in the app. */
SettingsPrivate* SettingsPrivate::instance()
{
	if (settings == nullptr) {
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
		qDebug() << Q_FUNC_INFO << "color is valid" << color;
		return color;
	} else {
		qDebug() << Q_FUNC_INFO << "color is NOT valid" << QApplication::palette().color(cr);

		return QApplication::palette().color(cr);
	}
}

const QString SettingsPrivate::customIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).toString();
}

QString SettingsPrivate::defaultLocationFileExplorer() const
{
	if (value("defaultLocationFileExplorer").isNull()) {
		QStringList l = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
		if (!l.isEmpty()) {
			return l.first();
		}
	} else {
		return value("defaultLocationFileExplorer").toString();
	}
	return "/";
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
			#if defined(Q_OS_WIN)
			font = QFont("Segoe UI Light");
			#elif defined(Q_OS_OSX)
			font = QFont("Helvetica Neue");
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
			#if defined(Q_OS_WIN)
			font = QFont("Segoe UI");
			#elif defined(Q_OS_OSX)
			font = QFont("Helvetica Neue");
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
		#if defined(Q_OS_OSX)
		pointSize = 16;
		#else
		pointSize = 12;
		#endif
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
		return true;
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

/** Returns true if the article should be displayed after artist's name. */
bool SettingsPrivate::isReorderArtistsArticle() const
{
	QVariant b = value("reorderArtistsArticle");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

/** Returns true if star outline must be displayed in the library. */
bool SettingsPrivate::isShowNeverScored() const
{
	QVariant b = value("showNeverScored");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

/** Returns true if stars are visible and active. */
bool SettingsPrivate::isStarDelegates() const
{
	QVariant b = value("delegates");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
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

/** Returns the last playlists that were opened when player was closed. */
QList<uint> SettingsPrivate::lastPlaylistSession() const
{
	QList<QVariant> l = value("currentSessionPlaylists").toList();
	QList<uint> playlistIds;
	for (int i = 0; i < l.count(); i++) {
		playlistIds.append(l.at(i).toUInt());
	}
	return playlistIds;
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

SettingsPrivate::LibrarySearchMode SettingsPrivate::librarySearchMode() const
{
	if (value("librarySearchMode").isNull()) {
		return SettingsPrivate::LSM_Filter;
	} else {
		int i = value("librarySearchMode").toInt();
		return (SettingsPrivate::LibrarySearchMode)i;
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

/** Default action to execute when one is closing a playlist. */
SettingsPrivate::PlaylistDefaultAction SettingsPrivate::playbackDefaultActionForClose() const
{
	return static_cast<SettingsPrivate::PlaylistDefaultAction>(value("playbackDefaultActionForClose").toInt());
}

/** Automatically save all playlists before exit. */
bool SettingsPrivate::playbackKeepPlaylists() const
{
	QVariant b = value("playbackKeepPlaylists");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return false;
	}
}

/** Automatically restore all saved playlists at startup. */
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
		//windowColor.setAlphaF(0.5);
		if (text == Qt::white) {
			windowColor = color.lighter(130);
		} else {
			windowColor = color.darker(110);
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

/** Sets the last playlists that were opened when player is about to close. */
void SettingsPrivate::setLastPlaylistSession(const QList<uint> &ids)
{
	QList<QVariant> l;
	for (int i = 0; i < ids.count(); i++) {
		l.append(ids.at(i));
	}
	setValue("currentSessionPlaylists", l);
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

void SettingsPrivate::setDefaultLocationFileExplorer(const QString &location)
{
	setValue("defaultLocationFileExplorer", location);
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

void SettingsPrivate::setLibraryFilteredByArticles(const QStringList &tagList)
{
	if (tagList.isEmpty()) {
		remove("libraryFilteredByArticles");
	} else {
		setValue("libraryFilteredByArticles", tagList);
	}
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

void SettingsPrivate::setReorderArtistsArticle(bool b)
{
	setValue("reorderArtistsArticle", b);
}

void SettingsPrivate::setSearchAndExcludeLibrary(bool b)
{
	LibrarySearchMode lsm;
	if (b) {
		lsm = LSM_Filter;
	} else {
		lsm = LSM_HighlightOnly;
	}
	setValue("librarySearchMode", lsm);
	emit librarySearchModeChanged();
}

void SettingsPrivate::setShowNeverScored(bool b)
{
	setValue("showNeverScored", b);
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
