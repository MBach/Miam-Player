#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <QPushButton>
#include "mediaplayer.h"

#include "miamcore_global.h"

/**
 * \brief		The MediaButton class is useful for buttons like "Play", "Stop", etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaButton : public QPushButton
{
	Q_OBJECT
public:
	MediaPlayer *_mediaPlayer;

	MediaButton(QWidget *parent = nullptr);

	virtual void setMediaPlayer(MediaPlayer *mediaPlayer);

	/** Redefined to load custom icons saved in settings. */
	void setIcon(const QIcon &);

protected:
	virtual void paintEvent(QPaintEvent *) override;

public slots:
	/** Load an icon from a chosen theme in options. */
	void setIconFromTheme(const QString &);

	/** Change the size of icons from the options. */
	void setSize(const int &);
};

#endif // MEDIABUTTON_H
