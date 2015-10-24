#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QPropertyAnimation>
#include <QStylePainter>
#include <QTimer>

#include "miamcore_global.h"
#include "searchbar.h"

/**
 * \brief		The LineEdit class
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY LineEdit : public SearchBar
{
	Q_OBJECT

private:
	QTimer *_timer;
	int _fps;
	QPropertyAnimation _fade;

public:
	explicit LineEdit(QWidget *parent = 0);

protected:
	virtual void drawCursor(QStylePainter *painter, const QRect &rText);

	virtual void focusInEvent(QFocusEvent *e) override;

	virtual void focusOutEvent(QFocusEvent *e) override;
};

#endif // LINEEDIT_H
