#ifndef PLAYBACKMODEWIDGETFACTORY_H
#define PLAYBACKMODEWIDGETFACTORY_H

#include <QPushButton>
#include "tabplaylist.h"
#include "playbackmodewidget.h"

/// Factory or helper?
class PlaybackModeWidgetFactory : public QObject
{
	Q_OBJECT
private:
	Q_ENUMS(Edge)

	QPushButton *_playbackModeButton;

	QList<PlaybackModeWidget*> _popups;

public:
	PlaybackModeWidgetFactory(QWidget *parent, QPushButton *playbackModeButton, TabPlaylist *tabPlaylists);

	enum Edge { UNDEFINED	= -1,
				TOP			= 0,
				RIGHT		= 1,
				BOTTOM		= 2,
				LEFT		= 3
			   };

	void move();

private:
	Edge _previousEdge;

public slots:
	void togglePlaybackModes();
};

#endif // PLAYBACKMODEWIDGETFACTORY_H
