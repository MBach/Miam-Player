#include "settingsprivate.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QGuiApplication>
#include <QHeaderView>
#include <QLibraryInfo>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTabWidget>

#include <QtDebug>

SettingsPrivate* SettingsPrivate::settings = nullptr;

/** Private constructor. */
SettingsPrivate::SettingsPrivate(const QString &organization, const QString &application)
	: QSettings(IniFormat, UserScope, organization, application)
{
	QPalette p = QApplication::palette();
	_standardPalette = p;

	if (isCustomColors()) {
		QApplication::setPalette(this->customPalette());
	}
}

/** Singleton pattern to be able to easily use SettingsPrivate everywhere in the app. */
SettingsPrivate* SettingsPrivate::instance()
{
	if (settings == nullptr) {
		settings = new SettingsPrivate;
		settings->initLanguage(settings->language());
	}
	return settings;
}

/** Add an activated plugin to the application. */
void SettingsPrivate::addPlugin(const PluginInfo &plugin)
{
	QMap<QString, QVariant> map = value("plugins").toMap();
	map.insert(plugin.absFilePath(), QVariant::fromValue(plugin));
	this->setValue("plugins", map);
}

/** Disable a previously registered plugin (so it still can be listed in options). */
void SettingsPrivate::disablePlugin(const QString &absFilePath)
{
	QMap<QString, QVariant> map = value("plugins").toMap();
	PluginInfo pluginInfo = map.value(absFilePath).value<PluginInfo>();
	pluginInfo.setEnabled(false);
	map.insert(absFilePath, QVariant::fromValue(pluginInfo));
	this->setValue("plugins", map);
}

/** Returns true if the background color in playlist is using alternatative colors. */
bool SettingsPrivate::colorsAlternateBG() const
{
	return value("colorsAlternateBG", true).toBool();
}

bool SettingsPrivate::copyTracksFromPlaylist() const
{
	return value("copyTracksFromPlaylist", false).toBool();
}

const QString SettingsPrivate::customIcon(const QString &buttonName) const
{
	return value("customIcons/" + buttonName).toString();
}

