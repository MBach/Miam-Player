#include "tabbar.h"
#include <settingsprivate.h>
#include "playlist.h"

#include <QApplication>
#include <QIcon>
#include <QMimeData>

#include <QtDebug>

TabBar::TabBar(TabPlaylist *parent) :
	QTabBar(parent), lineEdit(new QLineEdit(this)), tabPlaylist(parent)
{
	this->setAcceptDrops(true);
	this->setDocumentMode(true);
	this->setTabsClosable(true);
	this->setUsesScrollButtons(true);
	this->installEventFilter(this);

	lineEdit->setVisible(false);
	lineEdit->setAlignment(Qt::AlignCenter);
	lineEdit->setFrame(false);
	lineEdit->installEventFilter(this);
	lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

	_timer = new QTimer(this);
	_timer->setInterval(300);
	_timer->setSingleShot(true);

	connect(lineEdit, &QLineEdit::returnPressed, this, &TabBar::renameTab);
	// Switch between tabs if the Drag & Drop isn't finished
	connect(_timer, &QTimer::timeout, [=]() {
		this->setCurrentIndex(tabAt(_targetRect.center()));
		_targetRect = tabRect(currentIndex());
	});

	SettingsPrivate *settings = SettingsPrivate::instance();
	auto applyFont = [this, settings] (SettingsPrivate::FontFamily ff, const QFont &newFont)
	{
		if (ff == SettingsPrivate::FF_Playlist) {
			QFont font = newFont;
			font.setPointSizeF(font.pointSizeF() * 0.8);
			this->setFont(font);
			int h = fontMetrics().height() * 1.25;
			if (h >= 30) {
				this->setMinimumHeight(h);
				this->setMaximumHeight(h);
			} else {
				this->setMinimumHeight(30);
				this->setMaximumHeight(30);
			}
		}
	};
	// Init font from settings
	applyFont(SettingsPrivate::FF_Playlist, settings->font(SettingsPrivate::FF_Playlist));
	connect(settings, &SettingsPrivate::fontHasChanged, this, applyFont);
}

/** Trigger a double click to rename a tab. */
void TabBar::editTab(int indexTab)
{
	QMouseEvent dbClickEvent(QEvent::MouseButtonDblClick, tabRect(indexTab).center(), Qt::LeftButton, 0, 0);
	QApplication::sendEvent(this, &dbClickEvent);
}

/** Redefined to accept D&D from another playlist or the library. */
void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
	int idx = tabAt(event->pos());
	if (idx < count() - 1) {
		_targetRect = tabRect(idx);
	} else {
		_targetRect = QRect();
	}
	event->source() ? event->acceptProposedAction() : event->ignore();
}

/** Redefined to accept D&D from another playlist or the library. */
void TabBar::dragMoveEvent(QDragMoveEvent *event)
{
	int idx = tabAt(event->pos());
	if (idx < count()) {
		_targetRect = tabRect(idx);
	}
	Playlist *playlist = qobject_cast<Playlist*>(event->source());
	if (playlist && idx < count()) {
		_timer->start();
	}
	event->source() ? event->acceptProposedAction() : event->ignore();
}

/** Redefined to accept D&D from another playlist or the library. */
void TabBar::dropEvent(QDropEvent *event)
{
	int tab = this->tabAt(event->pos());
	if (Playlist *origin = qobject_cast<Playlist*>(event->source())) {
		Playlist *target = tabPlaylist->playlist(tab);

		// Copy tracks in the target
		QList<QMediaContent> medias;
		for (QModelIndex index : origin->selectionModel()->selectedRows()) {
			medias.append(origin->mediaPlaylist()->media(index.row()));
		}
		// Append tracks at the end
		target->insertMedias(target->model()->rowCount(), medias);

		// Remove tracks from the current playlist if necessary
		if (!SettingsPrivate::instance()->copyTracksFromPlaylist()) {
			origin->removeSelectedTracks();
		}
	} else {
		QByteArray byteArray = event->mimeData()->data("treeview/x-treeview-item");
		if (!byteArray.isEmpty()) {
			QList<QUrl> medias;
			QList<QByteArray> encodedUrls = byteArray.split('|');
			for (QByteArray encodedUrl : encodedUrls) {
				medias << QUrl::fromEncoded(encodedUrl);
			}
			tabPlaylist->insertItemsToPlaylist(-1, medias);
		}
	}

}

