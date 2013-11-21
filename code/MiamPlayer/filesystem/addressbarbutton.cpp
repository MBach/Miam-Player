#include "addressbarbutton.h"

#include <QDir>

#include <QMouseEvent>
#include <QtDebug>

AddressBarButton::AddressBarButton(ButtonType type, const QString &newPath, int index, QWidget *parent) :
	QPushButton(parent), _type(type), path(QDir::toNativeSeparators(newPath)), idx(index)
{
	QString styleSheet;
	styleSheet = "QPushButton {";
	styleSheet += " border: 0; border-radius: 0px; ";
	styleSheet += " height: 20px; ";
	styleSheet += " margin-left: 0px; margin-right: 0px; ";
	styleSheet += " padding-left: 2px; padding-right: 2px; ";
	styleSheet += "}";

	styleSheet += "QPushButton:hover {";
	styleSheet += " border: 1px solid #3c7fb1;";
	styleSheet += " border-radius: 0px;";
	styleSheet += " subcontrol-position: right; ";
	styleSheet += " background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eaf6fd, stop: 0.4 #d7effc, stop: 0.41 #bde6fd, stop: 1.0 #a6d9f4);";
	styleSheet += "}";

	styleSheet += "QPushButton::menu-indicator:hover {";
	styleSheet += " border-left: 1px solid #3c7fb1; ";
	styleSheet += "}";

	styleSheet += "QPushButton:hover {";
	styleSheet += " min-width: 17px; max-width: 17px; width: 17px;";
	styleSheet += " border: 1px solid #3c7fb1;";
	styleSheet += " border-radius: 0px;";
	styleSheet += " background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eaf6fd, stop: 0.4 #d7effc, stop: 0.41 #bde6fd, stop: 1.0 #a6d9f4);";
	styleSheet += "}";

	if (path.right(1) != QDir::separator()) {
		path += QDir::separator();
	}

	this->setStyleSheet(styleSheet);
	//this->setMouseTracking(true);
}

QString AddressBarButton::currentPath() const
{
	// Removes the last directory separator, unless for the root on windows which is like C:\. It's not possible to do "cd C:"
	if (!QDir(path).isRoot()) {
		return path.left(path.length() - 1);
	} else {
		return path;
	}
}

/*
void AddressBarButton::mouseMoveEvent(QMouseEvent * e)
{
	qDebug() << "AddressBarButton::mouseMoveEvent" << frameGeometry() << mapToParent(e->pos());
	if (frameGeometry().contains(mapToParent(e->pos()))) {
		//qDebug() << "inside!";
	}
	QPushButton::mouseMoveEvent(e);
}
*/
