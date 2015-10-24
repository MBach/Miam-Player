#ifndef PLAYBACKMODEWIDGET_H
#define PLAYBACKMODEWIDGET_H

#include <QMediaPlaylist>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * \brief		The PlaybackModeWidget class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class PlaybackModeWidget : public QWidget
{
	Q_OBJECT
private:
	QMediaPlaylist::PlaybackMode _mode;

	QPropertyAnimation *_animation;

	QString _playbackMode;

public:
	explicit PlaybackModeWidget(QMediaPlaylist::PlaybackMode mode, QPushButton *playbackModeButton = 0);

	/** Convert Enum in QString to dynamically load icons. */
	static QString nameFromMode(QMediaPlaylist::PlaybackMode mode);

	/** Reload icon when theme has changed or buttons size was changed in options by one. */
	void adjustIcon();

	/** Animates this button in circle or in line. */
	void animate(const QPoint &start, const QPoint &end);

	inline QPushButton *button() const { return qobject_cast<QPushButton*>(this->layout()->itemAt(0)->widget()); }

	inline QMediaPlaylist::PlaybackMode mode() const { return _mode; }
};

#endif // PLAYBACKMODEWIDGET_H