/** Redefined to validate new tab name if the focus is lost. */
bool TabBar::eventFilter(QObject *obj, QEvent *event)
{
	// Accept the escape key
	if (obj == lineEdit && event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape) {
			lineEdit->releaseMouse();
			lineEdit->setText(lineEdit->text().trimmed());
			lineEdit->close();
		}
	} else if (obj == lineEdit && event->type() == QEvent::FocusOut) {
		lineEdit->releaseMouse();
		lineEdit->setText(lineEdit->text().trimmed());
		renameTab();
	} else if (obj == lineEdit && event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *me = static_cast<QMouseEvent*>(event);
		if (!lineEdit->rect().contains(me->pos())) {
			lineEdit->releaseMouse();
			lineEdit->setText(lineEdit->text().trimmed());
			lineEdit->close();
		}
	} else if (obj == this && event->type() == QEvent::HoverLeave) {
		this->update();
	}
	return false;
}

/** Redefined to display an editable area. */
void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
	int tabIndex = tabAt(event->pos());
	int c = currentIndex();
	if (-1 < tabIndex && tabIndex < count() && c == tabIndex) {
		QRect visualRect = tabRect(tabIndex);
		SettingsPrivate *settings = SettingsPrivate::instance();
		if (settings->isRectTabs()) {
			visualRect.setLeft(visualRect.left() + 1);
			visualRect.setRight(visualRect.right() - 1);
		} else {
			visualRect.setLeft(visualRect.left() + 3 + settings->tabsOverlappingLength() * 1.25 + height() / 2);
			visualRect.setRight(visualRect.right() - settings->tabsOverlappingLength());
		}
		visualRect.setTop(visualRect.top() + 1);

		// Disable close buttons in case of unfortunate click
		for (int t = 0; t < count(); t++) {
			QWidget *button = tabButton(t, QTabBar::RightSide);
			if (button && t != tabIndex) {
				button->setEnabled(false);
			}
		}

		// Feed the QLineEdit with current tab text
		lineEdit->setText(tabText(tabIndex));
		lineEdit->selectAll();
		lineEdit->setFocus();
		lineEdit->setGeometry(visualRect);
		lineEdit->setVisible(true);
		lineEdit->grabMouse();
	}
	QTabBar::mouseDoubleClickEvent(event);
}

/** Redefined to validate new tab name without pressing return. */
void TabBar::mousePressEvent(QMouseEvent *event)
{
	int tabIndex = tabAt(event->pos());
	if (lineEdit->isVisible() && ((currentIndex() != tabIndex && lineEdit->isVisible()) || !lineEdit->geometry().contains(event->pos()))) {
		lineEdit->close();
	} else {
		QTabBar::mousePressEvent(event);
	}
}

void TabBar::paintEvent(QPaintEvent *e)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	QStylePainter p(this);
	int dist = 0;
	if (settings->isRectTabs()) {
		paintRectTabs(p);
	} else {
		dist = settings->tabsOverlappingLength();
		paintRoundedTabs(p, dist);
	}

	// Global bottom frame border
	int h = tabRect(0).height() - 1;
	p.setPen(QApplication::palette().mid().color());
	if (count() > 1) {
		if (currentIndex() == 0) {
			p.drawLine(tabRect(0).right() + 1 + dist, h, rect().right(), h);
		} else {
			p.drawLine(rect().left(), h, tabRect(currentIndex()).left(), h);
			p.drawLine(tabRect(currentIndex()).right() + 1 + dist, h, rect().right(), h);
		}
	} else {
		if (isLeftToRight()) {
			p.drawLine(tabRect(0).width() + dist, h, rect().right(), h);
		} else {
			p.drawLine(rect().left(), h, tabRect(0).left(), h);
		}
	}
}

