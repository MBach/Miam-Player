#include "libraryfilterlineedit.h"

#include "settingsprivate.h"
#include <QAction>
#include <QApplication>
#include <QRect>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent)
	: LineEdit(parent)
	, shortcut(new QShortcut(this))
{
	connect(SettingsPrivate::instance(), &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff, const QFont &newFont) {
		if (ff == SettingsPrivate::FF_Library) {
			this->setFont(newFont);
		}
	});

	// Add the possibility to grab focus if it's in library, tag editor, or a playlist
	// Do not work if widget is hidden (splitter or 2nd tab: file explorer)
	shortcut->setContext(Qt::ApplicationShortcut);
	connect(shortcut, &QShortcut::activated, this, [=]() {
		this->setFocus(Qt::ShortcutFocusReason);
	});

	this->setMouseTracking(true);
}

LibraryFilterLineEdit::~LibraryFilterLineEdit()
{
	this->disconnect();
}

void LibraryFilterLineEdit::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QStyleOptionFrame o;
	initStyleOption(&o);
	o.palette = QApplication::palette();
	//o.rect.adjust(10, 10, -10, -15);
	//qDebug() << o.rect.height() << p.fontMetrics().height();
	//o.rect.setHeight(p.fontMetrics().height());

	p.fillRect(rect(), o.palette.base().color());
	QRect rLeft = QRect(o.rect.x(),
						o.rect.y() + 1,
						o.rect.height(),
						o.rect.y() + o.rect.height() - 2);
	QRect rRight = QRect(o.rect.x() + o.rect.width() - o.rect.height(),
						 o.rect.y() + 1,
						 o.rect.height(),
						 o.rect.y() + o.rect.height() - 2);
	QRect rText = QRect(rLeft.topRight(), rRight.bottomLeft()).adjusted(0, 1, 0, -1);

	if (hasFocus()) {
		p.setPen(QPen(o.palette.highlight().color()));
	} else {
		p.setPen(QPen(o.palette.mid().color()));
	}
	p.save();

	p.setRenderHint(QPainter::Antialiasing, true);
	QPainterPath ppLeft, ppRight;
	// 2---1---->   Left curve is painted with 2 calls to cubicTo, starting in 1   <----10--9
	// |   |        First cubic call is with points p1, p2 and p3                       |   |
	// 3   +        Second is with points p3, p4 and p5                                 +   8
	// |   |        With that, a half circle can be filled with linear gradient         |   |
	// 4---5---->                                                                  <----6---7
	ppLeft.moveTo(rText.topLeft());
	ppLeft.cubicTo(rText.x(), rText.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y(),
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f);
	ppLeft.cubicTo(rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height() / 2.0f,
				   rLeft.x() + rLeft.width() / 2.0f, rLeft.y() + rLeft.height(),
				   rLeft.x() + rLeft.width(), rLeft.y() + rLeft.height());

	QPainterPath pp(ppLeft);

	ppRight.moveTo(rRight.bottomLeft());
	ppRight.cubicTo(rRight.x(), rRight.y() + rRight.height(),
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height(),
					rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f);
	ppRight.cubicTo(rRight.x() + rRight.width() / 2.0f, rRight.y() + rRight.height() / 2.0f,
					rRight.x() + rRight.width() / 2.0f, rRight.y(),
					rRight.x(), rRight.y());

	pp.connectPath(ppRight);

	p.drawPath(ppLeft);
	p.drawPath(ppRight);

	p.setRenderHint(QPainter::Antialiasing, false);
	p.drawLine(QPoint(rText.x(), rText.y() - 1), QPoint(rText.x() + rText.width(), rText.y() - 1));
	p.drawLine(QPoint(rText.x(), rText.y() + rText.height()), QPoint(rText.x() + rText.width(), rText.y() + rText.height()));
	p.restore();

	// Paint text and cursor
	if (hasFocus() || !text().isEmpty()) {

		// Highlight selected text
		p.setPen(o.palette.text().color());
		if (hasSelectedText()) {

			QRect rectTextLeft, rectTextMid, rectTextRight;
			QString leftText, midText, rightText;

			int sStart = selectionStart();
			int sEnd = selectedText().length() - 1;

			// Four uses cases to highlight a text
			if (sStart > 0 && sEnd < text().size() - 1) {
				// a[b]cd
				leftText = text().left(sStart);
				midText = selectedText();
				rightText = text().mid(sStart + selectedText().length());

				rectTextLeft.setX(rText.x());
				rectTextLeft.setY(rText.y());
				rectTextLeft.setWidth(fontMetrics().width(leftText));
				rectTextLeft.setHeight(rText.height());

				rectTextMid.setX(rectTextLeft.x() + rectTextLeft.width());
				rectTextMid.setY(rText.y());
				rectTextMid.setWidth(fontMetrics().width(midText));
				rectTextMid.setHeight(rText.height());

				rectTextRight.setX(rectTextMid.x() + rectTextMid.width());
				rectTextRight.setY(rText.y());
				rectTextRight.setWidth(fontMetrics().width(rightText));
				rectTextRight.setHeight(rText.height());

				p.fillRect(rectTextLeft, o.palette.base());
				p.fillRect(rectTextMid, o.palette.highlight());
				p.fillRect(rectTextRight, o.palette.base());

				p.drawText(rectTextLeft, Qt::AlignLeft | Qt::AlignVCenter, leftText);
				p.save();
				p.setPen(o.palette.highlightedText().color());
				p.drawText(rectTextMid, Qt::AlignLeft | Qt::AlignVCenter, midText);
				p.restore();
				p.drawText(rectTextRight, Qt::AlignLeft | Qt::AlignVCenter, rightText);
			} else if (sStart == 0 && sEnd < text().size() - 1) {
				// [ab]cd
				midText = selectedText();
				rightText = text().mid(sStart + selectedText().length());

				rectTextMid.setX(rText.x());
				rectTextMid.setY(rText.y());
				rectTextMid.setWidth(fontMetrics().width(midText));
				rectTextMid.setHeight(rText.height());

				rectTextRight.setX(rectTextMid.x() + rectTextMid.width());
				rectTextRight.setY(rText.y());
				rectTextRight.setWidth(fontMetrics().width(rightText));
				rectTextRight.setHeight(rText.height());

				p.fillRect(rectTextMid, o.palette.highlight());
				p.fillRect(rectTextRight, o.palette.base());

				p.save();
				p.setPen(o.palette.highlightedText().color());
				p.drawText(rectTextMid, Qt::AlignLeft | Qt::AlignVCenter, midText);
				p.restore();
				p.drawText(rectTextRight, Qt::AlignLeft | Qt::AlignVCenter, rightText);
			} else if (sStart == 0 && sEnd == text().size() - 1) {
				// [abcd]
				midText = selectedText();

				rectTextMid.setX(rText.x());
				rectTextMid.setY(rText.y());
				rectTextMid.setWidth(fontMetrics().width(midText));
				rectTextMid.setHeight(rText.height());

				p.fillRect(rectTextMid, o.palette.highlight());

				p.save();
				p.setPen(o.palette.highlightedText().color());
				p.drawText(rectTextMid, Qt::AlignLeft | Qt::AlignVCenter, midText);
				p.restore();
			} else {
				// ab[cd]
				// never?
			}
		} else {
			p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, text());
		}
		rText.adjust(0, 2, 0, -2);
		this->drawCursor(&p, rText);
	} else {
		p.setPen(o.palette.mid().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, placeholderText());
	}
}
