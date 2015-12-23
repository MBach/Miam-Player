#include "jumptowidget.h"
#include "settingsprivate.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QScrollBar>
#include <QStylePainter>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <QtDebug>

JumpToWidget::JumpToWidget(QAbstractItemView *view) :
	QWidget(view), _view(view), _pos(-1, -1)
{
	this->installEventFilter(this);
	this->setMouseTracking(true);
	auto settings = SettingsPrivate::instance();
	this->setFont(settings->font(SettingsPrivate::FF_Library));
	connect(settings, &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff, const QFont &font){
		if (ff == SettingsPrivate::FF_Library) {
			this->setFont(font);
		}
	});
	this->setBackgroundRole(QPalette::Button);
}

bool JumpToWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel) {
		return QApplication::sendEvent(_view->viewport(), event);
	} else if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent) {
			// The A-Z order has been chosen arbitrarily
			// This Widget won't automatically adapt to non-latin languages, like Russian, Japanese hiraganas, etc.
			int v = mouseEvent->y() * 26 / height();
			// in ASCII, the A letter is 65
			QString s(v + 65);
			emit aboutToScrollTo(s);
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

QSize JumpToWidget::sizeHint() const
{
	//qDebug() << Q_FUNC_INFO << QSize(20, _view->height());
	return QSize(20, _view->height());
}

void JumpToWidget::leaveEvent(QEvent *event)
{
	_pos = QPoint(-1, -1);
	QWidget::leaveEvent(event);
	this->update();
}

void JumpToWidget::mouseMoveEvent(QMouseEvent *event)
{
	_pos = event->pos();
	QWidget::mouseMoveEvent(event);
	this->update();
}

/** Reduce the font if this widget is too small. */
void JumpToWidget::resizeEvent(QResizeEvent *event)
{
	QFont f = this->font();
	int fontPointSize = SettingsPrivate::instance()->fontSize(SettingsPrivate::FF_Library);
	f.setPointSizeF(qMin((qreal)fontPointSize, height() / 60.0));
	this->setFont(f);
	QWidget::resizeEvent(event);
}

void JumpToWidget::paintEvent(QPaintEvent *)
{
	this->setMinimumSize(23, _view->height());
	this->setMaximumSize(23, _view->height());
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_view);
	o.palette = QApplication::palette();
	p.fillRect(rect(), o.palette.window());

	QFont f = p.font();
	for (int i = 0; i < 26; i++) {
		p.save();
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 22, height() / 26);
		if (_currentLetter == qc) {
			// Display a bright selection rectangle corresponding to the top letter in the library
			p.fillRect(r, o.palette.highlight());
		} else if (o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos)) {
			// Display a light rectangle under the mouse pointer
			p.fillRect(r, o.palette.highlight().color().lighter(160));
		}
		if (o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos)) {
			QColor lighterBG = o.palette.highlight().color().lighter(160);
			QColor highlightedText = o.palette.highlightedText().color();
			if (qAbs(lighterBG.value() - highlightedText.value()) > 128) {
				p.setPen(highlightedText);
			} else {
				p.setPen(o.palette.windowText().color());
			}
		} else {
			p.setPen(o.palette.windowText().color());
		}
		if (_lettersToHighlight.contains(qc)) {
			p.save();
			f.setBold(true);
			p.setFont(f);
			p.drawText(r, Qt::AlignCenter, qc);
			p.restore();
		} else {
			p.drawText(r, Qt::AlignCenter, qc);
		}
		p.restore();
	}

	// Draw a vertical line if there are few items in the library
	if (!_view->verticalScrollBar()->isVisible()) {
		p.setPen(o.palette.mid().color());
		if (isLeftToRight()) {
			p.drawLine(rect().topRight(), rect().bottomRight());
		} else {
			p.drawLine(rect().topLeft(), rect().bottomLeft());
		}
	}
}
