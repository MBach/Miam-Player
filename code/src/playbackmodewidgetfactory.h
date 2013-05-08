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

public:
	PlaybackModeWidgetFactory(QWidget *parent, QPushButton *playbackModeButton, TabPlaylist *tabPlaylists);

	//void syncPositions();

	void hideAll();

private:
	QRect moveButtons(int index);

public slots:
	void togglePlaybackModes();
};

#endif // PLAYBACKMODEWIDGETFACTORY_H
