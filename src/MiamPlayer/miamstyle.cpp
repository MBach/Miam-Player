#include "miamstyle.h"

#include <QApplication>
#include <QFileSelector>
#include <QScrollBar>
#include <QStyleOptionGroupBox>
#include <QStyleOptionSlider>
#include <QPainter>

#include "settingsprivate.h"
#include "tabplaylist.h"

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

QRect MiamStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
	QRect r = QProxyStyle::subElementRect(element, option, widget);
	if (widget && widget->objectName() == "tabPlaylists") {
		const TabPlaylist *t = dynamic_cast<const TabPlaylist*>(widget);
		QRect minRect = QRect(0, 0, 0, 0);
		for (int i = 0; i < t->tabBar()->count(); i++) {
			minRect.setWidth(minRect.width() + t->tabBar()->tabRect(i).width());
		}
		minRect.setHeight(t->tabBar()->tabRect(0).height());
		switch (element) {
		case SE_TabWidgetRightCorner:
			r.setX(minRect.x() + minRect.width() + 1);
			r.setY(0);
			break;
		case SE_TabWidgetTabBar:
			r = minRect.adjusted(0, 0, SettingsPrivate::instance()->tabsOverlappingLength(), 0);
			break;
		default:
			break;
		}
	}
	return r;
}

void MiamStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
	case CE_MenuBarItem:{
		const QStyleOptionMenuItem *somi = static_cast<const QStyleOptionMenuItem*>(option);
		const bool act = somi->state & (State_Sunken | State_Selected);
		QPalette palette = QApplication::palette();
		QBrush brush;
		if (act) {
			painter->setPen(palette.highlight().color());
			brush = palette.highlight().color().light();
			painter->setBrush(brush);
			painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
		} else {
			brush = palette.window();
			painter->fillRect(option->rect, palette.window());
		}

		uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
		if (!proxy()->styleHint(SH_UnderlineShortcut, somi, widget)) {
			alignment |= Qt::TextHideMnemonic;
		}
		if (qAbs(palette.text().color().value() - brush.color().value()) < 128) {
			painter->setPen(palette.highlightedText().color());
		} else {
			painter->setPen(palette.text().color());
		}
		painter->drawText(option->rect, alignment, somi->text);
		break;
	}
	case CE_MenuBarEmptyArea:{
		painter->fillRect(option->rect, QApplication::palette().window());
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

void MiamStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *opt, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
	// On Linux, don't fill the rectangular area where lives the PE_IndicatorBranch. Keep this area transparent
	// On Windows, this is the default behaviour
	case PE_PanelItemViewRow: {
		if (widget && widget->inherits("Playlist")) {
			QProxyStyle::drawPrimitive(element, opt, painter, widget);
		}
		break;
	}
	case PE_IndicatorTabClose: {
		if (opt->state.testFlag(State_MouseOver)) {
			QFileSelector fs;
			painter->drawPixmap(0, 0, 16, 16, QPixmap(fs.select(":/icons/config/close_tabs_hover.png")));
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
	case PE_PanelLineEdit: {
		QPen pen = opt->palette.window().color();
		QBrush brush = opt->palette.base();
		painter->setPen(pen);
		painter->setBrush(brush);
		painter->drawRect(widget->rect().adjusted(0, 0, -1, -1));
		break;
	}
	case PE_FrameGroupBox: {
		QPen pen = opt->palette.window().color();
		QBrush brush = opt->palette.base();
		painter->setPen(pen);
		painter->setBrush(brush);
		//painter->drawRect(widget->rect());
		QProxyStyle::drawPrimitive(element, opt, painter, widget);

		break;
	}
	case PE_FrameStatusBarItem:
	case PE_FrameButtonTool:
	case PE_FrameButtonBevel:
	case PE_FrameFocusRect:
		break;
	case PE_FrameMenu: {
		QPen pen = opt->palette.mid().color();
		QBrush brush = opt->palette.window();
		painter->setPen(pen);
		painter->setBrush(brush);
		painter->drawRect(widget->rect().adjusted(0, 0, -1, -1));
		break;
	}
	case PE_FrameTabBarBase:
		if (widget) {
			painter->fillRect(widget->rect(), QApplication::palette().window());
		} else {
			painter->fillRect(opt->rect, QApplication::palette().window());
		}
		break;
	default: {
		QProxyStyle::drawPrimitive(element, opt, painter, widget);
		break;
	}
	}
}
