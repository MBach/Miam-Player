#include "extendedtabbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTabBarBase>
#include "settings.h"

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

		tab.rect.adjust(1, 3, -3, 0);
		if (Settings::getInstance()->isCustomColors()) {
			p.fillRect(tab.rect, tab.palette.base());
		} else {
			p.fillRect(tab.rect, tab.palette.window().color().lighter(105));
		}
		p.drawText(tab.rect, Qt::AlignCenter, tab.text);
	}

	// Draw the selected tab last to get it "on top"
	if (selected >= 0) {
		QStyleOptionTab tab;
		initStyleOption(&tab, selected);
		tab.palette = QApplication::palette();
		p.fillRect(tab.rect, tab.palette.base().color().lighter(110));
		p.drawText(tab.rect, Qt::AlignCenter, tab.text);
		p.setPen(tab.palette.mid().color());
		p.drawLine(tab.rect.topLeft(), tab.rect.topRight());
		if (isLeftToRight()) {
			p.drawLine(tab.rect.topRight(), tab.rect.bottomRight());
			if (selected > 0) {
				p.drawLine(tab.rect.topLeft(), tab.rect.bottomLeft());
			}
		} else {
			p.drawLine(tab.rect.topLeft(), tab.rect.bottomLeft());
			if (selected > 0) {
				p.drawLine(tab.rect.topRight(), tab.rect.bottomRight());
			}
		}
	}
}
