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
	QList<PlaybackModeWidget*> _popups;
	QPushButton *_playbackModeButton;

	Q_ENUMS(Edge)

public:
	PlaybackModeWidgetFactory(QWidget *parent, QPushButton *playbackModeButton, TabPlaylist *tabPlaylists);

	enum Edge { UNDEFINED	= -1,
				TOP			= 0,
				RIGHT		= 1,
				BOTTOM		= 2,
				LEFT		= 3
			   };

	void hideAll();

	void move();

private:
	QRect moveButtons(int index);

public slots:
	void togglePlaybackModes();
};

#endif // PLAYBACKMODEWIDGETFACTORY_H
