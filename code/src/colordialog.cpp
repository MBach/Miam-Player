#include "colordialog.h"

#include "settings.h"

#include <QtDebug>

#include <QIcon>

ColorDialog::ColorDialog(QWidget *parent) :
	QColorDialog(parent)
{
	this->setOptions(QColorDialog::NoButtons);
	this->setWindowIcon(QIcon(":config/palette"));
}

void ColorDialog::closeEvent(QCloseEvent *event)
{
	parentWidget()->show();
	QColorDialog::closeEvent(event);
	foreach(QWidget *w, targets) {
		Settings::getInstance()->setCustomStyleSheet(w);
	}
}

void ColorDialog::setTargets(QList<QWidget *> t) {
	targets = t;
	this->show();
}
