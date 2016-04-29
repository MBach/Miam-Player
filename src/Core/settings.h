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
	Q_ENUMS(ViewProperty)

public:
	enum RequestSqlModel { RSM_Hierarchical = 1,
						   RSM_Flat			= 2};

	enum ViewProperty { VP_MediaControls					= 0,
						VP_SearchArea						= 1,
						VP_OwnWindow						= 2,
						VP_PlaylistFeature					= 3,
						VP_FileExplorerFeature				= 4,
						VP_VolumeIndicatorToggled			= 5,
						VP_HasAreaForRescan					= 6,
						VP_HasTracksToDisplay				= 7,
						VP_HideMenuBar						= 8,
						VP_LibraryCoverSize					= 9,
						VP_LibraryHasCovers					= 10,
						VP_LibraryHasCoverBelowTracks		= 11,
						VP_LibraryCoverBelowTracksOpacity	= 12,
						VP_LibraryHasStarsNextToTrack		= 13,
						VP_LibraryHasStarsForUnrated		= 14,
						VP_CanSendTracksToEditor			= 15};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* instance();

	/** Returns the actual size of media buttons. */
	int buttonsSize() const;

	qreal coverBelowTracksOpacity() const;

	/** Returns the size of a cover. */
	int coverSizeLibraryTree() const;
	int coverSizeUniqueLibrary() const;

	/** Returns true if big and faded covers are displayed in the library when an album is expanded. */
	bool isCoverBelowTracksEnabled() const;

	/** Returns true if the button in parameter is visible or not. */
	bool isMediaButtonVisible(const QString & buttonName) const;

	/** Returns true if star outline must be displayed in the library. */
	bool isShowNeverScored() const;

	/** Returns true if the volume value in percent is always visible in the upper left corner of the widget. */
	bool isVolumeBarTextAlwaysVisible() const;

	/** Returns true if stars are visible and active. */
	bool libraryHasStars() const;

	void setCoverBelowTracksEnabled(bool b);

	void setCoverBelowTracksOpacity(int v);

	void setCovers(bool b);

	void setCoverSizeLibraryTree(int s);
	void setCoverSizeUniqueLibrary(int s);

	/** Sets if the button in parameter is visible or not. */
	void setMediaButtonVisible(const QString & buttonName, const bool &value);

	void setShowNeverScored(bool b);

	/** Sets if stars are visible and active. */
	void setStarsInLibrary(const bool &value);

	QMap<QString, QVariant> shortcuts() const;

	RequestSqlModel sqlModel() const;

	/** Returns the actual theme name. */
	QString theme() const;

	/** Returns volume from the slider. */
	qreal volume() const;

private:
	void initShortcuts();

public slots:
	/** Sets a new button size. */
	void setButtonsSize(int s);

	/** Sets a new theme. */
	void setThemeName(const QString &theme);

	/** Sets volume from the slider. */
	void setVolume(qreal v);

	void setVolumeBarTextAlwaysVisible(bool b);

signals:
	void mediaButtonVisibilityChanged(const QString &buttonName, bool value);

	void themeHasChanged(const QString &theme);

	void viewPropertyChanged(ViewProperty vp, const QVariant &value);
};

#endif // SETTINGS_H
