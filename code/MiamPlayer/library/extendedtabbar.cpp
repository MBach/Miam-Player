#include "extendedtabbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTabBarBase>

#include <QtDebug>

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

	if (abs(o.palette.windowText().color().value() - o.palette.base().color().value()) < 128) {
		p.setPen(o.palette.brightText().color());
	}

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

		p.drawControl(QStyle::CE_TabBarTab, tab);
		//p.fillRect(tab.rect, tab.palette.base());
		//p.drawText(tab.rect, Qt::AlignCenter, tab.text);
	}

	// Draw the selected tab last to get it "on top"
	if (selected >= 0) {
		QStyleOptionTab tab;
		initStyleOption(&tab, selected);
		tab.palette = QApplication::palette();
		p.fillRect(tab.rect, tab.palette.base());
		p.drawText(tab.rect, Qt::AlignCenter, tab.text);
	}
}
