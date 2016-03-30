#include "miamstyle.h"

#include <QApplication>
#include <QFileSelector>
#include <QGuiApplication>
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
	if (scrollbar.palette.text().color().value() < 128) {
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
		case SE_TabWidgetTabBar: {
			SettingsPrivate *settings = SettingsPrivate::instance();
			if (!settings->isRectTabs()) {
				r = minRect.adjusted(0, 0, settings->tabsOverlappingLength(), 0);
			} else {
				r = minRect;
			}
			break;
		}
		default:
			break;
		}
	} else {
		if (widget && widget->objectName() == "tabBar") {
			const QStyleOptionTab *sot = qstyleoption_cast<const QStyleOptionTab*>(option);
			QStyleOptionTab tab = *sot;
			//qDebug() << Q_FUNC_INFO << element << r;
			if (element == SE_TabBarTabText) {
				//tab.rect.setX(tab.rect.x() + 20);
				//qDebug() << "SE_TabBarTabText" << sot->rect;

				return sot->rect;
			} else if (element == SE_CheckBoxContents) {
				//qDebug() << "SE_CheckBoxContents" << sot->rect;
				return sot->rect;
			}
		}
	}
	return r;
}

void MiamStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	switch (element) {
#if !defined(Q_OS_OSX)
	case CE_Splitter:
		break;
#endif
	case CE_MenuBarItem:{
		const QStyleOptionMenuItem *somi = static_cast<const QStyleOptionMenuItem*>(option);
		const bool act = somi->state & (State_Sunken | State_Selected);
		QPalette palette = QApplication::palette();
		QBrush brush;
		if (act) {
			painter->setPen(palette.highlight().color());
			brush = palette.highlight().color().lighter();
			painter->setBrush(brush);
			painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
		} else {
			brush = palette.window();
			painter->fillRect(option->rect, palette.window());
		}

		uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
		if (!styleHint(SH_UnderlineShortcut, somi, widget)) {
			alignment |= Qt::TextHideMnemonic;
		}
		if (somi->state.testFlag(QStyle::State_Enabled)) {
			if (act && SettingsPrivate::instance()->isCustomTextColorOverriden()) {
				painter->setPen(palette.highlightedText().color());
			} else if (qAbs(palette.text().color().value() - brush.color().value()) < 128) {
				painter->setPen(palette.highlightedText().color());
			} else {
				painter->setPen(palette.text().color());
			}
		} /*else if (act) {
			painter->setPen(palette.mid().color());
		}*/
		painter->drawText(option->rect, alignment, somi->text);
		break;
	}
	case CE_MenuItem:
		if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
			QGuiApplication *app = static_cast<QGuiApplication*>(QGuiApplication::instance());
			qreal checkcol = 25 / app->devicePixelRatio();
			qreal gutterWidth = 3 / app->devicePixelRatio();
			QRect rect = option->rect;
			//draw vertical menu line
			if (option->direction == Qt::LeftToRight)
				checkcol += rect.x();
			int x, y, w, h;
			menuitem->rect.getRect(&x, &y, &w, &h);
			int tab = menuitem->tabWidth;
			bool dis = !(menuitem->state & State_Enabled);
			bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable ? menuitem->checked : false;
			bool act = menuitem->state & State_Selected;
			QPalette palette = QApplication::palette();
			if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
				int yoff = y - 1 + h / 2;
				qreal separatorSize = 6 / app->devicePixelRatio();
				QPoint p1 = QPoint(x + checkcol, yoff);
				QPoint p2 = QPoint(x + w + separatorSize, yoff);
				painter->save();
				painter->fillRect(menuitem->rect, palette.window());
				painter->setPen(palette.mid().color());
				painter->drawLine(p1, p2);
				painter->restore();
				return;
			}
			QString s = menuitem->text;
			QBrush fill;
			if (act) {
				fill = palette.highlight().color().lighter();
			} else {
				fill = palette.window();
			}
			painter->fillRect(menuitem->rect, fill);
			QRect vCheckRect = visualRect(option->direction, menuitem->rect, QRect(menuitem->rect.x(),
																				   menuitem->rect.y(), checkcol - (gutterWidth + menuitem->rect.x()), menuitem->rect.height()));
			if (checked) {
				QStyleOptionMenuItem newMi = *menuitem;
				int windowsItemFrame = 2;
				newMi.rect = visualRect(option->direction,
										menuitem->rect,
										QRect(menuitem->rect.x() + windowsItemFrame,
											  menuitem->rect.y() + windowsItemFrame,
											  checkcol - 2 * windowsItemFrame,
											  menuitem->rect.height() - 2 * windowsItemFrame)
										);
				painter->setRenderHint(QPainter::Antialiasing, true);
				proxy()->drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, painter, widget);
				painter->setRenderHint(QPainter::Antialiasing, false);
			}
			if (!menuitem->icon.isNull()) {
				QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
				if (act && !dis) {
					mode = QIcon::Active;
				}
				QPixmap pixmap;
				if (checked) {
					pixmap = menuitem->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, option, widget), mode, QIcon::On);
				} else {
					pixmap = menuitem->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, option, widget), mode);
				}
				const int pixw = pixmap.width() / pixmap.devicePixelRatio();
				const int pixh = pixmap.height() / pixmap.devicePixelRatio();
				QRect pmr(0, 0, pixw, pixh);
				pmr.moveCenter(vCheckRect.center());
				painter->setPen(palette.text().color());
				painter->drawPixmap(pmr.topLeft(), pixmap);
			}
			//painter->setPen(palette.buttonText().color());
			QColor textColor = palette.text().color();
			if (dis) {
				textColor = palette.mid().color();
				painter->setPen(textColor);
			} else if (act && SettingsPrivate::instance()->isCustomTextColorOverriden()) {
				textColor = palette.highlightedText().color();
				painter->setPen(textColor);
			}
			int xm = checkcol + 2 + (gutterWidth - menuitem->rect.x()) - 1;
			int xpos = menuitem->rect.x() + xm;

			///
			int windowsItemVMargin = 3, windowsRightBorder = 3;
			QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
			QRect vTextRect = visualRect(option->direction, menuitem->rect, textRect);
			if (!s.isEmpty()) {    // draw text
				painter->save();
				int t = s.indexOf(QLatin1Char('\t'));
				int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
				if (!proxy()->styleHint(SH_UnderlineShortcut, menuitem, widget)) {
					text_flags |= Qt::TextHideMnemonic;
				}
				text_flags |= Qt::AlignLeft;
				if (t >= 0) {
					QRect vShortcutRect = visualRect(option->direction, menuitem->rect,
													 QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
					painter->drawText(vShortcutRect, text_flags, s.mid(t + 1));
					s = s.left(t);
				}
				QFont font = menuitem->font;
				if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
					font.setBold(true);
				}
				painter->setFont(font);
				painter->setPen(textColor);
				painter->drawText(vTextRect, text_flags, s.left(t));
				painter->restore();
			}
			if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
				int dim = (h - 2 * 5) / 2;
				PrimitiveElement arrow;
				arrow = (option->direction == Qt::RightToLeft) ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
				xpos = x + w - 2 - 2 - dim;
				QRect  vSubMenuRect = visualRect(option->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
				QStyleOptionMenuItem newMI = *menuitem;
				newMI.rect = vSubMenuRect;
				newMI.state = dis ? State_None : State_Enabled;
				proxy()->drawPrimitive(arrow, &newMI, painter, widget);
			}

		}
		break;
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
		gb2.textColor = QApplication::palette().text().color();
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

void MiamStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole cr) const
{
	if (cr == QPalette::HighlightedText) {
		QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, QPalette::HighlightedText);
	} else {
		QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, QPalette::Text);
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
		QPixmap tabClosePixmap(24, 24);
		int tabHeight = widget->style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, opt, widget);
#if defined(Q_OS_OSX)
		SettingsPrivate *settings = SettingsPrivate::instance();
		QRect tabCloseRect;
		if (settings->isRectTabs()) {
			tabCloseRect = QRect(0, 0, tabHeight, tabHeight);
		} else {
			tabCloseRect = QRect(settings->tabsOverlappingLength() / 1.5, 0, tabHeight, tabHeight);
		}
#else
		QRect tabCloseRect(0, 0, tabHeight, tabHeight);
#endif

		//qDebug() << Q_FUNC_INFO << tabWdidth;
		QFileSelector fs;
		if (opt->state.testFlag(State_MouseOver)) {
			tabClosePixmap.load(fs.select(":/icons/config/close_tabs_hover.png"));
		} else {
			tabClosePixmap.load(fs.select(":/icons/config/close_tabs.png"));
		}
		painter->drawPixmap(tabCloseRect, tabClosePixmap);
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

int MiamStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt, const QWidget *widget) const
{
	int pm = QProxyStyle::pixelMetric(metric, opt, widget);
#if defined(Q_OS_OSX)
	switch (metric) {
	case PM_TabCloseIndicatorWidth: {
		SettingsPrivate *settings = SettingsPrivate::instance();
		if (settings->isRectTabs()) {
			return 13;
		} else {
			return 12 + SettingsPrivate::instance()->tabsOverlappingLength() / 1.5;
		}
	}
	default:
		return pm;
	}
#endif
	return pm;
}
