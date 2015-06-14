#include "libraryfilterlineedit.h"

#include "settingsprivate.h"
#include <QAction>
#include <QApplication>
#include <QRect>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
	LineEdit(parent), shortcut(new QShortcut(this))
{
	connect(SettingsPrivate::instance(), &SettingsPrivate::fontHasChanged, [=](SettingsPrivate::FontFamily ff, const QFont &newFont) {
		if (ff == SettingsPrivate::FF_Library) {
			this->setFont(newFont);
			//this->setMinimumHeight(fontMetrics().height() * 1.6);
		}
	});

	// Add the possibility to grab focus if it's in library, tag editor, or a playlist
	// Do not work if widget is hidden (splitter or 2nd tab: file explorer)
	shortcut->setContext(Qt::ApplicationShortcut);
	connect(shortcut, &QShortcut::activated, this, [=]() {
		this->setFocus(Qt::ShortcutFocusReason);
	});

	// Do not start search when one is typing. Add a 300ms delay after the last key pressed.
	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(this, &QLineEdit::textEdited, this, [=]() {	timer->start(300); });
	connect(timer, &QTimer::timeout, this, [=]() { emit aboutToStartSearch(this->text()); });
}

void LibraryFilterLineEdit::focusInEvent(QFocusEvent *event)
{
	LineEdit::focusInEvent(event);
	// Notify registered objets that one has clicked inside this widget.
	// Useful to redisplay search results dialog
	if (!text().isEmpty()) {
		emit focusIn();
	}
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

	/// FIXME
	/// Use a path then fill it instead of using drawArc
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
		this->drawCursor(&p, rText);
	} else {
		p.setPen(o.palette.mid().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, placeholderText());
	}
}


