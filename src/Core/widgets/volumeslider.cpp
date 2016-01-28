#include "volumeslider.h"
#include "settingsprivate.h"
#include "settings.h"

#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QStylePainter>

#include <cmath>

#include <QtDebug>

VolumeSlider::VolumeSlider(QWidget *parent)
	: QSlider(parent)
	, _timer(new QTimer(this))
	, _isDown(false)
{
	this->setMinimumSize(50, 35);
	this->setMaximumSize(100, 100);
	this->setMaximum(100);
	this->setPageStep(0);
	this->setValue(99);
	this->setTracking(false);
	this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	_timer->setSingleShot(true);

	// Update the volume instantly
	connect(this, &QSlider::sliderMoved, [=](int v) {
		_isDown = true;
		this->setValue(v);
		this->update();
	});

	// Used to display percentage on the screen (like '75%')
	connect(this, &QSlider::sliderPressed, [=]() {
		_isDown = true;
		this->update();
	});

	SettingsPrivate *settings = SettingsPrivate::instance();
	connect(settings, &SettingsPrivate::viewPropertyChanged, this, [=](SettingsPrivate::ViewProperty vp, QVariant) {
		if (vp == SettingsPrivate::VP_VolumeIndicatorToggled) {
			this->update();
		}
	});

	// Used to hide percentage on the screen (like '75%')
	connect(this, &QSlider::sliderReleased, [=]() { _timer->start(settings->volumeBarHideAfter() * 1000); });

	// Simulate pressed / released event for wheel event too!
	// A repaint event will occur later so text will be removed when one will move the mouse outside the widget
	connect(_timer, &QTimer::timeout, [=]() {
		// One can hold the left button more than 1 second
		_isDown = false || QGuiApplication::mouseButtons().testFlag(Qt::LeftButton);
		this->update();
		Settings::instance()->setVolume((qreal)value() / 100.0);
	});

	connect(this, &QSlider::valueChanged, [=]() { _timer->start(settings->volumeBarHideAfter() * 1000); });
	this->setSingleStep(5);
	this->installEventFilter(this);
}

/** Redefined to react to default keys */
bool VolumeSlider::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
		if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
			_isDown = true;
			_timer->start(SettingsPrivate::instance()->volumeBarHideAfter() * 1000);
		}
	}
	return QSlider::eventFilter(obj, e);
}

void VolumeSlider::contextMenuEvent(QContextMenuEvent *e)
{
	QMenu m(this);
	QAction *actionBars = new QAction(tr("Bars"), this);
	QAction *actionLine = new QAction(tr("Line"), this);
	QAction *actionSlider = new QAction(tr("Slider"), this);

	actionBars->setCheckable(true);
	actionBars->setChecked(true);
	actionLine->setDisabled(true);
	actionSlider->setDisabled(true);

	m.addActions({ actionBars, actionLine, actionSlider});
	m.exec(e->globalPos());
}

/** Redefined. */
void VolumeSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		setValue(event->pos().x() * 100 / width());
		QSlider::mousePressEvent(event);
	}
}

/** Redefined for custom painting. */
void VolumeSlider::paintEvent(QPaintEvent *)
{
	static const float bars = 10.0;
	static double y0 = 4.0 / 5.0;
	float barWidth = width() / bars;

	// Do not take all available space
	float h = height() * 0.7;

	QStylePainter p(this);
	QStyleOptionSlider opt;
	opt.initFrom(this);
	opt.palette = QApplication::palette();

	int currentVolume = floor(value() / bars);

	for (int i = 0; i < bars; i++) {
		//linear interpolation: y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
		float y = y0 - y0 * i / bars;
		if (currentVolume >= i && value() > 0) {
			p.setPen(opt.palette.highlight().color());
			p.setBrush(opt.palette.highlight().color().lighter(100 + 100 * y));
		} else {
			p.setPen(opt.palette.mid().color());
			p.setBrush(opt.palette.window());
		}
		QRectF r;
		if (isLeftToRight()) {
			r.setX(i * barWidth);
		} else {
			r.setX(width() -  i * barWidth);
		}
		r.setY(height() * 0.15 + floor(y * h));
		r.setWidth(barWidth - 2);
		r.setHeight(h - floor(y * h));
		p.drawRect(r);
	}

	// When an action is triggered, display current volume in the upper left corner
	if (_isDown || SettingsPrivate::instance()->isVolumeBarTextAlwaysVisible()) {
		p.save();
		p.setPen(opt.palette.highlight().color());
		p.drawText(rect().adjusted(1, 2, 0, 0), Qt::AlignTop | Qt::AlignLeft, QString::number(value()).append("%"));
		p.restore();
	}
}

/** Redefined to allow one to change volume without having the focus on this widget. */
void VolumeSlider::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() > 0) {
		this->setValue(value() + singleStep());
	} else {
		this->setValue(value() - singleStep());
	}
	_isDown = true;
	if (!hasFocus()) {
		setFocus();
	}
	QSlider::wheelEvent(event);
}
