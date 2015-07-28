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
		int firstPos = INT_MAX;
		int spaces = 0;
		for (TagButton *tag : _tags) {
			if (firstPos > tag->position()) {
				firstPos = tag->position();
			}
			spaces += tag->spaceCount();
		}
		QString t2 = t.mid(firstPos, spaces);
		this->setText(t2);
		this->addTag(t);
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
	TagLineEdit::closeTagButton(t);
	qDebug() << Q_FUNC_INFO << this->toStringList();
	emit taglistHasChanged(this->toStringList());
}