/** Redefined to return a square for the last tab which is the [+] button. */
QSize TabBar::tabSizeHint(int index) const
{
	QSize s = QTabBar::tabSizeHint(index);
	if (!SettingsPrivate::instance()->isRectTabs()) {
		s.setWidth(s.width() + SettingsPrivate::instance()->tabsOverlappingLength() * 2);
	}
	// Adjust height to the minimum otherwise a small gap might appear between tab and header (depending of font size)
	if (s.height() > height()) {
		s.setHeight(height());
	}
	return s;
}

void TabBar::paintRectTabs(QStylePainter &p)
{
	static const qreal penScaleFactor = 0.2;
	for (int i = 0; i < count(); i++) {
		QStyleOptionTab o;
		initStyleOption(&o, i);

		// Background color
		p.save();
		if (i != currentIndex()) {
			o.rect.adjust(0, 2, 0, 0);
		} else if (i == count()) {
			o.rect.adjust(2, 2, -4, -4);
		}

		// Highlight the tab under the cursor
		if (o.state.testFlag(QStyle::State_MouseOver) && i != currentIndex()) {
			p.setPen(QPen(o.palette.highlight(), penScaleFactor));
			p.fillRect(o.rect, o.palette.highlight().color().lighter());
		} else {
			p.setPen(QPen(o.palette.mid(), penScaleFactor));
			if (i == currentIndex()) {
				/// XXX
				if (SettingsPrivate::instance()->isCustomColors()) {
					p.fillRect(o.rect, o.palette.base().color().lighter(110));
				} else {
					p.fillRect(o.rect, o.palette.base());
				}
			} else {
				p.fillRect(o.rect, o.palette.window());
			}
		}

		if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.highlight().color());
		} else {
			p.setPen(o.palette.mid().color());
		}
		// Frame tab, it is not a rectangle but only 3 lines
		p.drawLine(o.rect.topLeft(), o.rect.bottomLeft());
		p.drawLine(o.rect.topRight(), o.rect.bottomRight());
		p.drawLine(o.rect.topLeft(), o.rect.topRight());
		//}
		p.restore();

		// Icon
		QRect r = tabRect(i);
		r.setHeight(fontMetrics().ascent());
		r.translate(10, (height() - r.height()) / 2);
		r.setWidth(r.height() / 2);
		p.setRenderHint(QPainter::SmoothPixmapTransform);
		o.icon.paint(&p, r, Qt::AlignLeft | Qt::AlignVCenter);

		// Playlist name
		if (i == currentIndex()) {
			p.setPen(o.palette.windowText().color());
		} else if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.windowText().color());
		} else {
			p.setPen(o.palette.mid().color());
		}
		QRect rText(r.x() + r.width() + 10, r.y(), o.rect.width() - r.width() - 10, r.height());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, o.text);
	}
}

