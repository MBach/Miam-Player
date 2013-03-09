#include "mediabutton.h"

#include "mainwindow.h"

#include <QtDebug>

MediaButton::MediaButton(QWidget *parent) :
	QPushButton(parent)
{
	this->setFlat(Settings::getInstance()->buttonsFlat());
}

/** Redefined to load custom icons saved in settings. */
void MediaButton::setIcon(const QIcon &icon, bool toggled)
{
	Settings *settings = Settings::getInstance();
	QString path;

	// Used only for play/pause behaviour. Getting the custom icon for pause can produce unexpected behaviour
	// when replacing it by play.
	if (toggled) {
		path = settings->customIcon(this, toggled);
	}

	// If the path to the custom icon has been deleted meanwhile, then delete it from settings too
	if (path.isEmpty()) {
		QPushButton::setIcon(icon);
	} else if (QFile::exists(path)) {
		qDebug() << "la";
		QPushButton::setIcon(QIcon(path));
	} else {
		settings->setCustomIcon(this, QString());
		setIconFromTheme(settings->theme());
	}
}

/** Redefined to set shortcuts from settings at startup. */
void MediaButton::setObjectName(const QString &name)
{
	QKeySequence shortcut = Settings::getInstance()->shortcut(name.left(name.size() - QString("Button").size()));
	if (!shortcut.isEmpty()) {
		setShortcut(shortcut);
	}
	QObject::setObjectName(name);
}

/** Load an icon from a chosen theme in options. */
void MediaButton::setIconFromTheme(const QString &theme)
{
	// The objectName in the UI file MUST match the alias in the QRC file!
	QString iconFile = ":/player/" + theme.toLower() + "/" + this->objectName().remove("Button");
	this->setIcon(QIcon(iconFile));
}

/** Change the size of icons from the options. */
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


