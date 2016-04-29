#ifndef ABSTRACTVIEW_H
#define ABSTRACTVIEW_H

#include "miamcore_global.h"
#include "mediaplayer.h"
#include "mediaplayercontrol.h"
#include "settings.h"

#include <QDir>
#include <QModelIndex>

class MusicSearchEngine;

/**
 * \brief		The AbstractView class is the base class for all views in Miam-Player.
 * \details		Every view in the player should inherit from this hierarchy in order to provide minimal functionalities
 *				to have a ready-to-work player. Usually, a view in a media player has a seekbar, a volume slider and some specific areas.
 *				But it can be completely different, based on the general properties each Media Control View offers.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY AbstractView : public QWidget
{
	Q_OBJECT
private:
	Q_ENUMS(ViewType)

	AbstractView *_origin;

protected:
	MediaPlayerControl *_mediaPlayerControl;

public:
	AbstractView(MediaPlayerControl *mediaPlayerControl, QWidget *parent = nullptr)
		: QWidget(parent)
		, _origin(nullptr)
		, _mediaPlayerControl(mediaPlayerControl) {}

	enum ViewType {
		VT_BuiltIn	= 0,
		VT_Plugin	= 1
	};

	virtual ~AbstractView() {}

	virtual void bindShortcut(const QString & /*objectName*/, const QKeySequence & /*keySequence*/) {}

	virtual QPair<QString, QObjectList> extensionPoints() const { return qMakePair(QString(), QObjectList()); }

	inline MediaPlayerControl* mediaPlayerControl() const { return _mediaPlayerControl; }

	virtual void setMusicSearchEngine(MusicSearchEngine *) {}

	inline virtual void setMediaPlayerControl(MediaPlayerControl *mpc) { _mediaPlayerControl = mpc; }

	inline void setOrigin(AbstractView *origin) { _origin = origin; }
	inline AbstractView* origin() const { return _origin; }

	virtual ViewType type() const = 0;

	virtual bool viewProperty(Settings::ViewProperty) const { return false; }

public slots:
	virtual void initFileExplorer(const QDir &) {}

	virtual void setViewProperty(Settings::ViewProperty vp, QVariant value) = 0;

	virtual void volumeSliderIncrease() {}

	virtual void volumeSliderDecrease() {}

signals:
	void modelReloadRequested();

	void aboutToSendToTagEditor(const QModelIndexList &indexes, const QList<QUrl> &tracks);
};

#endif // ABSTRACTVIEW_H