void TabBar::paintRoundedTabs(QStylePainter &p, int dist)
{
	/// TODO: minor highlight bug when mouse goes on another tab without click
	// Draw all tabs before the selected tab
	QList<int> tabs;
	for (int i = 0; i < count(); i++)
		if (currentIndex() != i) tabs.append(i);
	tabs.append(currentIndex());

	for (int idx = 0; idx < count(); idx++) {
		int i = tabs.at(idx);
		QStyleOptionTab o;
		initStyleOption(&o, i);

		// Background color
		if (i != currentIndex()) {
			o.rect.adjust(0, 2, 0, 0);
		} else if (i == count()) {
			o.rect.adjust(2, 2, -4, 0);
		}

		/// Adjust parameters to tighten tabs
		//o.rect.adjust(-dist / 2, 0, dist / 2, 0);

		// Rounded frame tab
		QPainterPath ppLeft, ppRight;
		ppLeft.moveTo(o.rect.x() + dist * 0, o.rect.y() + o.rect.height());
		ppLeft.cubicTo(o.rect.x() + dist * 1, o.rect.y() + o.rect.height(),
					o.rect.x() + dist * 1, o.rect.y() + 1,
					o.rect.x() + dist * 2, o.rect.y() + 1);
		QPainterPath ppLeftCurve(ppLeft);
		// Add another point to be able to fill the path afterwards
		ppLeft.lineTo(o.rect.x() + dist * 2, o.rect.y() + o.rect.height());

		QLine topHozLine(o.rect.x() + dist * 2, o.rect.y(),
						 o.rect.x() + o.rect.width() - dist * 1, o.rect.y());

		ppRight.moveTo(o.rect.x() + o.rect.width() - dist * 1, o.rect.y() + 1);
		ppRight.cubicTo(o.rect.x() + o.rect.width() - dist * 0, o.rect.y() + 1,
					o.rect.x() + o.rect.width() - dist * 0, o.rect.y() + o.rect.height(),
					o.rect.x() + o.rect.width() + dist * 1, o.rect.y() + o.rect.height());
		QPainterPath ppRightCurve(ppRight);
		// Like first curve
		ppRight.lineTo(o.rect.x() + o.rect.width() - dist * 1, o.rect.y() + o.rect.height());

		p.save();
		if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.highlight().color());
		} else {
			p.setPen(o.palette.mid().color());
		}
		QRect midRect(topHozLine.p1(), QPoint(topHozLine.p2().x(), topHozLine.p2().y() + o.rect.height()));
		if (i == currentIndex()) {
			p.fillPath(ppLeft, o.palette.base());
			p.fillRect(midRect, o.palette.base());
			p.fillPath(ppRight, o.palette.base());
		} else if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.fillPath(ppLeft, o.palette.highlight().color().lighter());
			p.fillRect(midRect, o.palette.highlight().color().lighter());
			p.fillPath(ppRight, o.palette.highlight().color().lighter());
		} else {
			p.fillPath(ppLeft, o.palette.window());
			p.fillRect(midRect, o.palette.window());
			p.fillPath(ppRight, o.palette.window());
		}
		p.setRenderHint(QPainter::Antialiasing, true);
		p.drawPath(ppLeftCurve);
		p.drawPath(ppRightCurve);
		p.setRenderHint(QPainter::Antialiasing, false);
		p.drawLine(topHozLine);

		p.restore();

		/// DEBUG
		//p.drawRect(o.rect);

		// Icon
		QRect r = tabRect(i);
		r.setHeight(fontMetrics().ascent());
		r.translate(3 + dist * 1.25, (height() - r.height()) / 2);
		r.setWidth(r.height() / 2);
		p.setRenderHint(QPainter::SmoothPixmapTransform);
		o.icon.paint(&p, r, Qt::AlignLeft | Qt::AlignVCenter);

		// Playlist name
		if (i == currentIndex()) {
			p.setPen(o.palette.windowText().color());
		} else if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.windowText().color());
		} else {
			p.setPen(o.palette.mid().color());
		}
		QRect rText(r.x() + r.width() + 5, this->rect().y(),
					o.rect.width() - (r.width() + 5), this->height() - 2);
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, o.text);
	}
}

/** Rename a tab. */
void TabBar::renameTab()
{
	this->setTabText(currentIndex(), lineEdit->text());
	emit tabRenamed(currentIndex(), tabText(currentIndex()));
	lineEdit->close();
	for (int t = 0; t < count(); t++) {
		QWidget *button = tabButton(t, QTabBar::RightSide);
		if (button) {
			button->setEnabled(true);
		}
	}
}
