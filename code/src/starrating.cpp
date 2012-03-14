#include <QtGui>
#include <math.h>

#include "starrating.h"

int StarRating::maxStars=5;

StarRating::StarRating(int starCount)
{
	stars = starCount;
}

void StarRating::paint(QPainter *painter, const QRect *rect, const QPalette &palette, EditMode mode) const
{
	const QIcon favIcon(":/icons/favorite");

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);

	if (mode == Editable) {
		//qDebug() << "on est en EditMode !!!";
		painter->setBrush(palette.highlight());
	} else {
		painter->setBrush(palette.foreground());
	}

	/*if (mode == 0) {
		qDebug() << "stars to paint:" << stars;
		favIcon.paint(painter, *rect, Qt::AlignRight, QIcon::Active);
	} else {*/
		for (int i=maxStars; i>0; i--) {
			if (i!=maxStars || i>0) {
				painter->translate(-1.0 * (favIcon.actualSize(rect->size()).width()+3.0), 0);
			}
			if (i > stars) {
				favIcon.paint(painter, *rect, Qt::AlignRight, QIcon::Disabled);
			} else {
				favIcon.paint(painter, *rect, Qt::AlignRight, QIcon::Normal);
			}
		}
	//}
}

QSize StarRating::sizeHint() const
{
	// todo ?
	return maxStars * QSize(19+3.0, 1);
}
