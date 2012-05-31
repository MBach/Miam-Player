#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "mediabutton.h"
#include "reflector.h"

class Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam",
			 const QString & application = "MmeMiamMiamMusicPlayer");

	/** Store the size of each font used in the app. */
	QMap<QString, QVariant> fontPointSizeMap;

	QList<QVariant> locations;

	QMap<QString, QByteArray> stylesheets;

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

	inline bool toggleSeparators() const { return value("alphabeticalSeparators").toBool(); }

	/** Adds a new path in the application. */
	void addMusicLocation(const QString &location);

	/** Removes a path in the application. */
	void removeMusicLocation(const QString &location);

	/** Returns all music locations. */
	inline QList<QVariant> musicLocations() const { return value("musicLocations").toList(); }

	inline bool withCovers() const { return value("covers").toBool(); }

	/** Returns the size of a cover. */
	int coverSize() const;

	/** Returns the size of the buffer for a cover. */
	int bufferedCoverSize() const;

	/** Returns volume from the slider. */
	inline qreal volume() const { return value("volume").toReal(); }

	bool repeatPlayBack() const { return value("repeatPlayBack").toBool(); }

	bool shufflePlayBack() const { return value("shufflePlayBack").toBool(); }

	void setShortcut(const QString &objectName, int keySequence);

	int shortcut(const QString &objectName) const;

	QMap<QString, QVariant> shortcuts() const;

	/// Custom icons in CustomizeTheme
	bool hasCustomIcon(MediaButton *) const;
	void setCustomIcon(MediaButton *, const QString &buttonName);
	const QString customIcon(MediaButton *, bool toggled = false) const;

	/// PlayBack options
	qint64 playbackSeekTime() const;
	bool playbackKeepPlaylists() const;

	/// Colors
	bool colorsAlternateBG() const;

	/// Stylesheet
	QString styleSheet(QWidget *w) const;

signals:
	void currentFontChanged();

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

	/** Sets the font of the application. */
	inline void setFont(const FontFamily &fontFamily, const QFont &font) {
		setValue(QString(fontFamily), font.family());
		emit currentFontChanged();
	}

	inline void setFontPointSize(const FontFamily &fontFamily, int i) {
		fontPointSizeMap.insert(QString(fontFamily), i);
		setValue("fontPointSizeMap", fontPointSizeMap);
		emit currentFontChanged();
	}

	inline void setToggleSeparators(bool b) { setValue("alphabeticalSeparators", b); }

	inline void setCovers(bool b) { setValue("covers", b); }

	/** Sets the size of a cover. */
	inline void setCoverSize(int i) { setValue("coverSize", i); }

	/** Sets the size of the buffer for a cover. */
	inline void setBufferedCoverSize(int i) { setValue("bufferedCoverSize", i); }

	/** Sets volume from the slider. */
	inline void setVolume(qreal v) { setValue("volume", v); }

	inline void setRepeatPlayBack(bool b) { setValue("repeatPlayBack", b); }

	inline void setShufflePlayBack(bool b) { setValue("shufflePlayBack", b); }

	/// PlayBack options
	inline void setPlaybackSeekTime(int t) { setValue("playbackSeekTime", t*1000); }
	inline void setPlaybackKeepPlaylists(bool b) { setValue("playbackKeepPlaylists", b); }

	/// Colors
	inline void setColorsAlternateBG(bool b) { setValue("colorsAlternateBG", b); }

	/// StyleSheets
	void setCustomStyleSheet(QWidget *w);
};

#endif // SETTINGS_H
