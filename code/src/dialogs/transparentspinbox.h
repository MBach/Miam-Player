#ifndef TRANSPARENTSPINBOX_H
#define TRANSPARENTSPINBOX_H

#include <QDialog>
#include <QSpinBox>
#include <QTimer>

class TransparentSpinBox : public QSpinBox
{
	Q_OBJECT
private:
	QDialog *_dialog;
	QTimer *_timer;

public:
	explicit TransparentSpinBox(QWidget *parent = 0);

	inline void setDialog(QDialog *dialog) { _dialog = dialog; }

protected:
	void focusOutEvent(QFocusEvent *event);
};

#endif // TRANSPARENTSPINBOX_H
