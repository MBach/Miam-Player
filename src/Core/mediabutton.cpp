#include "mediabutton.h"

#include <settings.h>
#include <settingsprivate.h>
#include <QFile>
#include <QPainter>

#include <QtDebug>

MediaButton::MediaButton(QWidget *parent)
	: QPushButton(parent)
	, _mediaPlayer(nullptr)
{
	this->setMaximumWidth(SettingsPrivate::instance()->buttonsSize() + 10);
}

void MediaButton::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
}

/** Redefined to load custom icons saved in settings. */
void MediaButton::setIcon(const QIcon &icon)
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
	// The objectName in the UI file MUST match the alias in the QRC file!
	QString iconFile = ":/player/" + theme.toLower() + "/" + this->objectName().remove("Button");
	QIcon icon(iconFile);
	if (!icon.isNull()) {
		QPushButton::setIcon(QIcon(iconFile));
	}
	emit mediaButtonChanged();
}

/** Change the size of icons from the options. */
void MediaButton::setSize(const int &s)
{
	this->setIconSize(QSize(s, s));
	this->setMaximumWidth(s + 10);
	emit mediaButtonChanged();
}
