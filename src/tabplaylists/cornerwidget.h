#ifndef CORNERWIDGET_H
#define CORNERWIDGET_H

#include <QPushButton>

#include "miamtabplaylists_global.hpp"

/**
 * \brief		The CornerWidget class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY CornerWidget : public QPushButton
{
	Q_OBJECT
public:
	explicit CornerWidget(QWidget *parent);

protected:
	virtual void mouseMoveEvent(QMouseEvent *e) override;

	virtual void paintEvent(QPaintEvent *) override;

signals:
	void innerButtonClicked();
};

#endif // CORNERWIDGET_H
