#include "libraryitem.h"
#include "librarymodel.h"
#include "settings.h"

LibraryItem::LibraryItem(const QString &text) :
	QStandardItem(text)
{
	setFont(Settings::getInstance()->font(Settings::LIBRARY));
}

LibraryItem::LibraryItem() :
	QStandardItem()
{
	this->setData(-1, SUFFIX);
}

void LibraryItem::setDisplayedName(const char *name, int size)
{
	QByteArray byteArray(name, size);
	QVariant v(byteArray);
	setData(v.toString(), Qt::DisplayRole);
}
