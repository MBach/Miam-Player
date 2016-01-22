#ifndef VIEWLOADER_H
#define VIEWLOADER_H

#include <mediaplayer.h>
#include <QMenuBar>
#include <QString>
#include "abstractview.h"

class ViewLoader
{
private:
	MediaPlayer *_mediaPlayer;

public:
	ViewLoader(MediaPlayer *mediaPlayer);

	AbstractView *load(const QString &menuAction);
};

#endif // VIEWLOADER_H
