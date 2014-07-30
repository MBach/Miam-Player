#include "searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
	QDialog(parent, Qt::Tool | Qt::FramelessWindowHint)
{
	this->setupUi(this);
}
