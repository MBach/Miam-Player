#include "colordialog.h"

#include "settings.h"

#include <QtDebug>

ColorDialog::ColorDialog(QWidget *parent) :
	QColorDialog(parent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setOptions(QColorDialog::NoButtons);
	this->setWindowIcon(QIcon(":config/palette"));
}

void ColorDialog::closeEvent(QCloseEvent *event)
{
	emit aboutToBeClosed();
	parentWidget()->show();
	QColorDialog::closeEvent(event);
	/*foreach(QWidget *w, reflector->associatedInstances()) {
		Settings::getInstance()->setCustomStyleSheet(w);
	}*/
}

void ColorDialog::setPaintableElements(Reflector *r)
{
	this->reflector = r;
	this->show();
}
