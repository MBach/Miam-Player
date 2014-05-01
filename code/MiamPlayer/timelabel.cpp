#include "timelabel.h"

#include <QDateTime>
#include <QEvent>

#include <QtDebug>

TimeLabel::TimeLabel(QWidget *parent) :
	QLabel(parent), _mode(0)
{
	this->installEventFilter(this);
	connect(this, &TimeLabel::timeChanged, this, &TimeLabel::display);

	connect(this, &TimeLabel::aboutToChangeTime, [=]() {
		qDebug() << Q_FUNC_INFO;
		emit timeChanged();
	});
}

/** Setter. */
void TimeLabel::setTime(qint64 time, qint64 total)
{
	_time = time;
	_total = total;
	emit timeChanged();
}

/** Redefined to filter mouse press event. */
bool TimeLabel::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		// Only 3 modes (might be overkill to create an Enum just for this)
		_mode < 2 ? _mode++ : _mode = 0;

		// Need to force update the label
		emit timeChanged();
	}
	return QLabel::eventFilter(obj, event);
}

/** Display track length using the selected mode. */
void TimeLabel::display()
{
	switch (_mode) {
	case 0:
		if (_time == 0) {
			this->setText("--:--");
		} else {
			this->setText(QDateTime::fromTime_t(_time / 1000).toString("mm:ss"));
		}
		break;
	case 1:
		if (_time == 0) {
			this->setText("--:--");
		} else {
			this->setText(QDateTime::fromTime_t((_total - _time) / 1000).toString("-mm:ss"));
		}
		break;
	case 2:
		if (_time == 0) {
			this->setText("--:-- / --:--");
		} else {
			this->setText(QDateTime::fromTime_t(_time / 1000).toString("mm:ss").append(" / ").append(QDateTime::fromTime_t(_total / 1000).toString("mm:ss")));
		}
		break;
	}
}