QPalette SettingsPrivate::customPalette() const
{
	if (value("customPalette").isNull()) {
		return _standardPalette;
	} else {
		return value("customPalette").value<QPalette>();
	}
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

bool SettingsPrivate::isCustomColors() const
{
	return value("customColors", false).toBool();
}

bool SettingsPrivate::isCustomTextColorOverriden() const
{
	bool b = value("customTextColorOverriden", false).toBool();
	return b && isCustomColors();
}

bool SettingsPrivate::isExtendedSearchVisible() const
{
	return value("extendedSearchVisible", true).toBool();
}

/** Returns true if background process is active to keep library up-to-date. */
bool SettingsPrivate::isFileSystemMonitored() const
{
	return value("monitorFileSystem", true).toBool();
}

/** Returns the hierarchical order of the library tree view. */
bool SettingsPrivate::isLibraryFilteredByArticles() const
{
	return value("isLibraryFilteredByArticles", false).toBool();
}

bool SettingsPrivate::isPlaylistResizeColumns() const
{
	return value("playlistResizeColumns", true).toBool();
}

/** Returns true if tabs should be displayed like rectangles. */
bool SettingsPrivate::isRectTabs() const
{
	return value("rectangularTabs", false).toBool();
}

bool SettingsPrivate::isRemoteControlEnabled() const
{
	return value("remoteControl").toBool();
}

/** Returns true if the article should be displayed after artist's name. */
bool SettingsPrivate::isReorderArtistsArticle() const
{
	return value("reorderArtistsArticle", false).toBool();
}

/** Returns true if a user has modified one of defaults theme. */
bool SettingsPrivate::isButtonThemeCustomized() const
{
	return value("buttonThemeCustomized", false).toBool();
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

QByteArray SettingsPrivate::lastActiveViewGeometry(const QString &menuAction) const
{
	return value(menuAction).toByteArray();
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
	return value("tabsOverlappingLength", 10).toInt();
}

/// PlayBack options
qint64 SettingsPrivate::playbackSeekTime() const
{
	return value("playbackSeekTime", 5000).toLongLong();
}

/** Default action to execute when one is closing a playlist. */
SettingsPrivate::PlaylistDefaultAction SettingsPrivate::playbackDefaultActionForClose() const
{
	return static_cast<SettingsPrivate::PlaylistDefaultAction>(value("playbackDefaultActionForClose").toInt());
}

/** Automatically save all playlists before exit. */
bool SettingsPrivate::playbackKeepPlaylists() const
{
	return value("playbackKeepPlaylists", false).toBool();
}

/** Automatically restore all saved playlists at startup. */
bool SettingsPrivate::playbackRestorePlaylistsAtStartup() const
{
	return value("playbackRestorePlaylistsAtStartup", false).toBool();
}

QMap<QString, PluginInfo> SettingsPrivate::plugins() const
{
	QMap<QString, QVariant> list = value("plugins").toMap();
	QMapIterator<QString, QVariant> it(list);
	QMap<QString, PluginInfo> registeredPlugins;
	while (it.hasNext()) {
		it.next();
		PluginInfo pluginInfo = it.value().value<PluginInfo>();
		if (QFileInfo::exists(pluginInfo.absFilePath())) {
			registeredPlugins.insert(pluginInfo.absFilePath(), std::move(pluginInfo));
		}
	}
	return registeredPlugins;
}

uint SettingsPrivate::remoteControlPort() const
{
	return value("remoteControlPort", 5600).toUInt();
}

void SettingsPrivate::setCustomColorRole(QPalette::ColorRole cr, const QColor &color)
{
	QPalette palette = this->customPalette();
	if (cr == QPalette::Base) {

		palette.setColor(QPalette::Button, color);

		// Check if text color should be inverted when the base is too dark
		QColor text, alternateBase;
		if (color.value() < 128) {
			alternateBase = palette.base().color().lighter(110);
			QColor light = _standardPalette.light().color();
			QColor midLight = _standardPalette.midlight().color();
			QColor mid = _standardPalette.mid().color();
			QColor dark = _standardPalette.dark().color();
			QColor shadow = _standardPalette.shadow().color();

			light.setRgb(255 - light.red(), 255 - light.green(), 255 - light.blue());
			midLight.setRgb(255 - midLight.red(), 255 - midLight.green(), 255 - midLight.blue());
			mid.setRgb(255 - mid.red(), 255 - mid.green(), 255 - mid.blue());
			dark.setRgb(255 - dark.red(), 255 - dark.green(), 255 - dark.blue());
			shadow.setRgb(255 - shadow.red(), 255 - shadow.green(), 255 - shadow.blue());

			palette.setColor(QPalette::Light, light);
			palette.setColor(QPalette::Midlight, midLight);
			palette.setColor(QPalette::Mid, mid);
			palette.setColor(QPalette::Dark, dark);
			palette.setColor(QPalette::Shadow, shadow);

			text = Qt::white;
		} else {
			alternateBase = palette.base().color().darker(110);
			text = Qt::black;
		}
		palette.setColor(QPalette::AlternateBase, alternateBase);
		palette.setColor(QPalette::BrightText, text);
		palette.setColor(QPalette::ButtonText, text);
		palette.setColor(QPalette::Text, text);
		palette.setColor(QPalette::WindowText, text);

		// Automatically create a window color from the base one
		QColor windowColor = color;
		//windowColor.setAlphaF(0.5);
		if (text == Qt::white) {
			windowColor = color.lighter(130);
		} else {
			windowColor = color.darker(110);
		}
		palette.setColor(QPalette::Window, windowColor);

	} else if (cr == QPalette::Highlight) {

		if (!isCustomTextColorOverriden()) {
			QColor highlightedText;
			if (qAbs(color.value() - QColor(Qt::white).value()) < 128) {
				highlightedText = Qt::black;
			} else {
				highlightedText = Qt::white;
			}
			palette.setColor(QPalette::HighlightedText, highlightedText);
		}
	} else if (cr == QPalette::Text || cr == QPalette::HighlightedText) {
		if (!isCustomTextColorOverriden()) {
			QColor c1;
			if (cr == QPalette::Text) {
				c1 = palette.base().color();
			} else {
				c1 = palette.highlight().color();
			}
			QColor c2;
			if (qAbs(c1.value() - QColor(Qt::white).value()) < 128) {
				c2 = Qt::black;
			} else {
				c2 = Qt::white;
			}
			palette.setColor(cr, c2);
		}
	}
	/// XXX
	palette.setColor(cr, color);

	QApplication::setPalette(palette);
	this->setValue("customPalette", QVariant::fromValue<QPalette>(palette));
}

void SettingsPrivate::setCustomIcon(const QString &buttonName, const QString &iconPath)
{
	if (iconPath.isEmpty()) {
		remove("customIcons/" + buttonName);
	} else {
		setValue("customIcons/" + buttonName, iconPath);
	}
	emit customIconForMediaButtonChanged(buttonName);
}

bool SettingsPrivate::setLanguage(const QString &lang)
{
	setValue("language", lang);
	bool b = this->initLanguage(lang);
	if (b) {
		emit languageAboutToChange(lang);
	}
	return b;
}

void SettingsPrivate::setLastActiveViewGeometry(const QString &menuAction, const QByteArray &viewGeometry)
{
	setValue("lastActiveView", menuAction);
	setValue(menuAction, viewGeometry);
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
	QStringList old = value("musicLocations").toStringList();
	setValue("musicLocations", locations);
	emit musicLocationsHaveChanged(old, locations);
}

void SettingsPrivate::setRemoteControlEnabled(bool b)
{
	setValue("remoteControl", b);
	emit remoteControlChanged(b, value("remoteControlPort").toUInt());
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

int SettingsPrivate::volumeBarHideAfter() const
{
	if (value("volumeBarHideAfter").isNull()) {
		return 1;
	} else {
		return value("volumeBarHideAfter").toInt();
	}
}

bool SettingsPrivate::initLanguage(const QString &lang)
{
	QString language(":/translations/player_" + lang + ".qm");
	bool b = QFile::exists(language);
	if (b) {
		bool b = playerTranslator.load(language);
		defaultQtTranslator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		/// TODO: reload plugin UI
		b &= QApplication::installTranslator(&playerTranslator);
		QApplication::installTranslator(&defaultQtTranslator);
	}
	return b;
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

/** Add a list of folders to settings. */
void SettingsPrivate::addMusicLocations(const QList<QDir> &dirs)
{
	QStringList old = value("musicLocations").toStringList();
	QStringList locations;
	for (QDir d : dirs) {
		if (!old.contains(QDir::toNativeSeparators(d.absolutePath()))) {
			locations << QDir::toNativeSeparators(d.absolutePath());
		} else {
			qDebug() << Q_FUNC_INFO << old << "already contains" << QDir::toNativeSeparators(d.absolutePath());
		}
	}
	QStringList newLocations(old);
	newLocations.append(locations);
	if (old.toSet() != newLocations.toSet()) {
		setValue("musicLocations", newLocations);
		emit musicLocationsHaveChanged(old, locations);
	}
}

/** Sets an alternate background color for playlists. */
void SettingsPrivate::setColorsAlternateBG(bool b)
{
	setValue("colorsAlternateBG", b);
}

/** Copy or move tracks from one playlist to another. */
void SettingsPrivate::setCopyTracksFromPlaylist(bool b)
{
	setValue("copyTracksFromPlaylist", b);
}

/** Sets custom colors for the whole application. */
void SettingsPrivate::setCustomColors(bool b)
{
	setValue("customColors", b);
	if (!b) {
		QApplication::setPalette(_standardPalette);
	}
}

/** Sets custom text color instead of classic black or white. */
void SettingsPrivate::setCustomTextColorOverride(bool b)
{
	setValue("customTextColorOverriden", b);
	if (!b) {
		this->setCustomColorRole(QPalette::Text, _standardPalette.color(QPalette::Text));
		this->setCustomColorRole(QPalette::HighlightedText, _standardPalette.color(QPalette::HighlightedText));
	}
}

/** Sets the default action when one is dropping tracks or folders. */
void SettingsPrivate::setDragDropAction(DragDropAction action)
{
	setValue("dragDropAction", action);
}

/** Sets a popup when one is searching text in Library (Playlist mode only). */
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

/** Sets user defined articles (like 'The', 'Le') to sort the Library. */
void SettingsPrivate::setIsLibraryFilteredByArticles(bool b)
{
	setValue("isLibraryFilteredByArticles", b);
}

/** Save the last active playlist header state. */
void SettingsPrivate::setLastActivePlaylistGeometry(const QByteArray &ba)
{
	setValue("lastActivePlaylistGeometry", ba);
}

/** Sets user defined list of articles to sort the Library. */
void SettingsPrivate::setLibraryFilteredByArticles(const QStringList &tagList)
{
	if (tagList.isEmpty()) {
		remove("libraryFilteredByArticles");
	} else {
		setValue("libraryFilteredByArticles", tagList);
	}
}

/** Sets if MiamPlayer should launch background process to keep library up-to-date. */
void SettingsPrivate::setMonitorFileSystem(bool b)
{
	setValue("monitorFileSystem", b);
	emit monitorFileSystemChanged(b);
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
	emit librarySearchModeHasChanged();
}

void SettingsPrivate::setPlaybackRestorePlaylistsAtStartup(bool b)
{
	setValue("playbackRestorePlaylistsAtStartup", b);
}

void SettingsPrivate::setRemoteControlPort(uint port)
{
	setValue("remoteControlPort", port);
	emit remoteControlChanged(true, port);
}

void SettingsPrivate::setTabsOverlappingLength(int l)
{
	setValue("tabsOverlappingLength", l);
}

void SettingsPrivate::setTabsRect(bool b)
{
	setValue("rectangularTabs", b);
}

void SettingsPrivate::setButtonThemeCustomized(bool b)
{
	setValue("buttonThemeCustomized", b);
}

void SettingsPrivate::setVolumeBarHideAfter(int seconds)
{
	setValue("volumeBarHideAfter", seconds);
}
