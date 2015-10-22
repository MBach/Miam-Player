#ifndef PLAYLISTFRAME_H
#define PLAYLISTFRAME_H

#include <QApplication>
#include <QStylePainter>
#include <QWidget>

class PlaylistFrame : public QWidget
{
	Q_OBJECT
public:
	PlaylistFrame(QWidget *parent) : QWidget(parent) {}

protected:
	inline virtual void paintEvent(QPaintEvent *) {
		QStylePainter p(this);
		p.setPen(QApplication::palette().mid().color());
		if (isLeftToRight()) {
			p.drawLine(rect().topLeft(), rect().bottomLeft());
		} else {
			p.drawLine(rect().topRight(), rect().bottomRight());
		}
		p.drawLine(rect().bottomLeft(), rect().bottomRight());
	}
};

#endif // PLAYLISTFRAME_H
