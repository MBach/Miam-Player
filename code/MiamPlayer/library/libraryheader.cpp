#include "libraryheader.h"

#include "settings.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QLinearGradient>
#include <QStylePainter>

#include <QtDebug>

LibraryHeader::LibraryHeader(QWidget *parent) :
	QPushButton(parent), _lod(new LibraryOrderDialog(this))
{
	connect(this, &QPushButton::clicked, this, &LibraryHeader::aboutToChangeSortOrder);
	connect(_lod, &LibraryOrderDialog::accepted, this, &LibraryHeader::aboutToChangeHierarchyOrder);
}

void LibraryHeader::contextMenuEvent(QContextMenuEvent *e)
{
	_lod->move(mapToGlobal(e->pos()));
	_lod->show();
}

void LibraryHeader::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QColor base = QApplication::palette().base().color();

	// Gradient
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
    if (Settings::getInstance()->isCustomColors()) {
        g.setColorAt(0, base.lighter(110));
        g.setColorAt(1, base);
    } else {
        g.setColorAt(0, base);
        g.setColorAt(1, QApplication::palette().window().color());
    }
	p.fillRect(rect(), g);

	// Text
	//model()->headerData(logicalIndex, Qt::Horizontal).toString();
	QString header = _lod->headerValue();
	QFont f = Settings::getInstance()->font(Settings::LIBRARY);
	p.setFont(f);
	QFontMetrics fm(f);
	if (fm.height() > height()) {
		this->setMinimumHeight(fm.height());
		return;
	}

	// Check if label should be elided (in case of large font and small area on screen)
	QString elided = fm.elidedText(header, Qt::ElideRight, parentWidget()->width());
	p.drawText(rect().adjusted(10, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, elided);

	// Draw a thin line before the treeview
	p.save();
	p.setPen(base.darker(115));
	p.drawLine(rect().bottomLeft(), rect().bottomRight());
	p.restore();

	// Draw sort indicator
	static const QPoint sortIndicatorDown[3] = {
		QPoint(0.0, 0.0),
		QPoint(2.0, 0.0),
		QPoint(1.0, 1.0)
	};
	static const QPoint sortIndicatorUp[3] = {
		QPoint(0.0, 1.0),
		QPoint(1.0, 0.0),
		QPoint(2.0, 1.0)
	};

	int minX = rect().center().x() - (rect().height() / 2);
	if (fm.width(elided) > minX - 20) {
		/// XXX: improve this
		minX = fm.width(header) + 20;
	}
	p.save();
	p.translate(minX, rect().height() / 2.5);
	p.scale(rect().height() / 6, rect().height() / 6);

	// Highlight the sort indicator if needed
	if (rect().contains(mapFromGlobal(QCursor::pos()))) {
		p.setPen(QPen(QApplication::palette().highlight(), (6.0 / rect().height())));
		p.setBrush(QApplication::palette().highlight().color().lighter());
	} else {
		p.setPen(QPen(QApplication::palette().mid(), (6.0 / rect().height())));
		p.setBrush(Qt::NoBrush);
	}
	//if (Qt::AscendingOrder == sortIndicatorOrder()) {
	if (1) {
		p.drawPolygon(sortIndicatorDown, 3);
	} else {
		p.drawPolygon(sortIndicatorUp, 3);
	}
	p.restore();

	// Border
	p.setPen(QApplication::palette().mid().color());
	//p.fillRect(rect(), base);
	if (isLeftToRight()) {
		p.drawLine(rect().topRight(), rect().bottomRight());
	} else {
		p.drawLine(rect().topLeft(), rect().bottomLeft());
	}
}
