#include "libraryscrollbar.h"

#include <QLabel>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QVBoxLayout>

#include <QtDebug>

#include "librarytreeview.h"

LibraryScrollBar::LibraryScrollBar(LibraryTreeView *parent) :
	QScrollBar(parent), _libraryTreeView(parent), _hasNotEmittedYet(true)
{
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	for (int i = 0; i < 26; i++) {
		QLabel *label = new QLabel(this);
		char c = static_cast<char>(i + 65);
		QChar qc(c);
		QString letter("<a href=%1 style=\"text-decoration:none;\">%2</a>");
		letter = letter.arg(qc, qc);
		label->setText(letter);
		label->setAlignment(Qt::AlignCenter);
		label->setFrameStyle(QFrame::Box);
		label->setMinimumWidth(QScrollBar::sizeHint().width());

		vLayout->addWidget(label);
		connect(label, &QLabel::linkActivated, this, &LibraryScrollBar::jumpTo);
	}
	vLayout->setContentsMargins(0, 0, 0, 0);
	w = new QWidget(_libraryTreeView);
	w->setLayout(vLayout);
	w->setMaximumWidth(QScrollBar::sizeHint().width());
	vLayout->setSpacing(0);
}

void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		qDebug() << "hide covers when moving";
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mouseMoveEvent(e);
}

void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_hasNotEmittedYet) {
		qDebug() << "show covers when stopped moving";
		emit displayItemDelegate(true);
		_hasNotEmittedYet = true;
	}
	QScrollBar::mouseReleaseEvent(e);
}


void LibraryScrollBar::paintEvent(QPaintEvent *)
{
	qDebug() << Q_FUNC_INFO;

	QStylePainter pa(this);
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	w->move(_libraryTreeView->frameGeometry().right() - 2 * width(), 0);
	opt.subControls = QStyle::SC_All;
	pa.drawComplexControl(QStyle::CC_ScrollBar, opt);

	if (_letter.isValid()) {
		qDebug() << "need to highlight" << _letter.data().toString();
		//_libraryTreeView->letters().value(_letter)
		/// XXX: optimize with object name
		foreach (QLabel *l, w->findChildren<QLabel*>()) {
			QPalette p = l->palette();
			if (l->text().contains(_letter.data().toString())) {
				p.setBrush(QPalette::Text, p.highlight());
				l->setPalette(p);
				break;
			} else {
				l->setPalette(p);
			}
		}
	}
}

void LibraryScrollBar::jumpTo(const QString &letter)
{
	QStandardItem *item = _libraryTreeView->letters().value(letter);
	if (item) {
		LibraryFilterProxyModel *proxy = qobject_cast<LibraryFilterProxyModel*>(_libraryTreeView->model());
		QModelIndex index = proxy->mapFromSource(item->index());
		_libraryTreeView->scrollTo(index, QAbstractItemView::PositionAtTop);
	}
}
