#include "addressbarmenu.h"

#include <QAction>
#include <QDir>
#include <QMouseEvent>
#include <QFileIconProvider>

#include <QtDebug>

#include "addressbar.h"

#include <QApplication>

AddressBarMenu::AddressBarMenu(AddressBar *addressBar) :
	QListWidget(addressBar), _addressBar(addressBar), _hasSeparator(false)
{
	this->installEventFilter(this);
	this->setMouseTracking(true);
	this->setUniformItemSizes(true);
	this->setWindowFlags(Qt::Popup);

	connect(this, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
		_addressBar->init(item->data(Qt::UserRole).toString());
		this->clear();
		this->close();
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


bool AddressBarMenu::hasSeparator() const
{
	return _hasSeparator;
}

void AddressBarMenu::insertSeparator() const
{
	/// TODO
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

void AddressBarMenu::moveOrHide(const AddressBarButton *b)
{
	QPoint globalButtonPos = b->mapToGlobal(b->rect().bottomRight());
	globalButtonPos.rx() -= 2 * b->arrowRect().width();
	globalButtonPos.ry() += 2;
	//_sender = b;
	this->move(globalButtonPos);
	this->show();
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
	if (count() > 0) {
		QListWidget::show();
	} else {
		QListWidget::hide();
	}
}
