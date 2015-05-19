#include "colordialog.h"

#include "customizethemedialog.h"
#include "settingsprivate.h"

#include <QtDebug>

ColorDialog::ColorDialog(CustomizeThemeDialog *parent) :
	QColorDialog(parent), _customizeThemeDialog(parent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setOptions(QColorDialog::NoButtons);
#ifndef Q_OS_MAC
	this->setWindowIcon(QIcon(":config/palette"));
#endif
	Qt::WindowFlags flags = this->windowFlags();
	flags |= Qt::ForeignWindow;
	this->setWindowFlags(flags);

	//connect(this, &QColorDialog::currentColorChanged, _customizeThemeDialog->targetedColor(), &Reflector::setColor);
	auto settings = SettingsPrivate::instance();
	connect(this, &QColorDialog::currentColorChanged, this, [=](const QColor &c) {
		_customizeThemeDialog->targetedColor()->setColor(c);
		settings->setCustomColorRole(_customizeThemeDialog->targetedColor()->colorRole(), currentColor());
	});
}

void ColorDialog::closeEvent(QCloseEvent *event)
{
	qDebug() << Q_FUNC_INFO << currentColor();
	_customizeThemeDialog->targetedColor()->setColor(currentColor());
	SettingsPrivate::instance()->setCustomColorRole(_customizeThemeDialog->targetedColor()->colorRole(), currentColor());
	SettingsPrivate::instance()->sync();
	QColorDialog::closeEvent(event);
	//_customizeThemeDialog->exec();
}
