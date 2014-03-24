#include "tabbar.h"
#include "settings.h"
#include "playlist.h"
#include "../treeview.h"

#include <QtDebug>

#include <QIcon>

TabBar::TabBar(TabPlaylist *parent) :
	QTabBar(parent), lineEdit(new QLineEdit(this)), tabPlaylist(parent)
{
	this->setTabsClosable(true);
	this->addTab(QString());
	this->setAcceptDrops(true);
	this->setDocumentMode(true);
	this->setTabButton(count()-1, QTabBar::RightSide, 0);

	lineEdit->setVisible(false);
	lineEdit->setAlignment(Qt::AlignCenter);
	lineEdit->setFrame(false);
	lineEdit->installEventFilter(this);

	_timer = new QTimer(this);
	_timer->setInterval(300);
	_timer->setSingleShot(true);

	connect(lineEdit, &QLineEdit::returnPressed, this, &TabBar::renameTab);
	// Switch between tabs if the Drag & Drop isn't finished
	connect(_timer, &QTimer::timeout, [=]() {
		this->setCurrentIndex(tabAt(_targetRect.center()));
		_targetRect = tabRect(currentIndex());
	});

	connect(Settings::getInstance(), &Settings::fontHasChanged, [=](Settings::FontFamily ff, const QFont &newFont) {
		if (ff == Settings::PLAYLIST) {
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
	});
}

/** Redefined to validate new tab name if the focus is lost. */
bool TabBar::eventFilter(QObject *obj, QEvent *event)
{
	// Accept the escape key
	if (obj == lineEdit && event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape) {
			lineEdit->close();
		}
	} else if (obj == lineEdit && event->type() == QEvent::FocusOut) {
		renameTab();
	}
	return false;
}

/** Redefined to accept D&D from another playlist or the library. */
void TabBar::dropEvent(QDropEvent *event)
{
	int tab = this->tabAt(event->pos());
	if (Playlist *origin = qobject_cast<Playlist*>(event->source())) {
		Playlist *target;
		// Tracks were dropped on the (+) button
		if (tab == this->count() - 1) {
			target = tabPlaylist->addPlaylist();
		} else {
			target = tabPlaylist->playlist(tab);
		}
		// Copy tracks in the target
		QList<QMediaContent> medias;
		foreach (QPersistentModelIndex index, origin->selectionModel()->selectedRows()) {
			medias.append(origin->mediaPlaylist()->media(index.row()));
		}
		// Append tracks at the end
		target->insertMedias(target->model()->rowCount(), medias);

		// Remove tracks from the current playlist if necessary
		if (!Settings::getInstance()->copyTracksFromPlaylist()) {
			origin->removeSelectedTracks();
		}
	} else if (TreeView *origin = qobject_cast<TreeView*>(event->source())) {
		// Tracks were dropped on the (+) button
		if (tab == this->count() - 1) {
			tabPlaylist->addPlaylist();
		} else {
			tabPlaylist->setCurrentIndex(tab);
		}
		origin->appendToPlaylist();
	}
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
	// Exclude current tab and last one
	if (idx < count() - 1) {
		_targetRect = tabRect(idx);
	}
	Playlist *playlist = qobject_cast<Playlist*>(event->source());
	if (playlist) {
		// Exclude current tab and last one?
		if (idx < count() - 1) {
			_timer->start();
		}
	}
	event->source() ? event->acceptProposedAction() : event->ignore();
}

/** Redefined to display an editable area. */
void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
	int tabIndex = tabAt(event->pos());
	int c = currentIndex();
	if (-1 < tabIndex && tabIndex < count()-1 && c == tabIndex) {
		QRect visualRect = tabRect(tabIndex);
		visualRect.setLeft(visualRect.left() + 1);
		visualRect.setRight(visualRect.right() - 1);
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

void TabBar::paintEvent(QPaintEvent *)
{
	// A "[+]" button on the right
	static const QPointF plus[13] = {
		QPointF(1, 2), QPointF(2, 2), QPointF(2, 1),
		QPointF(3, 1), QPointF(3, 2), QPointF(4, 2),
		QPointF(4, 3), QPointF(3, 3), QPointF(3, 4),
		QPointF(2, 4), QPointF(2, 3), QPointF(1, 3),
		QPointF(1, 2),
	};

	static const qreal penScaleFactor = 0.2;

	QStylePainter p(this);
	QStyleOptionTab o;

	for (int i = 0; i < count(); i++) {
		initStyleOption(&o, i);
		//o.rect.adjust(0, 0, 0, -1);

		// Frame color
		p.setPen(o.palette.mid().color());

		// Background color
		p.save();
		if (i == currentIndex()) {
			p.setBrush(o.palette.base().color().lighter(110));
		} else if (o.state.testFlag(QStyle::State_MouseOver)) {
			o.rect.adjust(0, 2, 0, 0);
			p.setPen(o.palette.highlight().color());
			p.setBrush(o.palette.highlight().color().lighter());
		} else {
			o.rect.adjust(0, 2, 0, 0);
			p.setBrush(o.palette.base());
		}

		if (i + 1 == count()) {
			o.rect.setWidth(o.rect.height());
			o.rect.adjust(2, 1, -2, -3);
			p.drawRect(o.rect);
			p.translate(o.rect.topLeft());
			p.scale(o.rect.height() * penScaleFactor, o.rect.height() * penScaleFactor);
			if (o.state.testFlag(QStyle::State_MouseOver)) {
				p.setPen(QPen(o.palette.highlight(), penScaleFactor));
			} else {
				p.setPen(QPen(o.palette.mid(), penScaleFactor));
			}
			p.setBrush(QColor(253, 230, 116));
			p.drawPolygon(plus, 13);
		} else {
			p.drawRect(o.rect);
		}
		p.restore();

		// Icon
		/// FIXME
		QRect r = tabRect(i).adjusted(3, 3, 0, 0);
		qDebug() << i << r << r.left();
		p.save();
		p.translate(r.left(), 0);
		int w = o.iconSize.width(), h = o.iconSize.height();
		p.drawPixmap(0, r.top(), w, h, o.icon.pixmap(w, h));
		p.restore();

		// Playlist name
		if (i == currentIndex()) {
			p.setPen(o.palette.windowText().color());
		} else if (o.state.testFlag(QStyle::State_MouseOver)) {
			p.setPen(o.palette.windowText().color());
		} else {
			p.setPen(o.palette.mid().color());
		}
		p.drawText(o.rect, Qt::AlignCenter, o.text);
	}

	// Bottom frame border
	int h = tabRect(0).height();
	p.setPen(o.palette.mid().color());
	if (count() > 2) {
		if (currentIndex() == 0) {
			p.drawLine(tabRect(0).right() + 1, h, rect().right(), h);
		} else {
			p.drawLine(rect().left(), h, tabRect(currentIndex()).left(), h);
			p.drawLine(tabRect(currentIndex()).right() + 1, h, rect().right(), h);
		}
	} else {
		if (isLeftToRight()) {
			p.drawLine(tabRect(0).width(), h, rect().right(), h);
		} else {
			p.drawLine(rect().left(), h, tabRect(0).left(), h);
		}
	}
}

/** Rename a tab. */
void TabBar::renameTab()
{
	this->setTabText(currentIndex(), lineEdit->text());
	lineEdit->close();
	for (int t = 0; t < count(); t++) {
		QWidget *button = tabButton(t, QTabBar::RightSide);
		if (button) {
			button->setEnabled(true);
		}
	}
}
