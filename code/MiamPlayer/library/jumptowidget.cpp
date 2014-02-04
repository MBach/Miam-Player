#include "jumptowidget.h"

#include <QLabel>
#include <QStylePainter>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <QtDebug>

#include "librarytreeview.h"

JumpToWidget::JumpToWidget(LibraryTreeView *treeView) :
	QWidget(treeView), _libraryTreeView(treeView)
{
	//QVBoxLayout *vLayout = new QVBoxLayout(this);
	/*for (int i = 0; i < 26; i++) {
		QLabel *label = new QLabel(this);
		char c = static_cast<char>(i + 65);
		QChar qc(c);
		QString letter("<a href=%1 style=\"text-decoration:none;\">%2</a>");
		letter = letter.arg(qc, qc);
		label->setText(letter);
		label->setAlignment(Qt::AlignCenter);
		//label->setFrameStyle(QFrame::Box);
		//label->setMinimumWidth(QScrollBar::sizeHint().width());
		label->setObjectName(qc);

		vLayout->addWidget(label);
		//connect(label, &QLabel::linkActivated, this, &LibraryTreeView::jumpTo);
	}*/
	//vLayout->addWidget(new QLabel(this));
	//vLayout->setContentsMargins(0, 0, 0, 0);
	//vLayout->setSpacing(0);
	//this->setLayout(vLayout);
	//this->setMinimumSize(QSize(19, _libraryTreeView->height()));

	this->installEventFilter(this);
}

bool JumpToWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel) {
		return QApplication::sendEvent(_libraryTreeView->viewport(), event);
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

QSize JumpToWidget::sizeHint() const
{
	return QSize(19, _libraryTreeView->height());
}

void JumpToWidget::paintEvent(QPaintEvent *event)
{
	//qDebug() << Q_FUNC_INFO;
	if (this->height() < _libraryTreeView->height()) {
		this->setMinimumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
		this->setMaximumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	} else {
		this->setMinimumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
		this->setMaximumSize(19, _libraryTreeView->height() - 2 - _libraryTreeView->header()->height());
	}
	QStylePainter p(this);
	QStyleOptionViewItem o;
	o.initFrom(_libraryTreeView);
	//p.fillRect(QRect(0, 0, width(), _libraryTreeView->height()), o.palette.alternateBase());
	//p.setPen(o.palette.background().color());
	//p.setBrush(o.palette.alternateBase());
	//_libraryTreeView->frameStyle();
	//p.setBrush(o.palette.color(Qf));
	//p.drawLine(0, 0, 0, height());
	//p.drawPrimitive(QStyle::PE_FrameTabBarBase, o);
	//p.drawPrimitive(QStyle::PE_Frame, o);
	//p.drawPrimitive(QStyle::PE_FrameButtonBevel, o);
	p.drawPrimitive(QStyle::PE_FrameButtonTool, o);
	for (int i = 0; i < 26; i++) {
		QChar qc(i + 65);
		QRect r(0, height() * i / 26, 19, height() / 26);
		if (_libraryTreeView->currentLetter() == qc) {
			p.fillRect(r, o.palette.highlight());
		}
		p.save();
		p.setBrush(o.palette.color(QPalette::WindowText));
		p.drawText(r, Qt::AlignCenter, qc);
		p.restore();
	}
}
