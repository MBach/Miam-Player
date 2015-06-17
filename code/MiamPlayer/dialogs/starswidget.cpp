#include "starswidget.h"

#include <QPainter>
#include <QtDebug>

StarsWidget::StarsWidget(QWidget *parent) : QWidget(parent)
{
	_starRating.setStarCount(5);
}

void StarsWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QStyleOptionViewItem o;
	o.rect = rect();
	o.palette = this->palette();
	if (isEnabled()) {
		_starRating.paintStars(&p, o, StarRating::ReadOnly);
	} else {
		_starRating.paintStars(&p, o, StarRating::NoStarsYet);
	}
}
