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

	auto settingsPrivate = SettingsPrivate::instance();
	connect(this, &QColorDialog::currentColorChanged, this, [=](const QColor &c) {
		_customizeThemeDialog->targetedColor()->setColor(c);
		settingsPrivate->setCustomColorRole(_customizeThemeDialog->targetedColor()->colorRole(), currentColor());
	});
}
