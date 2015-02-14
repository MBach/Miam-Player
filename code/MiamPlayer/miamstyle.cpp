#include "miamstyle.h"

#include <QApplication>
#include <QScrollBar>
#include <QStyleOptionGroupBox>
#include <QStyleOptionSlider>
#include <QPainter>

#include <QtDebug>

MiamStyle::MiamStyle(QStyle *parent) :
	QProxyStyle(parent)
{
}

/// XXX
void MiamStyle::drawScrollBar(QPainter *p, const QWidget *widget) const
{
	QStyleOptionSlider scrollbar;
	scrollbar.palette = QApplication::palette();

	const QScrollBar *sc = qobject_cast<const QScrollBar *>(widget);
	scrollbar.initFrom(sc);


	QRect subLineRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSubLine, sc);
	QRect addLineRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarAddLine, sc);
	QRect sliderRect = QApplication::style()->subControlRect(QStyle::CC_ScrollBar, &scrollbar, QStyle::SC_ScrollBarSlider, sc);

	//qDebug() << subLineRect << sliderRect << addLineRect;


	if (sc->orientation() == Qt::Vertical) {
		subLineRect.adjust(0, 0, -1, 0);
		addLineRect.adjust(0, 0, -1, 0);
		sliderRect.adjust(0, 0, -1, 0);
	} else {
		subLineRect.adjust(0, 0, 0, -1);
		addLineRect.adjust(0, 0, 0, -1);
		sliderRect.adjust(0, 0, 0, -1);
	}

	p->setPen(Qt::NoPen);
	p->setBrush(scrollbar.palette.window());
	p->drawRect(sc->rect());

	p->setBrush(scrollbar.palette.base().color().darker(125));
	p->drawRect(sliderRect);

	// Highlight
	p->save();
	QPoint pos = sc->mapFromGlobal(QCursor::pos());
	p->setPen(scrollbar.palette.highlight().color());

	if (!sc->isSliderDown()) {
		p->setBrush(scrollbar.palette.highlight().color().lighter());
		if (subLineRect.contains(pos)) {
			p->drawRect(subLineRect);
		} else if (sliderRect.contains(pos)) {
			p->drawRect(sliderRect);
		} else if (addLineRect.contains(pos)) {
			p->drawRect(addLineRect);
		}
	} else {
		p->setBrush(scrollbar.palette.highlight().color());
		//if (_isDown == 0) {
		//	p->drawRect(subLineRect);
		//} else if (_isDown == 1) {
		//	p->drawRect(sliderRect);
		//} else if (_isDown == 2) {
		//	p->drawRect(addLineRect);
		//}
	}
	p->restore();

	// Draw sort indicator
	static const QPointF upArrow[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(2.0, 1.0)
	};
	static const QPointF downArrow[3] = {
		QPointF(0.0, 0.0),
		QPointF(2.0, 0.0),
		QPointF(1.0, 1.0)
	};
	static const QPointF leftArrow[3] = {
		QPointF(0.0, 1.0),
		QPointF(1.0, 0.0),
		QPointF(1.0, 2.0)
	};
	static const QPointF rightArrow[3] = {
		QPointF(0.0, 0.0),
		QPointF(1.0, 1.0),
		QPointF(0.0, 2.0)
	};

	// Arrows
	p->save();
	if (scrollbar.palette.windowText().color().value() < 128) {
		p->setPen(scrollbar.palette.dark().color());
		p->setBrush(scrollbar.palette.dark());
	} else {
		p->setPen(scrollbar.palette.mid().color());
		p->setBrush(scrollbar.palette.mid());
	}

	QTransform t;
	float ratio = (float) subLineRect.height() / 4.0;
	t.scale(ratio, ratio);

	if (sc->orientation() == Qt::Vertical) {
		QPolygonF up, down;
		up.append(t.map(upArrow[0]));
		up.append(t.map(upArrow[1]));
		up.append(t.map(upArrow[2]));
		down.append(t.map(downArrow[0]));
		down.append(t.map(downArrow[1]));
		down.append(t.map(downArrow[2]));
		p->translate(subLineRect.width() / 4.0, subLineRect.height() / 3.0);
		p->drawPolygon(up);
		p->translate(0, addLineRect.y());
		p->drawPolygon(down);
	} else {
		QPolygonF left, right;
		left.append(t.map(leftArrow[0]));
		left.append(t.map(leftArrow[1]));
		left.append(t.map(leftArrow[2]));
		right.append(t.map(rightArrow[0]));
		right.append(t.map(rightArrow[1]));
		right.append(t.map(rightArrow[2]));
		p->translate(subLineRect.height() / 3.0, subLineRect.width() / 4.0);
		p->drawPolygon(left);
		p->translate(addLineRect.x(), 0);
		p->drawPolygon(right);
	}
	p->restore();
}

