#include "jumptowidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QScrollBar>
#include <QStylePainter>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <QtDebug>

JumpToWidget::JumpToWidget(QAbstractItemView *treeView) :
	QWidget(treeView), _view(treeView), _pos(-1, -1)
{
	this->installEventFilter(this);
	this->setMouseTracking(true);
}

bool JumpToWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel) {
		return QApplication::sendEvent(_view->viewport(), event);
	} else if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent) {
			// The A-Z order has been chosen arbitrarily
			int v = mouseEvent->y() * 26 / height();
			// in ASCII, the A letter is 65
			QString s(v + 65);
			emit displayItemDelegate(false);
			/// FIXME
			/// _libraryTreeView->jumpTo(s);
			emit aboutToScrollTo(s);
			emit displayItemDelegate(true);
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

void JumpToWidget::setCurrentLetter(const QChar &currentLetter)
{
	_currentLetter = currentLetter;
}

QSize JumpToWidget::sizeHint() const
{
	return QSize(20, _view->height());
}

void JumpToWidget::leaveEvent(QEvent *e)
{
	_pos = QPoint(-1, -1);
	QWidget::leaveEvent(e);
	this->update();
}

void JumpToWidget::mouseMoveEvent(QMouseEvent *e)
{
	_pos = e->pos();
	QWidget::mouseMoveEvent(e);
	this->update();
}

void JumpToWidget::paintEvent(QPaintEvent *)
{
	this->setMinimumSize(20, _view->height());
	this->setMaximumSize(20, _view->height());
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_view);
	o.palette = QApplication::palette();
	p.fillRect(rect(), o.palette.window());

	// Reduce the font if this widget is too small
	QFont f = p.font();
	f.setPixelSize(height() / 26);
	for (int i = 0; i < 26; i++) {
		p.save();
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 19, height() / 26);
		if (_currentLetter == qc) {
			// Display a bright selection rectangle corresponding to the top letter in the library
			p.fillRect(r, o.palette.highlight());
		} else if (o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos)) {
			// Display a light rectangle under the mouse pointer
			p.fillRect(r, o.palette.highlight().color().lighter(160));
		}

		if (r.height() + 4 < p.fontMetrics().height() && r.width() >= p.fontMetrics().width(qc)) {
			p.setFont(f);
		}
		if (o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos)) {
			QColor lighterBG = o.palette.highlight().color().lighter(160);
			QColor highlightedText = o.palette.highlightedText().color();
			if (qAbs(lighterBG.saturation() - highlightedText.saturation()) > 128) {
				p.setPen(highlightedText);
			} else {
				p.setPen(o.palette.windowText().color());
			}
		} else {
			p.setPen(o.palette.windowText().color());
		}
		p.drawText(r, Qt::AlignCenter, qc);
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
