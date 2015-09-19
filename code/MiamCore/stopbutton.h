#ifndef STOPBUTTON_H
#define STOPBUTTON_H

#include "mediabutton.h"

#include <QMenu>

/**
 * \brief		The StopButton class is a custom class for the Stop button only
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY StopButton : public MediaButton
{
	Q_OBJECT
private:
	QMenu _menu;
	QAction *_action;

public:
	explicit StopButton(QWidget *parent = nullptr);

	virtual void setMediaPlayer(MediaPlayer *mediaPlayer) override;

protected:
	virtual void contextMenuEvent(QContextMenuEvent *) override;
};

#endif // STOPBUTTON_H