#include "tabplaylist.h"

QRect MiamStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
	QRect r = QProxyStyle::subElementRect(element, option, widget);


	/*QStyle::SE_TabWidgetLeftCorner	21	Area for the left corner widget in a tab widget.
	QStyle::SE_TabWidgetRightCorner	22	Area for the right corner widget in a tab widget.
	QStyle::SE_TabWidgetTabBar	18	Area for the tab bar widget in a tab widget.
	QStyle::SE_TabWidgetTabContents	20	Area for the contents of the tab widget.
	QStyle::SE_TabWidgetTabPane	19	Area for the pane of a tab widget.
	QStyle::SE_TabWidgetLayoutItem	?	Area that counts for the parent layout.
	*/
	if (widget && widget->objectName() == "tabPlaylists") {
		const TabPlaylist *t = dynamic_cast<const TabPlaylist*>(widget);
		QRect minRect = QRect(0, 0, 0, 0);
		for (int i = 0; i < t->tabBar()->count(); i++) {
			minRect.setWidth(minRect.width() + t->tabBar()->tabRect(i).width());
		}
		minRect.setHeight(t->tabBar()->tabRect(0).height());
		switch (element) {
		case SE_TabWidgetRightCorner:
			//qDebug() << r << "SE_TabWidgetRightCorner" << t->width();
			r.setX(minRect.x() + minRect.width() + 1);
			r.setWidth(t->cornerWidget()->width());
			r.setHeight(r.width());
			break;
		case SE_TabWidgetTabBar:
			//qDebug() << r << minRect << "SE_TabWidgetTabBar";
			r = minRect.adjusted(0, 0, 6, 0);
			break;
//		case SE_TabWidgetTabContents:
//			qDebug() << r << "SE_TabWidgetTabContents" << widget;
//			break;
//		case SE_TabWidgetTabPane:
//			qDebug() << r << "SE_TabWidgetTabPane" << widget;
//			break;
//		case SE_TabWidgetLayoutItem:
//			qDebug() << r << "SE_TabWidgetLayoutItem" << widget;
//			break;
		}
	}

	return r;
}

void MiamStyle::​drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	qDebug() << Q_FUNC_INFO << "here";
	/*QStyle::CE_MenuBarItem	20	A menu item in a QMenuBar.
	QStyle::CE_MenuBarEmptyArea	21	The empty area of a QMenuBar.
	QStyle::CE_MenuItem	14	A menu item in a QMenu.
	QStyle::CE_MenuScroller	15	Scrolling areas in a QMenu when the style supports scrolling.
	QStyle::CE_MenuTearoff	18	A menu item representing the tear off section of a QMenu.
	QStyle::CE_MenuEmptyArea	19	The area in a menu without menu items.
	QStyle::CE_MenuHMargin	17	The horizontal extra space on the left/right of a menu.
	QStyle::CE_MenuVMargin	16	The vertical extra space on the top/bottom of a menu.*/
	switch (element) {
	case CE_MenuItem: {
		const QStyleOptionMenuItem *somi = static_cast<const QStyleOptionMenuItem*>(option);
		//somi->font.setBold(true);
		painter->drawText(somi->rect, "test");
		qDebug() << Q_FUNC_INFO << "ici";
		break;
	}
	case CE_MenuBarItem:{
		qDebug() << Q_FUNC_INFO << "ici";
		break;
	}
	case CE_MenuBarEmptyArea:{
		qDebug() << Q_FUNC_INFO << "ici";
		break;
	}
	default:
		QProxyStyle::drawControl(element, option, painter, widget);
	}
}

void MiamStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *p, const QWidget *widget) const
{
	switch (control) {
	case CC_GroupBox:{
		const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
		QStyleOptionGroupBox gb2(*gb);
		gb2.textColor = QApplication::palette().windowText().color();
		QProxyStyle::drawComplexControl(control, &gb2, p, widget);
		break;
	}
	//case CC_ScrollBar:
	//	this->drawScrollBar(p, widget);
	//	break;
	default:
		QProxyStyle::drawComplexControl(control, option, p, widget);
	}
}

#include "settings.h"

void MiamStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *opt, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
	case PE_IndicatorTabClose: {
		if (opt->state.testFlag(State_MouseOver)) {
			painter->drawPixmap(0, 0, 16, 16, QPixmap(":/icons/win/closeTabsHover"));
		} else {
			painter->drawPixmap(0, 0, 16, 16, QPixmap(":/icons/closeTabs"));
		}
		break;
	}
	case PE_IndicatorBranch: {

		// Draw sort indicator
		static const QPointF downArrow[3] = { QPointF(0.0, 0.0), QPointF(2.0, 0.0), QPointF(1.0, 1.0) };
		static const QPointF leftArrow[3] = { QPointF(0.0, 1.0), QPointF(1.0, 0.0), QPointF(1.0, 2.0) };
		static const QPointF rightArrow[3] = { QPointF(0.0, 0.0), QPointF(1.0, 1.0), QPointF(0.0, 2.0) };

		QTransform t;
		float ratio = opt->rect.width() / 4.0;
		//qDebug() << "ratio" << ratio << opt->rect << opt->fontMetrics.height();
		t.scale(ratio, ratio);

		QPolygonF arrow;
		if (QGuiApplication::isLeftToRight()) {
			QPolygonF right;
			right.append(t.map(rightArrow[0]));
			right.append(t.map(rightArrow[1]));
			right.append(t.map(rightArrow[2]));
			arrow = right;
		} else {
			QPolygonF left;
			left.append(t.map(leftArrow[0]));
			left.append(t.map(leftArrow[1]));
			left.append(t.map(leftArrow[2]));
			arrow = left;
		}

		if (opt->state.testFlag(State_Children)) {
			painter->save();
			if (opt->state.testFlag(State_MouseOver)) {
				painter->setPen(opt->palette.highlight().color());
				painter->setBrush(opt->palette.highlight().color().lighter());
			} else {
				painter->setPen(opt->palette.mid().color());
				painter->setBrush(Qt::NoBrush);
			}
			if (opt->state.testFlag(State_Open)) {
				QPolygonF down;
				down.append(t.map(downArrow[0]));
				down.append(t.map(downArrow[1]));
				down.append(t.map(downArrow[2]));
				painter->translate(opt->rect.x() + opt->rect.width() / 2 - down.boundingRect().width() / 2,
								   opt->rect.y() + opt->rect.height() / 2 - down.boundingRect().height() / 2);
				painter->drawPolygon(down);
			} else {
				painter->translate(opt->rect.x() + opt->rect.width() / 2 - arrow.boundingRect().width() / 2,
								   opt->rect.y() + opt->rect.height() / 2 - arrow.boundingRect().height() / 2);
				painter->drawPolygon(arrow);
			}
			painter->restore();
		}
		break;
	}
	/*case PE_FrameMenu: {
		const QStyleOptionMenuItem *somi = static_cast<const QStyleOptionMenuItem*>(opt);
		//qDebug() << Q_FUNC_INFO << "PE_FrameMenu" << somi->rect << somi->font;
		break;
	}
	case PE_PanelMenuBar: {
		//qDebug() << Q_FUNC_INFO << "PE_PanelMenuBar";
		break;
	}
	case PE_PanelMenu: {
		//qDebug() << Q_FUNC_INFO << "PE_PanelMenu" << opt;
		QStyleOptionMenuItem mi;
		mi.init(widget);
		//qDebug() << Q_FUNC_INFO << "PE_FrameMenu" << mi.rect << mi.font;
		painter->drawRect(mi.rect);
		break;
	}*/
	default: {
		QProxyStyle::drawPrimitive(element, opt, painter, widget);
		break;
	}
	}
}
