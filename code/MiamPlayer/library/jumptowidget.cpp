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
			_libraryTreeView->jumpTo(s);
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
	//qDebug() << Q_FUNC_INFO;
	//if (this->height() < _libraryTreeView->height()) {
	this->setMinimumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	this->setMaximumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	//} else {
	//	this->setMinimumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	//	this->setMaximumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	//}
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_libraryTreeView);
	p.drawPrimitive(QStyle::PE_FrameButtonTool, o);
	for (int i = 0; i < 26; i++) {
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 19, height() / 26);
		if (_libraryTreeView->currentLetter() == qc) {
			p.fillRect(r, o.palette.highlight());
		} else if (o.state & QStyle::State_MouseOver && r.contains(_pos)) {
			p.fillRect(r, o.palette.highlight().color().lighter(160));
		}
		p.save();
		p.setBrush(o.palette.color(QPalette::WindowText));
		p.drawText(r, Qt::AlignCenter, qc);
		p.restore();
	}
}
