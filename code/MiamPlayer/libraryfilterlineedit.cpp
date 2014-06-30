#include "libraryfilterlineedit.h"

#include "settings.h"
#include <QAction>
#include <QApplication>
#include <QRect>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
	LineEdit(parent), shortcut(new QShortcut(this))
{
	connect(Settings::getInstance(), &Settings::fontHasChanged, [=](Settings::FontFamily ff, const QFont &newFont) {
		if (ff == Settings::FF_Library) {
			this->setFont(newFont);
			this->setMinimumHeight(fontMetrics().height() * 1.6);
		}
	});

	// Add the possibility to grab focus if it's in library, tag editor, or a playlist
	// Do not work if widget is hidden (splitter or 2nd tab: file explorer)
	shortcut->setContext(Qt::ApplicationShortcut);
	connect(shortcut, &QShortcut::activated, this, [=]() {
		this->setFocus(Qt::ShortcutFocusReason);
	});
}

void LibraryFilterLineEdit::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QStyleOptionFrame o;
	initStyleOption(&o);
	o.palette = QApplication::palette();
	o.rect.adjust(10, 10, -10, -15);

	p.fillRect(rect(), o.palette.base().color().lighter(110));

	static const int startAngle = 90 * 16;
	static const int spanAngle = 180 * 16;
	QRect rLeft = QRect(o.rect.x(),
						o.rect.y() + 1,
						o.rect.height(),
						o.rect.y() + o.rect.height() - 2);
	QRect rRight = QRect(o.rect.x() + o.rect.width() - o.rect.height(),
						 o.rect.y() + 1,
						 o.rect.height(),
						 o.rect.y() + o.rect.height() - 2);
	QRect rText = QRect(rLeft.topRight(), rRight.bottomLeft()).adjusted(0, 1, 0, -1);

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

	// Paint text and cursor
	if (hasFocus() || !text().isEmpty()) {

		// Highlight selected text
		p.setPen(o.palette.text().color());
		/*if (hasSelectedText()) {

			QRect rectTextLeft, rectTextMid, rectTextRight;
			QString leftText, midText, rightText;
			int sStart = selectionStart();
			int sEnd = selectedText().length() - 1;

			if (sStart == 0) {
				midText = rText;
				rectTextMid = rText;
				rectTextMid.setWidth(fontMetrics().width(midText));
			} else if (sEnd == text().length() - 1) {

			} else {
				leftText = text().left(sStart);
				rectTextLeft = rText;
				rectTextLeft.setWidth(fontMetrics().width(leftText));
			}

			p.drawText(rectTextLeft, Qt::AlignLeft | Qt::AlignVCenter, leftText);

			p.fillRect(rectTextMid, o.palette.highlight());
			p.setPen(o.palette.highlightedText().color());
			p.drawText(rectTextMid, Qt::AlignLeft | Qt::AlignVCenter, midText);

			p.setPen(o.palette.text().color());
			p.drawText(rectTextRight, Qt::AlignLeft | Qt::AlignVCenter, rightText);
		} else {
			p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, text());
		}*/
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, text());
		this->drawCursor(&p, rText);
	} else {
		p.setPen(o.palette.mid().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, placeholderText());
	}

	// Border of this widget
	p.setPen(o.palette.mid().color());
	if (QApplication::isLeftToRight()) {
		p.drawLine(QPoint(rect().center().x(), 0), rect().topRight());
		p.drawLine(rect().topRight(), rect().bottomRight());
	} else {
		p.drawLine(QPoint(rect().center().x() - 1, 0), rect().topLeft());
		p.drawLine(rect().topLeft(), rect().bottomLeft());
	}
}


