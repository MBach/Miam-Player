#include "libraryheader.h"

#include "settingsprivate.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QLinearGradient>
#include <QStylePainter>

#include <QtDebug>

LibraryHeader::LibraryHeader(QWidget *parent) :
	QPushButton(parent), _order(Qt::AscendingOrder), _uncheck(false), libraryOrderDialog(new LibraryOrderDialog(this))
{
	connect(this, &QPushButton::clicked, [=]() {
		if (_order == Qt::AscendingOrder) {
			_order = Qt::DescendingOrder;
		} else {
			_order = Qt::AscendingOrder;
		}
		this->update();
		emit aboutToChangeSortOrder();
	});
	connect(libraryOrderDialog, &LibraryOrderDialog::accepted, this, &LibraryHeader::aboutToChangeHierarchyOrder);
	libraryOrderDialog->installEventFilter(this);
}

bool LibraryHeader::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == libraryOrderDialog && event->type() == QEvent::Close) {
		_uncheck = true;
	}
	return QPushButton::eventFilter(obj, event);
}

void LibraryHeader::contextMenuEvent(QContextMenuEvent *e)
{
	libraryOrderDialog->move(mapToGlobal(e->pos()));
	libraryOrderDialog->show();
}

void LibraryHeader::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QColor base = QApplication::palette().base().color();

	// Gradient
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	if (SettingsPrivate::instance()->isCustomColors()) {
		g.setColorAt(0, base.lighter(110));
		g.setColorAt(1, base);
	} else {
		g.setColorAt(0, base);
		g.setColorAt(1, QApplication::palette().window().color());
	}
	p.fillRect(rect(), g);

	// Text
	QString header = libraryOrderDialog->headerValue();
	QFont f = SettingsPrivate::instance()->font(SettingsPrivate::FF_Library);
	p.setFont(f);
	QFontMetrics fm(f);
	this->setMinimumHeight(fm.height());

	// Check if label should be elided (in case of large font and small area on screen)
	QString elided = fm.elidedText(header, Qt::ElideRight, parentWidget()->width());
	p.setPen(QApplication::palette().windowText().color());
	p.drawText(rect().adjusted(10, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, elided);

	// Draw a thin line before the treeview
	p.save();
	p.setPen(base.darker(115));
	p.drawLine(rect().bottomLeft(), rect().bottomRight());
	p.restore();

	// Draw sort indicator
	static const QPointF sortIndicatorDown[3] = {
		QPointF(0.0, 0.0),
		QPointF(2.0, 0.0),
		QPointF(1.0, 1.0)
	};
	static const QPointF sortIndicatorUp[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(2.0, 1.0)
	};

	int minX = rect().center().x() - (rect().height() / 2);
	if (fm.width(elided) > minX - 20) {
		/// XXX: improve this
		minX = fm.width(header) + 20;
	}
	p.save();
	p.translate(minX, rect().height() / 2.5);

	// Highlight the sort indicator if needed
	if (rect().contains(mapFromGlobal(QCursor::pos()))) {
		p.setPen(QApplication::palette().highlight().color());
		p.setBrush(QApplication::palette().highlight().color().lighter());
	} else {
		p.setPen(QApplication::palette().mid().color());
		p.setBrush(Qt::NoBrush);
	}
	QTransform t;
	float ratio = (float) rect().height() / 6.0;
	t.scale(ratio, ratio);
	QPolygonF sortIndicator;
	if (Qt::DescendingOrder == _order) {
		sortIndicator.append(t.map(sortIndicatorDown[0]));
		sortIndicator.append(t.map(sortIndicatorDown[1]));
		sortIndicator.append(t.map(sortIndicatorDown[2]));
	} else {
		sortIndicator.append(t.map(sortIndicatorUp[0]));
		sortIndicator.append(t.map(sortIndicatorUp[1]));
		sortIndicator.append(t.map(sortIndicatorUp[2]));
	}
	p.drawPolygon(sortIndicator);
	p.restore();
}

void LibraryHeader::showDialog(bool enabled)
{
	QPushButton *b = qobject_cast<QPushButton*>(sender());
	if (b) {
		libraryOrderDialog->move(mapToGlobal(b->frameGeometry().topLeft()));
	}
	if (_uncheck) {
		libraryOrderDialog->setVisible(true);
		_uncheck = false;
	} else {
		libraryOrderDialog->setVisible(enabled);
	}
}
