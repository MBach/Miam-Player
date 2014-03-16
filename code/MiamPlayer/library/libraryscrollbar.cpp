#include "libraryscrollbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>

#include <QtDebug>

LibraryScrollBar::LibraryScrollBar(QWidget *parent)
	: QScrollBar(parent), _hasNotEmittedYet(true)
{}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mouseMoveEvent(e);
}

/** Redefined to temporarily hide covers when moving. */
void LibraryScrollBar::mousePressEvent(QMouseEvent *e)
{
	if (_hasNotEmittedYet) {
		emit displayItemDelegate(false);
		_hasNotEmittedYet = false;
	}
	QScrollBar::mousePressEvent(e);
}

/** Redefined to restore covers when move events are finished. */
void LibraryScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_hasNotEmittedYet) {
		emit displayItemDelegate(true);
		_hasNotEmittedYet = true;
	}
	QScrollBar::mouseReleaseEvent(e);
}

void LibraryScrollBar::paintEvent(QPaintEvent *e)
{
	QScrollBar::paintEvent(e);
	QStylePainter p(this);
	/*QStyleOptionSlider *scrollbar = new QStyleOptionSlider();
	initStyleOption(scrollbar);
	scrollbar->subControls |= QStyle::SC_ScrollBarSubLine;
	scrollbar->subControls |= QStyle::SC_ScrollBarAddLine;

	QStyleOptionSlider newScrollbar = *scrollbar;
	newScrollbar.palette = QApplication::palette();

	QStyle::State saveFlags = scrollbar->state;
	qDebug() << Q_FUNC_INFO << scrollbar->subControls;
	if (scrollbar->subControls & QStyle::SC_ScrollBarSubLine) {
		qDebug() << "has SC_ScrollBarSubLine";
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarSubLine, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarSubLine))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarSubLine, newScrollbar);
			qDebug() << "CE_ScrollBarSubLine";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarAddLine) {
		qDebug() << "has SC_ScrollBarAddLine";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarAddLine, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarAddLine))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarAddLine, newScrollbar);
			qDebug() << "CE_ScrollBarAddLine";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarSubPage) {
		qDebug() << "has SC_ScrollBarSubPage";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarSubPage, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarSubPage))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarSubPage, newScrollbar);
			qDebug() << "CE_ScrollBarSubPage";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarAddPage) {
		qDebug() << "has SC_ScrollBarAddPage";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarAddPage, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarAddPage))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarAddPage, newScrollbar);
			qDebug() << "CE_ScrollBarAddPage";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarFirst) {
		qDebug() << "has SC_ScrollBarFirst";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarFirst, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarFirst))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarFirst, newScrollbar);
			qDebug() << "CE_ScrollBarFirst";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarLast) {
		qDebug() << "has SC_ScrollBarLast";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarLast, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarLast))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarLast, newScrollbar);
			qDebug() << "CE_ScrollBarLast";
		}
	}
	if (scrollbar->subControls & QStyle::SC_ScrollBarSlider) {
		qDebug() << "has SC_ScrollBarSlider";
		newScrollbar.rect = scrollbar->rect;
		newScrollbar.state = saveFlags;
		newScrollbar.rect = style()->subControlRect(QStyle::CC_ScrollBar, &newScrollbar, QStyle::SC_ScrollBarSlider, this);
		if (newScrollbar.rect.isValid()) {
			if (!(scrollbar->activeSubControls & QStyle::SC_ScrollBarSlider))
				newScrollbar.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
			p.drawControl(QStyle::CE_ScrollBarSlider, newScrollbar);
			qDebug() << "CE_ScrollBarSlider";
			if (scrollbar->state & QStyle::State_HasFocus) {
				QStyleOptionFocusRect fropt;
				fropt.QStyleOption::operator=(newScrollbar);
				fropt.rect.setRect(newScrollbar.rect.x() + 2, newScrollbar.rect.y() + 2,
								   newScrollbar.rect.width() - 5,
								   newScrollbar.rect.height() - 5);
				p.drawPrimitive(QStyle::PE_FrameFocusRect, fropt);
				qDebug() << "PE_FrameFocusRect";
			}
		}
	}*/
	p.setPen(QApplication::palette().mid().color());
	if (isLeftToRight()) {
		p.drawLine(rect().topRight(), rect().bottomRight());
	} else {
		p.drawLine(rect().topLeft(), rect().bottomLeft());
	}
}
