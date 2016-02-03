#include "mediabutton.h"

#include <settings.h>
#include <settingsprivate.h>
#include <QFile>
#include <QPainter>

#include <QtDebug>

MediaButton::MediaButton(QWidget *parent)
	: QPushButton(parent)
{
	this->setMaximumWidth(SettingsPrivate::instance()->buttonsSize() + 10);
	auto settings = Settings::instance();
	connect(settings, &Settings::themeHasChanged, this, &MediaButton::setIconFromTheme);
}

MediaButton::~MediaButton()
{
	//Settings::instance()->disconnect();
}

/** Redefined to load custom icons saved in settings. */
/*void MediaButton::setIcon(const QIcon &icon)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (settings->isButtonThemeCustomized() && settings->hasCustomIcon(objectName())) {
		QPushButton::setIcon(QIcon(settings->customIcon(objectName())));
	} else if (icon.isNull()){
		settings->setCustomIcon(objectName(), QString());
		setIconFromTheme(Settings::instance()->theme());
	} else {
		QPushButton::setIcon(icon);
	}
}*/

void MediaButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	int x = (this->height() - iconSize().height()) / 2;
	int y = (this->width() - iconSize().width()) / 2;
	if (this->isCheckable() && this->isChecked()) {
		p.drawPixmap(QPoint(x, y), icon().pixmap(iconSize(), QIcon::Selected));
	} else {
		p.drawPixmap(QPoint(x, y), icon().pixmap(iconSize()));
	}
}

/** Load an icon from a chosen theme in options. */
void MediaButton::setIconFromTheme(const QString &theme)
{
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	if (settingsPrivate->isButtonThemeCustomized() && settingsPrivate->hasCustomIcon(objectName())) {
		setIcon(QIcon(settingsPrivate->customIcon(objectName())));
	} else {
		// The objectName in the UI file MUST match the alias in the QRC file!
		QString iconFile = ":/player/" + theme.toLower() + "/" + this->objectName().remove("Button");
		QIcon icon(iconFile);
		if (!icon.isNull()) {
			QPushButton::setIcon(QIcon(iconFile));
		} else {
			qDebug() << Q_FUNC_INFO << objectName();
		}
	}

}

/** Change the size of icons from the options. */
void MediaButton::setSize(int s)
{
	this->setIconSize(QSize(s, s));
	this->setMaximumWidth(s + 10);
}
