#include "mediabutton.h"

#include <settings.h>
#include <settingsprivate.h>
#include <QFile>
#include <QPainter>

#include <QtDebug>

MediaButton::MediaButton(QWidget *parent)
	: QPushButton(parent)
{
	auto settings = Settings::instance();
	this->setMaximumWidth(settings->buttonsSize() + 10);
	connect(settings, &Settings::themeHasChanged, this, &MediaButton::setIconFromTheme);
	connect(settings, &Settings::mediaButtonVisibilityChanged, this, [=](const QString &buttonName, bool value) {
		if (buttonName == objectName()) {
			this->setVisible(value);
		}
	});

	connect(SettingsPrivate::instance(), &SettingsPrivate::customIconForMediaButtonChanged, this, [=](const QString &button) {
		if (button == objectName()) {
			this->setIconFromTheme(settings->theme());
		}
	});
}

MediaButton::~MediaButton()
{

}

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
		}
	}
}

/** Change the size of icons from the options. */
void MediaButton::setSize(int s)
{
	this->setIconSize(QSize(s, s));
	this->setMaximumWidth(s + 10);
}
