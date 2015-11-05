#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>
#include "miamlibrary_global.hpp"

/**
 * \brief		The ScrollBar class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY ScrollBar : public QScrollBar
{
	Q_OBJECT
private:
	int _isDown;

	bool _top, _left, _bottom, _right;

public:
	explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

	void setFrameBorder(bool top, bool left, bool bottom, bool right);

protected:
	virtual void mousePressEvent(QMouseEvent *e) override;

	virtual void mouseReleaseEvent(QMouseEvent *e) override;

	virtual void paintEvent(QPaintEvent *e) override;
};

#endif // SCROLLBAR_H
