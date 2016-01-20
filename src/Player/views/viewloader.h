#ifndef VIEWLOADER_H
#define VIEWLOADER_H

#include <mediaplayer.h>
#include <QString>

class ViewLoader
{
private:
	MediaPlayer *_mediaPlayer;

public:
	ViewLoader(MediaPlayer *mediaPlayer);

	QWidget *load(const QString &action);
};

#endif // VIEWLOADER_H
