#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "miamcore_global.h"

/**
 * \brief		Settings class contains all relevant pairs of (keys, values) used by Miam-Player.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam",
			 const QString &application = "MiamPlayer");

	Q_ENUMS(RequestSqlModel)

public:
	enum RequestSqlModel { RSM_Hierarchical = 1,
						   RSM_Flat			= 2};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* instance();

	/** Returns true if the button in parameter is visible or not. */
	bool isMediaButtonVisible(const QString & buttonName) const;

	/** Sets if the button in parameter is visible or not. */
	void setMediaButtonVisible(const QString & buttonName, const bool &value);

	QMap<QString, QVariant> shortcuts() const;

	RequestSqlModel sqlModel() const;

	/** Returns the actual theme name. */
	QString theme() const;

	/** Returns volume from the slider. */
	qreal volume() const;

private:
	void initShortcuts();

public slots:
	/** Sets a new theme. */
	void setThemeName(const QString &theme);

	/** Sets volume from the slider. */
	void setVolume(qreal v);

signals:
	void mediaButtonVisibilityChanged(const QString &buttonName, bool value);

	void themeHasChanged(const QString &theme);
};

#endif // SETTINGS_H
