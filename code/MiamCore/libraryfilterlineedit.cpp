#include "libraryfilterlineedit.h"

#include "settings.h"
#include <QApplication>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
	QLineEdit(parent)
{
	// Remove text when clicked
	this->setClearButtonEnabled(true);
	connect(this, &QLineEdit::cursorPositionChanged, [=](int, int newP){
		if (newP == 0) {
			emit textEdited(QString());
		}
	});
	this->setMinimumHeight(QFontMetrics(Settings::getInstance()->font(Settings::LIBRARY)).height() * 2);
}

void LibraryFilterLineEdit::paintEvent(QPaintEvent *)
{
	this->setMinimumHeight(QFontMetrics(Settings::getInstance()->font(Settings::LIBRARY)).height() * 2);

	QStylePainter p(this);
	QStyleOption o;
	o.initFrom(this);
	o.palette = QApplication::palette();
	o.rect.adjust(10, 10, -20, -20);

	// Border of this widget
	p.setPen(o.palette.mid().color());
	if (QApplication::isLeftToRight()) {
		p.drawLine(rect().topRight(), rect().bottomRight());
	} else {
		p.drawLine(rect().topLeft(), rect().bottomLeft());
	}

	p.fillRect(rect(), o.palette.base());

	int startAngle = 90 * 16;
	int spanAngle = 180 * 16;
	QRect rLeft = QRect(o.rect.x(),
						o.rect.y() + 1,
						o.rect.height(),
						o.rect.y() + o.rect.height() - 2);
	QRect rRight = QRect(o.rect.x() + o.rect.width() - o.rect.height(),
						 o.rect.y() + 1,
						 o.rect.height(),
						 o.rect.y() + o.rect.height() - 2);

	p.save();
	if (o.state.testFlag(QStyle::State_HasFocus)) {
		p.setPen(o.palette.highlight().color());
	} else {
		p.setPen(o.palette.mid().color());
	}
	p.setRenderHint(QPainter::Antialiasing, true);
	p.drawArc(rLeft, startAngle, spanAngle);
	p.drawArc(rRight, (180 * 16) + startAngle, spanAngle);
	p.setRenderHint(QPainter::Antialiasing, false);
	p.drawLine(QPoint(rLeft.center().x(), rLeft.y() - 1), QPoint(rRight.center().x(), rRight.y() - 1));
	p.drawLine(QPoint(rLeft.center().x(), rLeft.bottom() + 1), QPoint(rRight.center().x(), rRight.bottom() + 1));
	p.restore();
	if (o.state.testFlag(QStyle::State_HasFocus) && !text().isEmpty()) {
		p.setPen(Qt::black);
		p.drawText(o.rect.adjusted(5, 0, 0, 0), text());
		p.drawLine(cursorRect().topRight(), cursorRect().bottomRight());
	} else {
		p.setPen(o.palette.mid().color());
		p.drawText(o.rect.adjusted(5, 0, 0, 0), placeholderText());
	}
}
