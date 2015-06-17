#include "extendedtabbar.h"

#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionTabBarBase>
#include "settingsprivate.h"

#include <QtDebug>

/** Default constructor. */
ExtendedTabBar::ExtendedTabBar(QWidget *parent)
	: QTabBar(parent)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	QFont f = settings->font(SettingsPrivate::FF_Library);
	f.setPointSizeF(f.pointSizeF() * 0.8);
	this->setFont(f);
	this->setMouseTracking(true);


	connect(settings, &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff, const QFont &newFont) {
		if (ff == SettingsPrivate::FF_Library) {
			this->setMinimumHeight(fontMetrics().height() * 1.25);
			this->setMaximumHeight(fontMetrics().height() * 1.25);
			QFont font = newFont;
			font.setPointSizeF(font.pointSizeF() * 0.8);
			this->setFont(font);
		}
	});
}

QSize ExtendedTabBar::tabSizeHint(int) const
{
	/// FIXME or bug?
#if defined(Q_OS_OSX)
	return QSize(this->rect().width() / 2.05, this->rect().height());
#else
	return QSize(this->rect().width() / 2, this->rect().height());
#endif
}

/** Redefined to be style-aware at runtime. */
void ExtendedTabBar::paintEvent(QPaintEvent *)
{
	QStyleOptionTab lib, fe;
	initStyleOption(&lib, 0);
	initStyleOption(&fe, 1);
	QPalette pal = QApplication::palette();

	QStylePainter p(this);
	// Library is selected
	if (currentIndex() == 0) {
		p.fillRect(lib.rect, lib.palette.base().color());
		p.setPen(pal.text().color());
		p.drawText(lib.rect, Qt::AlignCenter, fontMetrics().elidedText(lib.text, Qt::ElideRight, lib.rect.width()));

		// Draw Top and Right lines only
		p.setPen(pal.mid().color());
		p.drawLine(lib.rect.x(), lib.rect.y(), lib.rect.x() + lib.rect.width(), lib.rect.y());
		if (isLeftToRight()) {
			p.drawLine(lib.rect.x() + lib.rect.width(), lib.rect.y(), lib.rect.x() + lib.rect.width(), lib.rect.y() + lib.rect.height());
		} else {
			p.drawLine(lib.rect.x(), lib.rect.y(), lib.rect.x(), lib.rect.y() + lib.rect.height());
		}

		// Reduce the size of unselected tab
		fe.rect.adjust(3, 2, -3, 0);

		if (fe.state.testFlag(QStyle::State_MouseOver)) {
			p.fillRect(fe.rect, pal.highlight().color().light());
			p.setPen(pal.text().color());
			p.drawText(fe.rect, Qt::AlignCenter, fontMetrics().elidedText(fe.text, Qt::ElideRight, fe.rect.width()));
			p.setPen(fe.palette.highlight().color());
		} else {
			p.fillRect(fe.rect, pal.window().color());
			p.setPen(pal.mid().color());
			p.drawText(fe.rect, Qt::AlignCenter, fontMetrics().elidedText(fe.text, Qt::ElideRight, fe.rect.width()));
		}

		// Draw Left, Top and Right lines
		p.drawLine(fe.rect.x(), fe.rect.y() + fe.rect.height(), fe.rect.x(), fe.rect.y());
		p.drawLine(fe.rect.x(), fe.rect.y(), fe.rect.x() + fe.rect.width(), fe.rect.y());
		p.drawLine(fe.rect.x() + fe.rect.width(), fe.rect.y(), fe.rect.x() + fe.rect.width(), fe.rect.y() + fe.rect.height());

		// Draw bottom line
		p.setPen(pal.mid().color());
		if (isLeftToRight()) {
			p.drawLine(rect().x() + rect().width() / 2, rect().y() + rect().height() - 1, rect().x() + rect().width(), rect().y() + rect().height() - 1);
		} else {
			p.drawLine(rect().x(), rect().y() + rect().height() - 1, rect().x() + rect().width() / 2, rect().y() + rect().height() - 1);
		}
	} else {
		p.fillRect(fe.rect, lib.palette.base().color());
		p.setPen(pal.text().color());
		p.drawText(fe.rect, Qt::AlignCenter, fontMetrics().elidedText(fe.text, Qt::ElideRight, fe.rect.width()));

		if (isLeftToRight()) {
			if (rect().width() % 2 == 0) {
				lib.rect.adjust(0, 0, -1, 0);
				fe.rect.adjust(-1, 0, -1, 0);
			}
		} else {
			if (rect().width() % 2 == 1) {
				lib.rect.adjust(0, 0, -1, 0);
				fe.rect.adjust(1, 0, -1, 0);
			}
		}

		// Draw Left, Top and Right lines
		p.setPen(pal.mid().color());
		p.drawLine(fe.rect.x(), fe.rect.y() + fe.rect.height(), fe.rect.x(), fe.rect.y());
		p.drawLine(fe.rect.x(), fe.rect.y(), fe.rect.x() + fe.rect.width(), fe.rect.y());
		p.drawLine(fe.rect.x() + fe.rect.width(), fe.rect.y(), fe.rect.x() + fe.rect.width(), fe.rect.y() + fe.rect.height());

		// Reduce the size of unselected tab
		lib.rect.adjust(2, 2, -3, 0);
		if (lib.state.testFlag(QStyle::State_MouseOver)) {
			p.fillRect(lib.rect, pal.highlight().color().light());
			p.setPen(pal.text().color());
			p.drawText(lib.rect, Qt::AlignCenter, fontMetrics().elidedText(lib.text, Qt::ElideRight, lib.rect.width()));
			p.setPen(lib.palette.highlight().color());
		} else {
			p.fillRect(lib.rect, pal.window().color());
			p.setPen(pal.mid().color());
			p.drawText(lib.rect, Qt::AlignCenter, fontMetrics().elidedText(lib.text, Qt::ElideRight, lib.rect.width()));
		}

		// Draw Left, Top and Right lines
		p.drawLine(lib.rect.x(), lib.rect.y() + lib.rect.height(), lib.rect.x(), lib.rect.y());
		p.drawLine(lib.rect.x(), lib.rect.y(), lib.rect.x() + lib.rect.width(), lib.rect.y());
		p.drawLine(lib.rect.x() + lib.rect.width(), lib.rect.y(), lib.rect.x() + lib.rect.width(), lib.rect.y() + lib.rect.height());
	}
}
