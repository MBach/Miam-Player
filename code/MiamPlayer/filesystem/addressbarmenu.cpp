#include "addressbarmenu.h"

#include <QAction>
#include <QDir>
#include <QMouseEvent>
#include <QFileIconProvider>

#include <QtDebug>

#include "addressbar.h"

#include <QApplication>

AddressBarMenu::AddressBarMenu(AddressBar *addressBar) :
	QListWidget(addressBar), _addressBar(addressBar)
{
	this->installEventFilter(this);
	//this->setMaximumWidth(300);
	this->setMouseTracking(true);
	this->setUniformItemSizes(true);
	this->setWindowFlags(Qt::Popup);

	connect(this, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
		this->clear();
		this->close();
		//_addressBar->createSubDirButtons();
	});
}

bool AddressBarMenu::eventFilter(QObject *, QEvent *event)
{
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (!rect().contains(mouseEvent->pos())) {
			_addressBar->findAndHighlightButton(mapToGlobal(mouseEvent->pos()));
		}
	}
	return false;
}

void AddressBarMenu::appendSubfolder(AddressBarButton *button)
{
	foreach (QFileInfo drive, QDir::drives()) {
		if (QDir::toNativeSeparators(drive.absolutePath()).remove(QDir::separator()) != button->text()) {
			subfolders.prepend(button);
		}
		break;
	}
}

void AddressBarMenu::removeSubfolder(AddressBarButton *button)
{
	for (int i = 0; i < subfolders.length(); i++) {
		if (button->currentPath() == subfolders.at(i)->currentPath()) {
			subfolders.removeAt(i);
			break;
		}
	}
	// Remove the separator
	if (subfolders.isEmpty()) {
		foreach (QAction *action, actions()) {
			if (action->isSeparator()) {
				delete action;
				break;
			}
		}
	}
}

void AddressBarMenu::show()
{
	static const int maxBeforeScrolling = 18;
	static const int margin = 4;
	if (count() < maxBeforeScrolling) {
		this->setMinimumHeight(count() * sizeHintForRow(0) + margin);
		this->setMaximumHeight(count() * sizeHintForRow(0) + margin);
	} else {
		this->setMinimumHeight(maxBeforeScrolling * sizeHintForRow(0) + margin);
		this->setMaximumHeight(maxBeforeScrolling * sizeHintForRow(0) + margin);
	}
	QListWidget::show();
}
