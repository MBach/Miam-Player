#ifndef PAINTABLEWIDGET_H
#define PAINTABLEWIDGET_H

#include <QApplication>
#include <QPainter>
#include <QWidget>

/**
 * \brief		The PaintableWidget class is a small class which can react to color change.
 * \details		When one is updating colors in options, this class dynamically repaints itself by adapting the background color.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class PaintableWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PaintableWidget(QWidget *parent) : QWidget(parent) {}

protected:
	virtual void paintEvent(QPaintEvent *)
	{
		QPainter p(this);
		p.setPen(QApplication::palette().text().color());
		p.setBrush(QApplication::palette().base());
		p.drawRect(this->rect().adjusted(0, 0, -1, -1));
	}
};

#endif // PAINTABLEWIDGET_H
