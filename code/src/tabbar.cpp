#include "tabbar.h"
#include "settings.h"

#include <QtDebug>

#include <QIcon>

TabBar::TabBar(QWidget *parent) :
	QTabBar(parent)
{
	this->setTabsClosable(true);
	this->addTab(QIcon(":/icons/plusIcon"), QString());
	this->setTabButton(count()-1, QTabBar::RightSide, 0);

	lineEdit = new QLineEdit(this);
	lineEdit->setVisible(false);
	lineEdit->setAlignment(Qt::AlignCenter);
	lineEdit->setFrame(false);
	lineEdit->installEventFilter(this);

	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(renameTab()));
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
