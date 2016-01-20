#ifndef VIEWPLAYLISTS_H
#define VIEWPLAYLISTS_H

#include "ui_viewplaylists.h"

class ViewPlaylists : public QWidget, public Ui::ViewPlaylists
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;

public:
	ViewPlaylists(MediaPlayer *mediaPlayer);
};

#endif // VIEWPLAYLISTS_H
