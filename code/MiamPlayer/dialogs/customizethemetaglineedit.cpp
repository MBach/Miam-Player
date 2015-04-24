#include "customizethemetaglineedit.h"

#include <QEvent>

#include <QtDebug>

CustomizeThemeTagLineEdit::CustomizeThemeTagLineEdit(QWidget *parent)
	: TagLineEdit(parent), _timerTag(new QTimer(this))
{
	_timerTag->setInterval(1000);
	_timerTag->setSingleShot(true);

	connect(_timerTag, &QTimer::timeout, this, [=]() {
		QString t = text();
		this->blockSignals(true);
		this->clear();
		this->blockSignals(false);
		this->addTag(t);
		qDebug() << Q_FUNC_INFO;
		emit taglistHasChanged(this->toStringList());
	});
}

bool CustomizeThemeTagLineEdit::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == this && event->type() == QEvent::KeyRelease) {
		_timerTag->start();
	} else if (obj == this && event->type() == QEvent::KeyPress) {
		if (_timerTag->isActive()) {
			_timerTag->start();
		}
	}
	return TagLineEdit::eventFilter(obj, event);
}

void CustomizeThemeTagLineEdit::closeTagButton(TagButton *t)
{
	qDebug() << Q_FUNC_INFO;
	TagLineEdit::closeTagButton(t);
	emit taglistHasChanged(this->toStringList());
}
