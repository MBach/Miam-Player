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
	/// FIXME: what about separators?
	for (int i = 0; i < subfolders.length(); i++) {
		if (button->text() == subfolders.at(i)->text()) {
			subfolders.removeAt(i);
			qDebug() << button->text() << "was successfully removed";
			break;
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
