#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QPropertyAnimation>
#include <QStylePainter>
#include <QTimer>

class LineEdit : public QLineEdit
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

	virtual void focusInEvent(QFocusEvent *e);

	virtual void focusOutEvent(QFocusEvent *e);
};

#endif // LINEEDIT_H
