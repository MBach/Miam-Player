#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFileInfo>
#include <QPushButton>
#include <QSettings>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam",
			 const QString &application = "MiamPlayer");

	/** Store the size of each font used in the app. */
	QMap<QString, QVariant> fontPointSizeMap;

	/** Store the family of each font used in the app. */
	QMap<QString, QVariant> fontFamilyMap;

	QList<QVariant> locations;

	QMap<QString, QByteArray> stylesheets;

	QMap<QString, QVariant> columnStates;

	Q_ENUMS(FontFamily)
	Q_ENUMS(PlaylistDefaultAction)
	Q_ENUMS(DragDropAction)

public:
	enum FontFamily{FF_Playlist, FF_Library, FF_Menu};

	enum PlaylistDefaultAction{PL_AskUserForAction	= 0,
							   PL_SaveOnClose		= 1,
							   PL_DiscardOnClose	= 2};

	enum DragDropAction{DD_OpenPopup		= 0,
						DD_AddToLibrary		= 1,
						DD_AddToPlaylist	= 2};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* getInstance();

	qreal bigCoverOpacity() const;

	/** Returns the actual size of media buttons. */
	int buttonsSize() const;

	/** Returns true if buttons are displayed without any border. */
	bool buttonsFlat() const;

	/** Returns true if the background color in playlist is using alternatative colors. */
	bool colorsAlternateBG() const;

	bool copyTracksFromPlaylist() const;

	/** Returns the size of a cover. */
	int coverSize() const;

	QColor customColors(QPalette::ColorRole cr) const;

	/** Custom icons in CustomizeTheme */
	const QString customIcon(const QString &buttonName) const;

	DragDropAction dragDropAction() const;

	/** Returns the font of the application. */
	QFont font(const FontFamily fontFamily);

	/** Sets the font of the application. */
	int fontSize(const FontFamily fontFamily);

	/** Custom icons in CustomizeTheme */
	bool hasCustomIcon(const QString &buttonName) const;

	/** Returns true if big and faded covers are displayed in the library when an album is expanded. */
	bool isBigCoverEnabled() const;

	/** Returns true if covers are displayed in the library. */
	bool isCoversEnabled() const;

	bool isCustomColors() const;

	bool isLibraryFilteredByArticles() const;

	/** Returns true if the button in parameter is visible or not. */
	bool isMediaButtonVisible(const QString & buttonName) const;

	bool isPlaylistResizeColumns() const;

	/** Returns true if tabs should be displayed like rectangles. */
	bool isRectTabs() const;

	bool isSearchAndExcludeLibrary() const;

	/** Returns true if stars are visible and active. */
	bool isStarDelegates() const;

	/** Returns true if a user has modified one of defaults theme. */
	bool isThemeCustomized() const;

	/** Returns true if the volume value in percent is always visible in the upper left corner of the widget. */
	bool isVolumeBarTextAlwaysVisible() const;

	/** Returns the language of the application. */
	QString language();

	QStringList libraryFilteredByArticles() const;

	/** Returns all music locations. */
	QStringList musicLocations() const;

	int tabsOverlappingLength() const;

	/// PlayBack options
	qint64 playbackSeekTime() const;

	PlaylistDefaultAction playbackDefaultActionForClose() const;

	bool playbackKeepPlaylists() const;

	bool playbackRestorePlaylistsAtStartup() const;

	QByteArray restoreColumnStateForPlaylist(int playlistIndex) const;

	void saveColumnStateForPlaylist(int playlistIndex, const QByteArray &state);

	void setCustomColorRole(QPalette::ColorRole cr, const QColor &color);

	/** Custom icons in CustomizeTheme */
	void setCustomIcon(const QString &buttonName, const QString &iconPath);

	/** Sets the language of the application. */
	void setLanguage(const QString &lang);

	void setMusicLocations(const QStringList &locations);

	void setShortcut(const QString &objectName, const QKeySequence &keySequence);

	QKeySequence shortcut(const QString &objectName) const;

	QMap<QString, QVariant> shortcuts() const;

	/** Returns the actual theme name. */
	QString theme() const;

	/** Returns volume from the slider. */
	int volume() const;

	int volumeBarHideAfter() const;

public slots:
	void setBigCoverOpacity(int v);

	void setBigCovers(bool b);

	/** Sets a new button size. */
	void setButtonsSize(const int &s);
	void setButtonsFlat(bool b);

	/// Colors
	void setColorsAlternateBG(bool b);

	void setCopyTracksFromPlaylist(bool b);

	void setCovers(bool b);
	void setCoverSize(int s);

	void setCustomColors(bool b);

	/** Sets if stars are visible and active. */
	void setDelegates(const bool &value);

	void setDragDropAction(DragDropAction action);

	/** Sets the font of a part of the application. */
	void setFont(const FontFamily &fontFamily, const QFont &font);

	/** Sets the font size of a part of the application. */
	void setFontPointSize(const FontFamily &fontFamily, int i);

	void setIsLibraryFilteredByArticles(bool b);

	void setLibraryFilteredByArticles(const QStringList &tagList);

	/** Sets if the button in parameter is visible or not. */
	void setMediaButtonVisible(const QString & buttonName, const bool &value);

	/// PlayBack options
	void setPlaybackSeekTime(int t);
	void setPlaybackCloseAction(PlaylistDefaultAction action);
	void setPlaybackKeepPlaylists(bool b);
	void setPlaybackRestorePlaylistsAtStartup(bool b);

	void setSearchAndExcludeLibrary(bool b);

	void setTabsOverlappingLength(int l);

	void setTabsRect(bool b);

	void setThemeCustomized(bool b);

	/** Sets a new theme. */
	void setThemeName(const QString &theme);

	/** Sets volume from the slider. */
	void setVolume(int v);

	void setVolumeBarHideAfter(int seconds);
	void setVolumeBarTextAlwaysVisible(bool b);

signals:
	void themeHasChanged();

	void fontHasChanged(const FontFamily &fontFamily, const QFont &font);
};

Q_DECLARE_METATYPE(QPalette::ColorRole)

#endif // SETTINGS_H
