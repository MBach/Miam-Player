#include "extendedtabbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTabBarBase>
#include "settings.h"

#include <QtDebug>

ExtendedTabBar::ExtendedTabBar(QWidget *parent)
	: QTabBar(parent)
{
	Settings *settings = Settings::getInstance();
	QFont f = settings->font(Settings::FF_Library);
	f.setPointSizeF(f.pointSizeF() * 0.8);
	this->setFont(f);
	this->setMouseTracking(true);


	connect(settings, &Settings::fontHasChanged, [=](Settings::FontFamily ff, const QFont &newFont) {
		if (ff == Settings::FF_Library) {
			this->setMinimumHeight(fontMetrics().height() * 1.25);
			this->setMaximumHeight(fontMetrics().height() * 1.25);
			QFont font = newFont;
			font.setPointSizeF(font.pointSizeF() * 0.8);
			this->setFont(font);
		}
	});
}

void ExtendedTabBar::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);

	QStyleOptionTabBarBase o;
	o.palette = QApplication::palette();
	int selected = currentIndex();

	for (int i = 0; i < count(); ++i)
		o.tabBarRect |= tabRect(i);

	o.selectedTabRect = tabRect(selected);
	if (drawBase())
		p.drawPrimitive(QStyle::PE_FrameTabBarBase, o);

	for (int i = 0; i < count(); ++i) {
		QStyleOptionTab tab;
		initStyleOption(&tab, i);
		tab.palette = QApplication::palette();
		if (!(tab.state & QStyle::State_Enabled)) {
			tab.palette.setCurrentColorGroup(QPalette::Disabled);
		}

		o.tabBarRect |= tab.rect;
		if (i == selected)
			continue;

		// Reduces the size of the tab
		if (i > 0) {
			if (isLeftToRight()) {
				tab.rect.adjust(1, 3, -3, 0);
			} else {
				tab.rect.adjust(1, 3, 0, 0);
			}
		} else {
			if (isLeftToRight()) {
				tab.rect.adjust(3, 3, 0, 0);
			} else {
				tab.rect.adjust(0, 3, 0, 0);
			}
		}
		/// XXX: custom/default colors shouldn't be treated here
		p.save();
		if (Settings::getInstance()->isCustomColors()) {
			if (tab.state.testFlag(QStyle::State_MouseOver)) {
				p.setPen(tab.palette.highlight().color());
				p.fillRect(tab.rect, tab.palette.highlight().color().lighter());
			} else {
				p.setPen(tab.palette.mid().color());
				p.fillRect(tab.rect, tab.palette.base());
			}
		} else {
			if (tab.state.testFlag(QStyle::State_MouseOver)) {
				p.setPen(o.palette.highlight().color());
				p.fillRect(tab.rect, tab.palette.highlight().color().lighter(170));
			} else {
				p.setPen(o.palette.midlight().color());
				p.fillRect(tab.rect, tab.palette.window().color().lighter(105));
			}
		}

		p.drawLine(tab.rect.x(), tab.rect.y(),
				   tab.rect.x() + tab.rect.width(), tab.rect.y());
		p.drawLine(tab.rect.x(), tab.rect.y(),
				   tab.rect.x(), tab.rect.y() + tab.rect.height());
		p.drawLine(tab.rect.x() + tab.rect.width(), tab.rect.y(),
				   tab.rect.x() + tab.rect.width(), tab.rect.y() + tab.rect.height());
		p.restore();

		/*if (tab.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.highlightedText().color());
		} else {
			p.setPen(o.palette.windowText().color());
		}*/
		// If the rectangle is smaller than the text, shrink it
		p.drawText(tab.rect, Qt::AlignCenter, fontMetrics().elidedText(tab.text, Qt::ElideRight, tab.rect.width()));
	}

	// Draw the selected tab last to get it "on top"
	if (selected >= 0) {
		QStyleOptionTab tab;
		initStyleOption(&tab, selected);
		tab.palette = QApplication::palette();
		p.fillRect(tab.rect, tab.palette.base().color().lighter(110));
		/*if (tab.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.highlightedText().color());
		} else {
			p.setPen(o.palette.windowText().color());
		}*/
		p.drawText(tab.rect, Qt::AlignCenter, fontMetrics().elidedText(tab.text, Qt::ElideRight, tab.rect.width()));
		p.setPen(tab.palette.mid().color());
		p.drawLine(tab.rect.x(), tab.rect.y(),
				   tab.rect.x() + tab.rect.width(), tab.rect.y());
		if (isLeftToRight()) {
			p.drawLine(tab.rect.x() + tab.rect.width(), tab.rect.y(),
					   tab.rect.x() + tab.rect.width(), tab.rect.y() + tab.rect.height());
			if (selected > 0) {
				p.drawLine(tab.rect.x(), tab.rect.y(),
						   tab.rect.x(), tab.rect.y() + tab.rect.height());
			}
		} else {
			tab.rect.adjust(1, 0, 0, 0);
			p.drawLine(tab.rect.topLeft(), tab.rect.bottomLeft());
			if (selected > 0) {
				p.drawLine(tab.rect.topRight(), tab.rect.bottomRight());
			}
		}
	}
}
