#ifndef PLAYBACKMODEWIDGET_H
#define PLAYBACKMODEWIDGET_H

#include <QMediaPlaylist>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

class PlaybackModeWidget : public QWidget
{
	Q_OBJECT
private:
	QMediaPlaylist::PlaybackMode _mode;

	QPropertyAnimation *_animation;

public:
	explicit PlaybackModeWidget(QMediaPlaylist::PlaybackMode mode, QPushButton *playbackModeButton = 0);

	void animate(const QPoint &start, const QPoint &end);

	inline QPushButton *button() const { return qobject_cast<QPushButton*>(this->layout()->itemAt(0)->widget()); }

	inline QMediaPlaylist::PlaybackMode mode() const { return _mode;}
};

#endif // PLAYBACKMODEWIDGET_H
