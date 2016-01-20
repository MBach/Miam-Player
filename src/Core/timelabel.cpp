#include "timelabel.h"

#include <QDateTime>
#include <QEvent>
#include <cmath>

#include "settingsprivate.h"

#include <QtDebug>

TimeLabel::TimeLabel(QWidget *parent) :
	QLabel(parent), _time(0), _total(0)
{
	this->installEventFilter(this);
	this->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	connect(this, &TimeLabel::timeChanged, this, &TimeLabel::display);

	_mode = SettingsPrivate::instance()->value("timeMode").toInt();
}

/** Redefined to filter mouse press event. */
bool TimeLabel::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		// Only 3 modes (might be overkill to create an Enum just for this)
		_mode < 2 ? _mode++ : _mode = 0;
		SettingsPrivate::instance()->setValue("timeMode", _mode);

		// Need to force update the label
		emit timeChanged();
	}
	return QLabel::eventFilter(obj, event);
}

QSize TimeLabel::minimumSizeHint() const
{
	QSize s = QLabel::minimumSizeHint();
	QDateTime::fromTime_t((_total - _time) / 1000).toString("-mm:ss");
	static const QString text("-00:00");
	if (fontMetrics().width(text) > s.width()) {
		s.setWidth(fontMetrics().width(text));
	}
	return s;
}


/** Setter. */
void TimeLabel::setTime(qint64 time, qint64 total)
{
	_time = time;
	_total = total;
	emit timeChanged();
}

/** Display track length using the selected mode. */
void TimeLabel::display()
{
	switch (_mode) {
	case 0:
		if (_time == 0 && _total == 0) {
			this->setText("--:--");
		} else {
			this->setText(QDateTime::fromTime_t(_time / 1000).toString("mm:ss"));
		}
		break;
	case 1:
		if (_time == 0 && _total == 0) {
			this->setText("--:--");
		} else {
			this->setText(QDateTime::fromTime_t((_total - _time) / 1000).toString("-mm:ss"));
		}
		break;
	case 2:
		if (_time == 0 && _total == 0) {
			this->setText("--:-- / --:--");
		} else {
			uint t = round(_total / 1000);
			this->setText(QDateTime::fromTime_t(_time / 1000).toString("mm:ss").append(" / ").append(QDateTime::fromTime_t(t).toString("mm:ss")));
		}
		break;
	}
}
