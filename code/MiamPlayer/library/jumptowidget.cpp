#include "jumptowidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QScrollBar>
#include <QStylePainter>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <QtDebug>

#include "librarytreeview.h"

JumpToWidget::JumpToWidget(LibraryTreeView *treeView) :
	QWidget(treeView), _libraryTreeView(treeView), _pos(-1, -1)
{
	this->installEventFilter(this);
	this->setMouseTracking(true);
}

bool JumpToWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel) {
		return QApplication::sendEvent(_libraryTreeView->viewport(), event);
	} else if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent) {
			// The A-Z order has been chosen arbitrarily
			int v = mouseEvent->y() * 26 / height();
			// in ASCII, the A letter is 65
			QString s(v + 65);
			emit displayItemDelegate(false);
			_libraryTreeView->jumpTo(s);
			emit displayItemDelegate(true);
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

QSize JumpToWidget::sizeHint() const
{
	return QSize(20, _libraryTreeView->height());
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
	this->setMinimumSize(20, _libraryTreeView->height() - _libraryTreeView->header()->height());
	this->setMaximumSize(20, _libraryTreeView->height() - _libraryTreeView->header()->height());
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_libraryTreeView);
	o.palette = QApplication::palette();
	//p.drawPrimitive(QStyle::PE_FrameButtonTool, o);
	p.fillRect(rect(), o.palette.window());

	//QColor hiColor = Settings::getInstance()->customColors(QPalette::Highlight);

	// Reduce the font if this widget is too small
	QFont f = p.font();
	f.setPixelSize(height() / 26);
	for (int i = 0; i < 26; i++) {
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 19, height() / 26);
		if (_libraryTreeView->currentLetter() == qc) {
			// Display a bright selection rectangle corresponding to the top letter in the library
			p.fillRect(r, o.palette.highlight());
		} else if (o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos)) {
			// Display a light rectangle under the mouse pointer
			p.fillRect(r, o.palette.highlight().color().lighter(160));
		}

		if (r.height() < p.fontMetrics().height() && r.width() >= p.fontMetrics().width(qc)) {
			p.setFont(f);
		}
		if (o.state.testFlag(QStyle::State_Selected)) {
			p.setPen(o.palette.highlightedText().color());
		} else if ((o.state.testFlag(QStyle::State_MouseOver) && r.contains(_pos) || _libraryTreeView->currentLetter() == qc)) {
			p.setPen(o.palette.highlightedText().color());
		} else {
			p.setPen(o.palette.windowText().color());
		}
		p.drawText(r, Qt::AlignCenter, qc);
	}

	// Draw a vertical line if there are few items in the library
	if (!_libraryTreeView->verticalScrollBar()->isVisible()) {
		p.setPen(o.palette.mid().color());
		if (isLeftToRight()) {
			p.drawLine(rect().topRight(), rect().bottomRight());
		} else {
			p.drawLine(rect().topLeft(), rect().bottomLeft());
		}
	}
}
