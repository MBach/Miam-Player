#include "addressbarmenu.h"

#include <QDir>
#include <QFileIconProvider>

#include <QtDebug>

AddressBarMenu::AddressBarMenu(QWidget *parent) :
	QMenu(parent)
{}

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

void AddressBarMenu::clear()
{
	QMenu::clear();
}

QAction * AddressBarMenu::exec(const QPoint &p, bool showSubfolers, QAction *action)
{
	QAction *firstAction = NULL;
	if (showSubfolers && !this->actions().isEmpty()) {
		firstAction = this->actions().first();
		foreach (AddressBarButton *button, subfolders) {
			QAction *action = new QAction(QFileIconProvider().icon(button->currentPath()), button->text(), this);
			this->insertAction(firstAction, action);
			button->insertAction(firstAction, action);
		}
		this->insertSeparator(firstAction);
	}
	return QMenu::exec(p, action);
}
