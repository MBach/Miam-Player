#ifndef VIEWPLAYLISTS_H
#define VIEWPLAYLISTS_H

#include <abstractviewplaylists.h>
#include <playbackmodewidgetfactory.h>
#include "dialogs/searchdialog.h"

#include <QMenu>

#include "ui_viewplaylists.h"

class ViewPlaylists : public AbstractViewPlaylists, public Ui::ViewPlaylists
{
	Q_OBJECT
private:
	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;
	SearchDialog *_searchDialog;

	/** Displays and animates the media button "PlaybackMode". */
	PlaybackModeWidgetFactory *_playbackModeWidgetFactory;

public:
	ViewPlaylists(MediaPlayer *mediaPlayer);

	inline virtual bool hasPlaylistFeature() const override { return true; }

	virtual int selectedTracksInCurrentPlaylist() const override;

protected:
	virtual void moveEvent(QMoveEvent *event) override;

public slots:
	virtual void addPlaylist() override;

	virtual void removeCurrentPlaylist() override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;

private slots:
	void mediaPlayerStateHasChanged(QMediaPlayer::State state);
};

#endif // VIEWPLAYLISTS_H
