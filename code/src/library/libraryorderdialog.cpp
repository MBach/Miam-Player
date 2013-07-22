#include "libraryorderdialog.h"

LibraryOrderDialog::LibraryOrderDialog(QWidget *parent) :
	QDialog(parent, Qt::Popup)
{
	setupUi(this);

	foreach (QHeaderView *header, findChildren<QHeaderView*>()) {
		header->hide();
	}
}
