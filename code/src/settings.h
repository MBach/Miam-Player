#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "mediabutton.h"
#include "dialogs/reflector.h"

class Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam",
			 const QString &application = "MmeMiamMiamMusicPlayer");

	/** Store the size of each font used in the app. */
	QMap<QString, QVariant> fontPointSizeMap;

	/** Store the family of each font used in the app. */
	QMap<QString, QVariant> fontFamilyMap;

	QList<QVariant> locations;

	QMap<QString, QByteArray> stylesheets;

	QMap<QString, QVariant> columnStates;

	Q_ENUMS(FontFamily)

public:
	enum FontFamily{PLAYLIST, LIBRARY, MENUS};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* getInstance();

	/** Returns the actual theme name. */
	QString theme() const;

	/// Buttons
	/** Returns the actual size of media buttons. */
	int buttonsSize() const;
	bool buttonsFlat() const;

	/** Returns true if the button in parameter is visible or not. */
	bool isVisible(MediaButton *b) const;

	/** Returns true if stars are visible and active. */
	inline bool isStarDelegates() const { return value("delegates").toBool(); }

	/** Sets the language of the application. */
	inline void setLanguage(const QString &lang) { setValue("language", lang); }

	/** Returns the language of the application. */
	QString language();

	/** Returns the font of the application. */
	QFont font(const FontFamily fontFamily);

	/** Sets the font of the application. */
	int fontSize(const FontFamily fontFamily);

	void setMusicLocations(const QStringList &locations);

	/** Returns all music locations. */
	QStringList musicLocations() const;

	bool withCovers() const;

	/** Returns the size of a cover. */
	int coverSize() const;

	/** Returns the size of the buffer for a cover. */
	int bufferedCoverSize() const;

	/** Returns volume from the slider. */
	int volume() const;

	void setShortcut(const QString &objectName, int keySequence);

	int shortcut(const QString &objectName) const;

	QMap<QString, QVariant> shortcuts() const;

	/// Custom icons in CustomizeTheme
	bool hasCustomIcon(QPushButton *) const;
	void setCustomIcon(QPushButton *, const QString &buttonName);
	const QString customIcon(QPushButton *, bool toggled = false) const;

	/// PlayBack options
	qint64 playbackSeekTime() const;
	bool playbackKeepPlaylists() const;

	/// Colors
	bool colorsAlternateBG() const;
	bool customColors() const;

	/// Stylesheet
	QString styleSheet(QWidget *w) const;

	bool copyTracksFromPlaylist() const { return value("copyTracksFromPlaylist").toBool(); }

	QString dragAndDropBehaviour() const { return value("dragAndDropBehaviour").toString(); }

	void saveColumnStateForPlaylist(int playlistIndex, const QByteArray &state);

	QByteArray restoreColumnStateForPlaylist(int playlistIndex) const;

public slots:
	/** Sets a new theme. */
	inline void setThemeName(const QString &theme) { setValue("theme", theme.toLower()); }

	/** Sets a new button size. */
	inline void setButtonsSize(const int &s) { setValue("buttonsSize", s); }
	inline void setButtonsFlat(bool b) { setValue("buttonsFlat", b); }

	/** Sets if the button in parameter is visible or not. */
	void setVisible(MediaButton *b, const bool &value);

	/** Sets if stars are visible and active. */
	inline void setDelegates(const bool &value) { setValue("delegates", value); }

	/** Sets the font of a part of the application. */
	inline void setFont(const FontFamily &fontFamily, const QFont &font) {
		fontFamilyMap.insert(QString(fontFamily), font.family());
		setValue("fontFamilyMap", fontFamilyMap);
	}

	/** Sets the font size of a part of the application. */
	inline void setFontPointSize(const FontFamily &fontFamily, int i) {
		fontPointSizeMap.insert(QString(fontFamily), i);
		setValue("fontPointSizeMap", fontPointSizeMap);
	}

	inline void setCovers(bool b) { setValue("covers", b); }

	/** Sets the size of a cover. */
	inline void setCoverSize(int i) { setValue("coverSize", i); }

	/** Sets the size of the buffer for a cover. */
	inline void setBufferedCoverSize(int i) { setValue("bufferedCoverSize", i); }

	/** Sets volume from the slider. */
	inline void setVolume(int v) { setValue("volume", v); }

	/// PlayBack options
	inline void setPlaybackSeekTime(int t) { setValue("playbackSeekTime", t*1000); }
	inline void setPlaybackKeepPlaylists(bool b) { setValue("playbackKeepPlaylists", b); }

	/// Colors
	inline void setColorsAlternateBG(bool b) { setValue("colorsAlternateBG", b); }
	inline void setCustomColors(bool b) { setValue("customColors", b); }

	/// StyleSheets
	void setCustomStyleSheet(QWidget *w);

	inline void setCopyTracksFromPlaylist(bool b) { setValue("copyTracksFromPlaylist", b); }

	inline void setDragAndDropBehaviour() { setValue("dragAndDropBehaviour", sender()->objectName()); }
};

#endif // SETTINGS_H
