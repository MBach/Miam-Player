#ifndef STARSWIDGET_H
#define STARSWIDGET_H

#include "starrating.h"
#include <QWidget>

/**
 * \brief		The StarsWidget class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class StarsWidget : public QWidget
{
	Q_OBJECT
private:
	StarRating _starRating;

public:
	explicit StarsWidget(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // STARSWIDGET_H
