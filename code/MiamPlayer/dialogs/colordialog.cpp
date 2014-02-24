#include "colordialog.h"

#include "settings.h"

#include <QtDebug>

ColorDialog::ColorDialog(QWidget *parent) :
	QColorDialog(parent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setOptions(QColorDialog::NoButtons);
	this->setWindowIcon(QIcon(":config/palette"));
	Qt::WindowFlags flags = this->windowFlags();
	flags |= Qt::ForeignWindow;
	this->setWindowFlags(flags);

}

void ColorDialog::closeEvent(QCloseEvent *event)
{
	emit aboutToBeClosed();
	parentWidget()->show();
	QColorDialog::closeEvent(event);
}
