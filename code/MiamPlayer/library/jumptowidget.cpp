#include "jumptowidget.h"

#include <QLabel>
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
			int v = mouseEvent->y() * 26 / height();
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
	return QSize(19, _libraryTreeView->height());
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
	this->setMinimumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	this->setMaximumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_libraryTreeView);
	p.drawPrimitive(QStyle::PE_FrameButtonTool, o);

	// Reduce the font if this widget is too small
	QFont f = p.font();
	f.setPixelSize(height() / 26);
	for (int i = 0; i < 26; i++) {
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 19, height() / 26);
		//Settings *settings = Settings::getInstance();
		if (_libraryTreeView->currentLetter() == qc) {
			// Display a bright selection rectangle corresponding to the top letter in the library
			//if (settings->isCustomColors()) {
			//	p.fillRect(r, settings->customColors(Settings::ColorHighlight));
			//} else {
				p.fillRect(r, o.palette.highlight());
			//}
		} else if (o.state & QStyle::State_MouseOver && r.contains(_pos)) {
			// Display a light rectangle under the mouse pointer
			//if (settings->isCustomColors()) {
			//	p.fillRect(r, settings->customColors(Settings::ColorHighlight).lighter());
			//} else {
				p.fillRect(r, o.palette.highlight().color().lighter(160));
			//}
		}
		p.save();
		if (r.height() < p.fontMetrics().height() && r.width() >= p.fontMetrics().width(qc)) {
			p.setFont(f);
		}
		//Check if selected color and QPalette::WindowText has poor contrast
		/*QColor hiColor = settings->customColors(Settings::ColorHighlight);
		if (hiColor.value() > 128) {
			p.setPen(o.palette.windowText().color());
		} else {
			p.setPen(o.palette.highlightedText().color());
		}*/
		p.drawText(r, Qt::AlignCenter, qc);
		p.restore();
	}
}
