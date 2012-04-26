#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "mediabutton.h"

class Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam", const QString & application = "MmeMiamMiamMusicPlayer")
		: QSettings(organization, application) {}

	/** Store the size of each font used in the app. */
	QMap<QString, QVariant> fontPointSizeMap;

	QList<QVariant> locations;

	Q_ENUMS(FontFamily)

public:
	enum FontFamily{PLAYLIST, LIBRARY, MENUS};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* getInstance();

	/** Returns the actual theme name. */
	QString theme() const;

	/** Returns the actual size of media buttons. */
	int buttonSize() const;

	/** Returns true if the button in parameter is visible or not. */
	bool isVisible(MediaButton *b) const;

	/** Returns true if stars are visible and active. */
	inline bool isStarDelegates() const { return value("delegates").toBool(); }

	/** Sets the language of the application. */
	inline void setLanguage(QString lang) { setValue("language", lang); }

	/** Returns the language of the application. */
	QString language();

	/** Returns the font of the application. */
	QFont font(FontFamily fontFamily);

	/** Sets the font of the application. */
	const int fontSize(FontFamily fontFamily);

	inline bool toggleSeparators() const { return value("alphabeticalSeparators").toBool(); }

	/** Adds a new path in the application. */
	void addMusicLocation(QString location);

	/** Removes a path in the application. */
	void removeMusicLocation(QString location);

	/** Returns all music locations. */
	inline QList<QVariant> musicLocations() { return value("musicLocations").toList(); }

	inline bool withCovers() const { return value("covers").toBool(); }

	/** Returns the size of a cover. */
	int coverSize() const;

	/** Returns the size of the buffer for a cover. */
	int bufferedCoverSize() const;

	/** Returns volume from the slider. */
	inline qreal volume() { return value("volume").toReal(); }

	bool repeatPlayBack() { return value("repeatPlayBack").toBool(); }

	void setShortcut(const QString &objectName, int keySequence);

	int shortcut(const QString &objectName) const;

	QMap<QString, QVariant> shortcuts() const;

signals:
	void currentFontChanged();
	
public slots:
	/** Sets a new theme. */
	inline void setThemeName(const QString &theme) { setValue("theme", theme.toLower()); }

	/** Sets a new button size. */
	inline void setButtonSize(const int &s) { setValue("buttonSize", s); }

	/** Sets if the button in parameter is visible or not. */
	void setVisible(MediaButton *b, const bool &value);

	/** Sets if stars are visible and active. */
	inline void setDelegates(const bool &value) { setValue("delegates", value); }

	/** Sets the font of the application. */
	inline void setFont(FontFamily fontFamily, const QFont &font) {
		setValue(QString(fontFamily), font.family());
		emit currentFontChanged();
	}

	inline void setFontPointSize(FontFamily fontFamily, int i) {
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
	
};

#endif // SETTINGS_H
