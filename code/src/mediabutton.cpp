#include "mediabutton.h"

#include <QDebug>

#include "mainwindow.h"

MediaButton::MediaButton(QWidget *parent) :
    QPushButton(parent)
{

}

void MediaButton::setIconFromTheme(const QString &theme)
{
	// The objectName in the UI file MUST match the alias in the QRC file !
	QString iconFile = ":/player/" + theme.toLower() + "/" + this->objectName().remove("Button");
	this->setIcon(QIcon(iconFile));
}

void MediaButton::setSize(const int &s)
{
	this->setIconSize(QSize(s, s));
}

/** Override the QPushButton slot to add a write/read QSetting system. */
void MediaButton::setVisible(bool visible)
{
	// Send a signal to write changes in QSettings object
	emit visibilityChanged(this, visible);
	QPushButton::setVisible(visible);
}
