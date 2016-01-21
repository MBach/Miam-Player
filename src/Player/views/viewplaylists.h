#ifndef VIEWPLAYLISTS_H
#define VIEWPLAYLISTS_H

#include <playbackmodewidgetfactory.h>
#include "dialogs/searchdialog.h"
#include "abstractview.h"

#include <QMenu>

#include "ui_viewplaylists.h"

class ViewPlaylists : public AbstractView, public Ui::ViewPlaylists
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;
	SearchDialog *_searchDialog;

	/** Displays and animates the media button "PlaybackMode". */
	PlaybackModeWidgetFactory *_playbackModeWidgetFactory;

public:
	ViewPlaylists(QMenu *menuPlaylist, MediaPlayer *mediaPlayer);

protected:
	virtual void moveEvent(QMoveEvent *event) override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;

private slots:
	void mediaPlayerStateHasChanged(QMediaPlayer::State state);
};

#endif // VIEWPLAYLISTS_H
