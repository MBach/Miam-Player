#include "transparentspinbox.h"

#include <QtDebug>

TransparentSpinBox::TransparentSpinBox(QWidget *parent) :
	QSpinBox(parent), _dialog(NULL)
{
	_timer = new QTimer(this);
	_timer->setInterval(3000);
	_timer->setSingleShot(true);

	connect(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]() {
		if (_dialog && _dialog->isVisible()) {
			_dialog->setWindowOpacity(0.5);
			_timer->start();
		}
	});

	connect(_timer, &QTimer::timeout, [=]() { _dialog->setWindowOpacity(1.0); });
}

void TransparentSpinBox::focusOutEvent(QFocusEvent *event)
{
	if (_timer->isActive()) {
		_timer->stop();
	}
	_dialog->setWindowOpacity(1.0);
	QSpinBox::focusOutEvent(event);
}
