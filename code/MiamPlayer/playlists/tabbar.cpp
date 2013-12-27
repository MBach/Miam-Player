#include "tabbar.h"
#include "settings.h"
#include "playlist.h"
#include "../treeview.h"

#include <QtDebug>

#include <QIcon>

TabBar::TabBar(TabPlaylist *parent) :
	QTabBar(parent), tabPlaylist(parent)
{
	this->setTabsClosable(true);
	this->addTab(QIcon(":/icons/plusIcon"), QString());
	this->setTabButton(count()-1, QTabBar::RightSide, 0);
	this->setAcceptDrops(true);

	lineEdit = new QLineEdit(this);
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
